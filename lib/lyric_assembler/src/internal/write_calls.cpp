
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/internal/write_calls.h>
#include <lyric_assembler/template_handle.h>
#include <tempo_utils/bytes_appender.h>

static tempo_utils::Status
write_call(
    lyric_assembler::CallSymbol *callSymbol,
    lyric_assembler::TypeCache *typeCache,
    lyric_assembler::SymbolCache *symbolCache,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::CallDescriptor>> &calls_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector,
    std::vector<uint8_t> &bytecode)
{
    auto index = static_cast<tu_uint32>(calls_vector.size());

    auto *proc = callSymbol->callProc();
    auto *code = proc->procCode();
    auto bytecodeOffset = static_cast<tu_uint32>(bytecode.size());

    // build the proc prologue
    tempo_utils::BytesAppender prologue;
    prologue.appendU16(static_cast<tu_uint16>(proc->getArity()));
    prologue.appendU16(static_cast<tu_uint16>(proc->numLocals()));
    prologue.appendU16(static_cast<tu_uint16>(proc->numLexicals()));
    for (auto lexical = proc->lexicalsBegin(); lexical != proc->lexicalsEnd(); lexical++) {
        prologue.appendU32(lexical->activationCall.getAddress());
        prologue.appendU32(lexical->targetOffset);
        prologue.appendU8(static_cast<tu_uint8>(lexical->lexicalTarget));
    }

    // build the proc header
    tempo_utils::BytesAppender header;
    header.appendU32(prologue.getSize() + code->bytecodeSize());

    // write the proc to the bytecode
    bytecode.insert(bytecode.cend(), header.bytesBegin(), header.bytesEnd());
    bytecode.insert(bytecode.cend(), prologue.bytesBegin(), prologue.bytesEnd());
    bytecode.insert(bytecode.cend(), code->bytecodeBegin(), code->bytecodeEnd());

    // add call descriptor
    auto callPathString = callSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(callPathString);

    lyo1::CallFlags callFlags = lyo1::CallFlags::NONE;
    if (!callSymbol->getAddress().isValid())
        callFlags |= lyo1::CallFlags::DeclOnly;
    switch (callSymbol->getAccessType()) {
        case lyric_object::AccessType::Public:
            callFlags |= lyo1::CallFlags::GlobalVisibility;
            break;
        case lyric_object::AccessType::Protected:
            callFlags |= lyo1::CallFlags::InheritVisibility;
            break;
        case lyric_object::AccessType::Private:
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                "invalid call access");
    }

    tu_uint32 callTemplate = lyric_runtime::INVALID_ADDRESS_U32;
    if (callSymbol->callTemplate() != nullptr) {
        callTemplate = callSymbol->callTemplate()->getAddress().getAddress();
    }

    lyo1::TypeSection receiverSection = lyo1::TypeSection::Invalid;
    tu_uint32 receiverDescriptor = lyric_runtime::INVALID_ADDRESS_U32;

    auto receiverUrl = callSymbol->getReceiverUrl();
    if (receiverUrl.isValid()) {
        lyric_assembler::AbstractSymbol *receiver;
        TU_ASSIGN_OR_RETURN (receiver, symbolCache->getOrImportSymbol(receiverUrl));
        switch (receiver->getSymbolType()) {
            case lyric_assembler::SymbolType::CLASS:
                receiverSection = lyo1::TypeSection::Class;
                receiverDescriptor = cast_symbol_to_class(receiver)->getAddress().getAddress();
                break;
            case lyric_assembler::SymbolType::ENUM:
                receiverSection = lyo1::TypeSection::Enum;
                receiverDescriptor = cast_symbol_to_enum(receiver)->getAddress().getAddress();
                break;
            case lyric_assembler::SymbolType::INSTANCE:
                receiverSection = lyo1::TypeSection::Instance;
                receiverDescriptor = cast_symbol_to_instance(receiver)->getAddress().getAddress();
                break;
            case lyric_assembler::SymbolType::STRUCT:
                receiverSection = lyo1::TypeSection::Struct;
                receiverDescriptor = cast_symbol_to_struct(receiver)->getAddress().getAddress();
                break;
            default:
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid call receiver");
        }
        callFlags |= lyo1::CallFlags::Bound;
    }

    switch (callSymbol->getMode()) {
        case lyric_object::CallMode::Inline:
            callFlags |= lyo1::CallFlags::Inline;
            break;
        case lyric_object::CallMode::Constructor:
            callFlags |= lyo1::CallFlags::Ctor;
            break;
        default:
            break;
    }

    // FIXME: i think we should remove the call type field. the function type can be synthesized as needed.
    tu_uint32 callType = lyric_runtime::INVALID_ADDRESS_U32;
    if (callSymbol->callType() != nullptr) {
        callType = callSymbol->callType()->getAddress().getAddress();
    }

    std::vector<flatbuffers::Offset<lyo1::Parameter>> listParameters;
    for (auto it = callSymbol->listPlacementBegin(); it != callSymbol->listPlacementEnd(); it++) {
        const auto &param = *it;

        lyo1::ParameterT p;
        p.parameter_name = param.name;

        p.flags = lyo1::ParameterFlags::NONE;
        switch (param.placement) {
            case lyric_object::PlacementType::List:
            case lyric_object::PlacementType::ListOpt:
                break;
            default:
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid placement type");
        }
        if (param.isVariable) {
            p.flags |= lyo1::ParameterFlags::Var;
        }

        lyric_assembler::TypeHandle *paramTypeHandle;
        TU_ASSIGN_OR_RETURN (paramTypeHandle, typeCache->getOrMakeType(param.typeDef));
        p.parameter_type = paramTypeHandle->getAddress().getAddress();
        p.initializer_call = lyric_runtime::INVALID_ADDRESS_U32;
        if (callSymbol->hasInitializer(param.name)) {
            if (param.placement != lyric_object::PlacementType::ListOpt)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid placement");
            auto initializerUrl = callSymbol->getInitializer(param.name);
            lyric_assembler::AbstractSymbol *initializer;
            TU_ASSIGN_OR_RETURN (initializer, symbolCache->getOrImportSymbol(initializerUrl));
            if (initializer->getSymbolType() != lyric_assembler::SymbolType::CALL)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid initializer {}", initializerUrl.toString());
            auto *initSymbol = cast_symbol_to_call(initializer);
            p.initializer_call = initSymbol->getAddress().getAddress();
        }

        listParameters.push_back(lyo1::CreateParameter(buffer, &p));
    }
    auto fb_listParameters = buffer.CreateVector(listParameters);

    std::vector<flatbuffers::Offset<lyo1::Parameter>> namedParameters;
    for (auto it = callSymbol->namedPlacementBegin(); it != callSymbol->namedPlacementEnd(); it++) {
        const auto &param = *it;

        lyo1::ParameterT p;
        p.parameter_name = param.name;

        p.flags = lyo1::ParameterFlags::NONE;
        switch (param.placement) {
            case lyric_object::PlacementType::Ctx:
                p.flags |= lyo1::ParameterFlags::Ctx;
                break;
            case lyric_object::PlacementType::Named:
            case lyric_object::PlacementType::NamedOpt:
                break;
            default:
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid placement type");
        }
        if (param.isVariable) {
            p.flags |= lyo1::ParameterFlags::Var;
        }

        lyric_assembler::TypeHandle *paramTypeHandle;
        TU_ASSIGN_OR_RETURN (paramTypeHandle, typeCache->getOrMakeType(param.typeDef));
        p.parameter_type = paramTypeHandle->getAddress().getAddress();
        p.initializer_call = lyric_runtime::INVALID_ADDRESS_U32;
        if (callSymbol->hasInitializer(param.name)) {
            if (param.placement != lyric_object::PlacementType::NamedOpt)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid placement");
            auto initializerUrl = callSymbol->getInitializer(param.name);
            lyric_assembler::AbstractSymbol *initializer;
            TU_ASSIGN_OR_RETURN (initializer, symbolCache->getOrImportSymbol(initializerUrl));
            if (initializer->getSymbolType() != lyric_assembler::SymbolType::CALL)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid initializer {}", initializerUrl.toString());
            auto *initSymbol = cast_symbol_to_call(initializer);
            p.initializer_call = initSymbol->getAddress().getAddress();
        }

        namedParameters.push_back(lyo1::CreateParameter(buffer, &p));
    }
    auto fb_namedParameters = buffer.CreateVector(namedParameters);

    ::flatbuffers::Offset<lyo1::Parameter> fb_restParameter = 0;
    auto *rest = callSymbol->restPlacement();
    if (rest != nullptr) {
        lyo1::ParameterT p;
        p.parameter_name = rest->name;

        p.flags = lyo1::ParameterFlags::NONE;
        if (rest->isVariable) {
            p.flags |= lyo1::ParameterFlags::Var;
        }

        lyric_assembler::TypeHandle *paramTypeHandle;
        TU_ASSIGN_OR_RETURN (paramTypeHandle, typeCache->getOrMakeType(rest->typeDef));
        p.parameter_type = paramTypeHandle->getAddress().getAddress();
        p.initializer_call = 0;

        fb_restParameter = lyo1::CreateParameter(buffer, &p);
    }

    lyric_assembler::TypeHandle *returnTypeHandle;
    TU_ASSIGN_OR_RETURN (returnTypeHandle, typeCache->getOrMakeType(callSymbol->getReturnType()));
    tu_uint32 resultType = returnTypeHandle->getAddress().getAddress();

    // add call descriptor
    calls_vector.push_back(lyo1::CreateCallDescriptor(buffer, fullyQualifiedName,
        callTemplate, receiverSection, receiverDescriptor, callType, bytecodeOffset, callFlags,
        fb_listParameters, fb_namedParameters, fb_restParameter, resultType));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Call, index));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_calls(
    const AssemblyState *assemblyState,
    flatbuffers::FlatBufferBuilder &buffer,
    CallsOffset &callsOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector,
    std::vector<tu_uint8> &bytecode)
{
    TU_ASSERT (assemblyState != nullptr);

    SymbolCache *symbolCache = assemblyState->symbolCache();
    TypeCache *typeCache = assemblyState->typeCache();
    std::vector<flatbuffers::Offset<lyo1::CallDescriptor>> calls_vector;

    for (auto iterator = assemblyState->callsBegin(); iterator != assemblyState->callsEnd(); iterator++) {
        auto &callSymbol = *iterator;
        TU_RETURN_IF_NOT_OK (
            write_call(callSymbol, typeCache, symbolCache, buffer, calls_vector, symbols_vector, bytecode));
    }

    // create the calls vector
    callsOffset = buffer.CreateVector(calls_vector);

    return {};
}
