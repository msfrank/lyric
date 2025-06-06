
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/internal/write_calls.h>
#include <lyric_assembler/object_writer.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <tempo_utils/bytes_appender.h>

tempo_utils::Status
lyric_assembler::internal::touch_call(
    const CallSymbol *callSymbol,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (callSymbol != nullptr);

    auto callUrl = callSymbol->getSymbolUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertSymbol(callUrl, callSymbol, alreadyInserted));
    if (alreadyInserted)
        return {};

    // if call is bound then write the receiver
    if (callSymbol->isBound()) {
        auto *symbolCache = objectState->symbolCache();
        AbstractSymbol *receiverSymbol;
        TU_ASSIGN_OR_RETURN (receiverSymbol, symbolCache->getOrImportSymbol(callSymbol->getReceiverUrl()));
        switch (receiverSymbol->getSymbolType()) {
            case SymbolType::CLASS:
                TU_RETURN_IF_NOT_OK (writer.touchClass(cast_symbol_to_class(receiverSymbol)));
                break;
            case SymbolType::CONCEPT:
                TU_RETURN_IF_NOT_OK (writer.touchConcept(cast_symbol_to_concept(receiverSymbol)));
                break;
            case SymbolType::ENUM:
                TU_RETURN_IF_NOT_OK (writer.touchEnum(cast_symbol_to_enum(receiverSymbol)));
                break;
            case SymbolType::EXISTENTIAL:
                TU_RETURN_IF_NOT_OK (writer.touchExistential(cast_symbol_to_existential(receiverSymbol)));
                break;
            case SymbolType::INSTANCE:
                TU_RETURN_IF_NOT_OK (writer.touchInstance(cast_symbol_to_instance(receiverSymbol)));
                break;
            case SymbolType::STRUCT:
                TU_RETURN_IF_NOT_OK (writer.touchStruct(cast_symbol_to_struct(receiverSymbol)));
                break;
            default:
                return AssemblerStatus::forCondition(
                    AssemblerCondition::kAssemblerInvariant, "invalid receiver symbol for call");
        }
    }

    // if call is an imported symbol then we are done
    if (callSymbol->isImported())
        return {};

    for (auto it = callSymbol->listPlacementBegin(); it != callSymbol->listPlacementEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchType(it->typeDef));
    }

    for (auto it = callSymbol->namedPlacementBegin(); it != callSymbol->namedPlacementEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchType(it->typeDef));
    }

    auto *rest = callSymbol->restPlacement();
    if (rest != nullptr) {
        TU_RETURN_IF_NOT_OK (writer.touchType(rest->typeDef));
    }

    TU_RETURN_IF_NOT_OK (writer.touchType(callSymbol->getReturnType()));

    auto *templateHandle = callSymbol->callTemplate();
    if (templateHandle) {
        TU_RETURN_IF_NOT_OK (writer.touchTemplate(templateHandle));
    }

    auto *proc = callSymbol->callProc()->procCode();
    return proc->touch(writer);
}

static tempo_utils::Status
write_call(
    const lyric_assembler::CallSymbol *callSymbol,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::CallDescriptor>> &calls_vector,
    std::vector<uint8_t> &bytecode)
{
    auto bytecodeOffset = static_cast<tu_uint32>(bytecode.size());

    auto *proc = callSymbol->callProc();
    auto *code = proc->procCode();

    lyric_object::BytecodeBuilder bytecodeBuilder;
    TU_RETURN_IF_NOT_OK (code->build(writer, bytecodeBuilder));

    // build the proc prologue
    tempo_utils::BytesAppender prologue;
    prologue.appendU16(static_cast<tu_uint16>(proc->getArity()));
    prologue.appendU16(static_cast<tu_uint16>(proc->numLocals()));
    prologue.appendU16(static_cast<tu_uint16>(proc->numLexicals()));
    for (auto lexical = proc->lexicalsBegin(); lexical != proc->lexicalsEnd(); lexical++) {
        tu_uint32 address;
        TU_ASSIGN_OR_RETURN (address,
            writer.getSectionAddress(lexical->activationCall->getSymbolUrl(), lyric_object::LinkageSection::Call));
        prologue.appendU32(address);
        prologue.appendU32(lexical->targetOffset);
        prologue.appendU8(static_cast<tu_uint8>(lexical->lexicalTarget));
    }

    // build the proc header
    tempo_utils::BytesAppender header;
    header.appendU32(prologue.getSize() + bytecodeBuilder.bytecodeSize());

    // write the proc to the bytecode
    bytecode.insert(bytecode.cend(), header.bytesBegin(), header.bytesEnd());
    bytecode.insert(bytecode.cend(), prologue.bytesBegin(), prologue.bytesEnd());
    bytecode.insert(bytecode.cend(), bytecodeBuilder.bytecodeBegin(), bytecodeBuilder.bytecodeEnd());

    // add call descriptor
    auto callPathString = callSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(callPathString);

    lyo1::CallFlags callFlags = lyo1::CallFlags::NONE;
    if (callSymbol->isDeclOnly())
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
        TU_ASSIGN_OR_RETURN (callTemplate,
            writer.getTemplateOffset(callSymbol->callTemplate()->getTemplateUrl()));
    }

    auto receiverUrl = callSymbol->getReceiverUrl();

    tu_uint32 receiverSymbolIndex = lyric_object::INVALID_ADDRESS_U32;
    if (receiverUrl.isValid()) {
        lyric_object::LinkageSection receiverSection;
        TU_ASSIGN_OR_RETURN (receiverSection, writer.getSymbolSection(receiverUrl));
        switch (receiverSection) {
            case lyric_object::LinkageSection::Class:
            case lyric_object::LinkageSection::Concept:
            case lyric_object::LinkageSection::Enum:
            case lyric_object::LinkageSection::Existential:
            case lyric_object::LinkageSection::Instance:
            case lyric_object::LinkageSection::Struct:
                break;
            default:
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid call receiver");
        }
        TU_ASSIGN_OR_RETURN (receiverSymbolIndex, writer.getSymbolAddress(receiverUrl));
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

    if (callSymbol->isNoReturn()) {
        callFlags |= lyo1::CallFlags::NoReturn;
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

        TU_ASSIGN_OR_RETURN (p.parameter_type, writer.getTypeOffset(param.typeDef));

        p.initializer_call = lyric_runtime::INVALID_ADDRESS_U32;
        if (callSymbol->hasInitializer(param.name)) {
            if (param.placement != lyric_object::PlacementType::ListOpt)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid placement");
            auto initializerUrl = callSymbol->getInitializer(param.name);
            TU_ASSIGN_OR_RETURN (p.initializer_call,
                writer.getSectionAddress(initializerUrl, lyric_object::LinkageSection::Call));
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

        TU_ASSIGN_OR_RETURN (p.parameter_type, writer.getTypeOffset(param.typeDef));

        p.initializer_call = lyric_runtime::INVALID_ADDRESS_U32;
        if (callSymbol->hasInitializer(param.name)) {
            if (param.placement != lyric_object::PlacementType::NamedOpt)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid placement");
            auto initializerUrl = callSymbol->getInitializer(param.name);
            TU_ASSIGN_OR_RETURN (p.initializer_call,
                writer.getSectionAddress(initializerUrl, lyric_object::LinkageSection::Call));
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

        TU_ASSIGN_OR_RETURN (p.parameter_type, writer.getTypeOffset(rest->typeDef));
        p.initializer_call = lyric_object::INVALID_ADDRESS_U32;

        fb_restParameter = lyo1::CreateParameter(buffer, &p);
    }

    tu_uint32 returnType;
    TU_ASSIGN_OR_RETURN (returnType, writer.getTypeOffset(callSymbol->getReturnType()));

    // add call descriptor
    calls_vector.push_back(lyo1::CreateCallDescriptor(buffer,
        fullyQualifiedName, callTemplate, receiverSymbolIndex, bytecodeOffset, callFlags,
        fb_listParameters, fb_namedParameters, fb_restParameter, returnType));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_calls(
    const std::vector<const CallSymbol *> &calls,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    CallsOffset &callsOffset,
    std::vector<tu_uint8> &bytecode)
{
    std::vector<flatbuffers::Offset<lyo1::CallDescriptor>> calls_vector;

    for (const auto *callSymbol : calls) {
        TU_RETURN_IF_NOT_OK (write_call(callSymbol, writer, buffer, calls_vector, bytecode));
    }

    // create the calls vector
    callsOffset = buffer.CreateVector(calls_vector);

    return {};
}
