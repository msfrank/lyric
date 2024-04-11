
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

static lyric_assembler::AssemblerStatus
write_call(
    lyric_assembler::CallSymbol *callSymbol,
    lyric_assembler::TypeCache *typeCache,
    lyric_assembler::SymbolCache *symbolCache,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::CallDescriptor>> &calls_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector,
    std::vector<uint8_t> &bytecode)
{
    auto index = static_cast<uint32_t>(calls_vector.size());

    auto *proc = callSymbol->callProc();
    auto *code = proc->procCode();
    auto bytecodeOffset = static_cast<uint32_t>(bytecode.size());

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

    uint32_t callTemplate = lyric_runtime::INVALID_ADDRESS_U32;
    if (callSymbol->callTemplate() != nullptr)
        callTemplate = callSymbol->callTemplate()->getAddress().getAddress();

    lyo1::TypeSection receiverSection = lyo1::TypeSection::Invalid;
    uint32_t receiverDescriptor = lyric_runtime::INVALID_ADDRESS_U32;

    auto receiverUrl = callSymbol->getReceiverUrl();
    if (receiverUrl.isValid() && symbolCache->hasSymbol(receiverUrl)) {
        auto *sym = symbolCache->getSymbol(receiverUrl);
        switch (sym->getSymbolType()) {
            case lyric_assembler::SymbolType::CLASS:
                receiverSection = lyo1::TypeSection::Class;
                receiverDescriptor = cast_symbol_to_class(sym)->getAddress().getAddress();
                break;
            case lyric_assembler::SymbolType::ENUM:
                receiverSection = lyo1::TypeSection::Enum;
                receiverDescriptor = cast_symbol_to_enum(sym)->getAddress().getAddress();
                break;
            case lyric_assembler::SymbolType::INSTANCE:
                receiverSection = lyo1::TypeSection::Instance;
                receiverDescriptor = cast_symbol_to_instance(sym)->getAddress().getAddress();
                break;
            case lyric_assembler::SymbolType::STRUCT:
                receiverSection = lyo1::TypeSection::Struct;
                receiverDescriptor = cast_symbol_to_struct(sym)->getAddress().getAddress();
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

    // FIXME: call must have a type
    uint32_t callType = lyric_runtime::INVALID_ADDRESS_U32;
    if (callSymbol->callType() != nullptr)
        callType = callSymbol->callType()->getAddress().getAddress();

    std::vector<lyo1::Parameter> parameters;
    std::vector<std::string> names;

    for (const auto &param : callSymbol->getParameters()) {
        lyo1::ParameterFlags flags = lyo1::ParameterFlags::NONE;
        switch (param.placement) {
            case lyric_object::PlacementType::Ctx:
                flags |= lyo1::ParameterFlags::Ctx;
                break;
            case lyric_object::PlacementType::Named:
            case lyric_object::PlacementType::Opt:
                flags |= lyo1::ParameterFlags::Named;
                break;
            case lyric_object::PlacementType::List:
                break;
            default:
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid placement type");
        }
        if (param.isVariable)
            flags |= lyo1::ParameterFlags::Var;
        uint32_t paramType = typeCache->getType(param.typeDef)->getAddress().getAddress();
        uint32_t paramDefault = lyric_runtime::INVALID_ADDRESS_U32;
        if (callSymbol->hasInitializer(param.name)) {
            if (param.placement != lyric_object::PlacementType::Opt)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid placement");
            auto initializerUrl = callSymbol->getInitializer(param.name);
            auto *sym = symbolCache->getSymbol(initializerUrl);
            if (sym == nullptr)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "missing initializer {}", initializerUrl.toString());
            if (sym->getSymbolType() != lyric_assembler::SymbolType::CALL)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid initializer {}", initializerUrl.toString());
            auto *initSymbol = cast_symbol_to_call(sym);
            paramDefault = initSymbol->getAddress().getAddress();
        }
        uint8_t nameOffset = names.size();
        names.emplace_back(param.name);
        parameters.emplace_back(flags, paramType, paramDefault, nameOffset, lyric_runtime::INVALID_OFFSET_U8);
    }

    // FIXME: call must have a result type
    uint32_t resultType = lyric_runtime::INVALID_ADDRESS_U32;
    if (typeCache->getType(callSymbol->getReturnType()) != nullptr)
        resultType = typeCache->getType(callSymbol->getReturnType())->getAddress().getAddress();

    auto restParam = callSymbol->getRest();
    if (!restParam.isEmpty()) {
        const auto &param = restParam.getValue();
        lyo1::ParameterFlags flags = lyo1::ParameterFlags::Rest;
        uint32_t paramType = typeCache->getType(param.typeDef)->getAddress().getAddress();
        uint8_t nameOffset = lyric_runtime::INVALID_OFFSET_U8;
        if (!param.name.empty()) {
            nameOffset = names.size();
            names.emplace_back(param.name);
            flags |= lyo1::ParameterFlags::Named;
        }
        if (param.isVariable)
            flags |= lyo1::ParameterFlags::Var;
        auto rest = lyo1::Parameter(flags, paramType,
            lyric_runtime::INVALID_ADDRESS_U32, nameOffset, lyric_runtime::INVALID_OFFSET_U8);
        calls_vector.push_back(lyo1::CreateCallDescriptor(buffer, fullyQualifiedName,
            callTemplate, receiverSection, receiverDescriptor,
            callType, bytecodeOffset, callFlags,
            buffer.CreateVectorOfStructs(parameters), &rest,
            buffer.CreateVectorOfStrings(names), resultType));
    } else {
        calls_vector.push_back(lyo1::CreateCallDescriptor(buffer, fullyQualifiedName,
            callTemplate, receiverSection, receiverDescriptor,
            callType, bytecodeOffset, callFlags,
            buffer.CreateVectorOfStructs(parameters), nullptr,
            buffer.CreateVectorOfStrings(names), resultType));
    }

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Call, index));

    return lyric_assembler::AssemblerStatus::ok();
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

    return AssemblerStatus::ok();
}
