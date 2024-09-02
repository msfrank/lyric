
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/internal/write_actions.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Status
lyric_assembler::internal::touch_action(
    const ActionSymbol *actionSymbol,
    const ObjectState *objectState,
    ObjectWriter &writer)
{
    TU_ASSERT (actionSymbol != nullptr);

    auto actionUrl = actionSymbol->getSymbolUrl();

    bool alreadyInserted;
    TU_RETURN_IF_NOT_OK (writer.insertSymbol(actionUrl, actionSymbol, alreadyInserted));
    if (alreadyInserted)
        return {};

    // if action is an imported symbol then we are done
    if (actionSymbol->isImported())
        return {};

    for (auto it = actionSymbol->listPlacementBegin(); it != actionSymbol->listPlacementEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchType(it->typeDef));
    }

    for (auto it = actionSymbol->namedPlacementBegin(); it != actionSymbol->namedPlacementEnd(); it++) {
        TU_RETURN_IF_NOT_OK (writer.touchType(it->typeDef));
    }

    auto *rest = actionSymbol->restPlacement();
    if (rest != nullptr) {
        TU_RETURN_IF_NOT_OK (writer.touchType(rest->typeDef));
    }

    TU_RETURN_IF_NOT_OK (writer.touchType(actionSymbol->getReturnType()));

    auto *templateHandle = actionSymbol->actionTemplate();
    if (templateHandle) {
        TU_RETURN_IF_NOT_OK (writer.touchTemplate(templateHandle));
    }

    return {};
}

static tempo_utils::Status
write_action(
    const lyric_assembler::ActionSymbol *actionSymbol,
    const lyric_assembler::ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    std::vector<flatbuffers::Offset<lyo1::ActionDescriptor>> &actions_vector,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    auto index = static_cast<tu_uint32>(actions_vector.size());

    auto actionPathString = actionSymbol->getSymbolUrl().getSymbolPath().toString();
    auto fullyQualifiedName = buffer.CreateSharedString(actionPathString);

    lyo1::ActionFlags actionFlags = lyo1::ActionFlags::NONE;
    if (actionSymbol->isDeclOnly())
        actionFlags |= lyo1::ActionFlags::DeclOnly;

    tu_uint32 actionTemplate = lyric_runtime::INVALID_ADDRESS_U32;
    if (actionSymbol->actionTemplate() != nullptr) {
        TU_ASSIGN_OR_RETURN (actionTemplate,
            writer.getTemplateOffset(actionSymbol->actionTemplate()->getTemplateUrl()));
    }

    auto receiverUrl = actionSymbol->getReceiverUrl();
    lyric_assembler::SymbolEntry receiver;
    TU_ASSIGN_OR_RETURN (receiver, writer.getSymbolEntry(receiverUrl));

    lyo1::TypeSection receiverSection = lyo1::TypeSection::Invalid;
    tu_uint32 receiverDescriptor = lyric_runtime::INVALID_ADDRESS_U32;
    switch (receiver.section) {
        case lyric_object::LinkageSection::Concept:
            receiverSection = lyo1::TypeSection::Concept;
            receiverDescriptor = receiver.address;
            break;
        default:
            return lyric_assembler::AssemblerStatus::forCondition(
                lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                "invalid action receiver");
    }

    std::vector<flatbuffers::Offset<lyo1::Parameter>> listParameters;
    for (auto it = actionSymbol->listPlacementBegin(); it != actionSymbol->listPlacementEnd(); it++) {
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
        if (actionSymbol->hasInitializer(param.name)) {
            if (param.placement != lyric_object::PlacementType::ListOpt)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid placement");
            auto initializerUrl = actionSymbol->getInitializer(param.name);
            TU_ASSIGN_OR_RETURN (p.initializer_call,
                writer.getSymbolAddress(initializerUrl, lyric_object::LinkageSection::Call));
        }

        listParameters.push_back(lyo1::CreateParameter(buffer, &p));
    }
    auto fb_listParameters = buffer.CreateVector(listParameters);

    std::vector<flatbuffers::Offset<lyo1::Parameter>> namedParameters;
    for (auto it = actionSymbol->namedPlacementBegin(); it != actionSymbol->namedPlacementEnd(); it++) {
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
        if (actionSymbol->hasInitializer(param.name)) {
            if (param.placement != lyric_object::PlacementType::NamedOpt)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid placement");
            auto initializerUrl = actionSymbol->getInitializer(param.name);
            TU_ASSIGN_OR_RETURN (p.initializer_call,
                writer.getSymbolAddress(initializerUrl, lyric_object::LinkageSection::Call));
        }

        namedParameters.push_back(lyo1::CreateParameter(buffer, &p));
    }
    auto fb_namedParameters = buffer.CreateVector(namedParameters);

    ::flatbuffers::Offset<lyo1::Parameter> fb_restParameter = 0;
    auto *rest = actionSymbol->restPlacement();
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
    TU_ASSIGN_OR_RETURN (returnType, writer.getTypeOffset(actionSymbol->getReturnType()));

    // add action descriptor
    actions_vector.push_back(lyo1::CreateActionDescriptor(buffer, fullyQualifiedName,
        actionTemplate, receiverSection, receiverDescriptor, actionFlags,
        fb_listParameters, fb_namedParameters, fb_restParameter, returnType));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Action, index));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_actions(
    const std::vector<const ActionSymbol *> &actions,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    ActionsOffset &actionsOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    std::vector<flatbuffers::Offset<lyo1::ActionDescriptor>> actions_vector;

    for (const auto *actionSymbol : actions) {
        TU_RETURN_IF_NOT_OK (write_action(actionSymbol, writer, buffer, actions_vector, symbols_vector));
    }

    // create the actions vector
    actionsOffset = buffer.CreateVector(actions_vector);

    return {};
}
