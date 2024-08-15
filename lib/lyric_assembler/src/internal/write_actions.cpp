
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/internal/write_actions.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

static tempo_utils::Status
write_action(
    lyric_assembler::ActionSymbol *actionSymbol,
    lyric_assembler::TypeCache *typeCache,
    lyric_assembler::SymbolCache *symbolCache,
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
    if (actionSymbol->actionTemplate() != nullptr)
        actionTemplate = actionSymbol->actionTemplate()->getAddress().getAddress();

    auto receiverUrl = actionSymbol->getReceiverUrl();
    lyric_assembler::AbstractSymbol *receiver;
    TU_ASSIGN_OR_RETURN (receiver, symbolCache->getOrImportSymbol(receiverUrl));

    lyo1::TypeSection receiverSection = lyo1::TypeSection::Invalid;
    tu_uint32 receiverDescriptor = lyric_runtime::INVALID_ADDRESS_U32;
    switch (receiver->getSymbolType()) {
        case lyric_assembler::SymbolType::CONCEPT:
            receiverSection = lyo1::TypeSection::Concept;
            receiverDescriptor = cast_symbol_to_concept(receiver)->getAddress().getAddress();
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

        lyric_assembler::TypeHandle *paramTypeHandle;
        TU_ASSIGN_OR_RETURN (paramTypeHandle, typeCache->getOrMakeType(param.typeDef));
        p.parameter_type = paramTypeHandle->getAddress().getAddress();
        p.initializer_call = lyric_runtime::INVALID_ADDRESS_U32;
        if (actionSymbol->hasInitializer(param.name)) {
            if (param.placement != lyric_object::PlacementType::ListOpt)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid placement");
            auto initializerUrl = actionSymbol->getInitializer(param.name);
            lyric_assembler::AbstractSymbol *initializer;
            TU_ASSIGN_OR_RETURN (initializer, symbolCache->getOrImportSymbol(initializerUrl));
            if (initializer->getSymbolType() != lyric_assembler::SymbolType::CALL)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid initializer {}", initializerUrl.toString());
            auto *initSymbol = cast_symbol_to_action(initializer);
            p.initializer_call = initSymbol->getAddress().getAddress();
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

        lyric_assembler::TypeHandle *paramTypeHandle;
        TU_ASSIGN_OR_RETURN (paramTypeHandle, typeCache->getOrMakeType(param.typeDef));
        p.parameter_type = paramTypeHandle->getAddress().getAddress();
        p.initializer_call = lyric_runtime::INVALID_ADDRESS_U32;
        if (actionSymbol->hasInitializer(param.name)) {
            if (param.placement != lyric_object::PlacementType::NamedOpt)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid placement");
            auto initializerUrl = actionSymbol->getInitializer(param.name);
            lyric_assembler::AbstractSymbol *initializer;
            TU_ASSIGN_OR_RETURN (initializer, symbolCache->getOrImportSymbol(initializerUrl));
            if (initializer->getSymbolType() != lyric_assembler::SymbolType::CALL)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid initializer {}", initializerUrl.toString());
            auto *initSymbol = cast_symbol_to_action(initializer);
            p.initializer_call = initSymbol->getAddress().getAddress();
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

        lyric_assembler::TypeHandle *paramTypeHandle;
        TU_ASSIGN_OR_RETURN (paramTypeHandle, typeCache->getOrMakeType(rest->typeDef));
        p.parameter_type = paramTypeHandle->getAddress().getAddress();
        p.initializer_call = 0;

        fb_restParameter = lyo1::CreateParameter(buffer, &p);
    }

    lyric_assembler::TypeHandle *returnTypeHandle;
    TU_ASSIGN_OR_RETURN (returnTypeHandle, typeCache->getOrMakeType(actionSymbol->getReturnType()));
    tu_uint32 resultType = returnTypeHandle->getAddress().getAddress();

    // add action descriptor
    actions_vector.push_back(lyo1::CreateActionDescriptor(buffer, fullyQualifiedName,
        actionTemplate, receiverSection, receiverDescriptor, actionFlags,
        fb_listParameters, fb_namedParameters, fb_restParameter, resultType));

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Action, index));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_actions(
    const ObjectState *objectState,
    flatbuffers::FlatBufferBuilder &buffer,
    ActionsOffset &actionsOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    TU_ASSERT (objectState != nullptr);

    SymbolCache *symbolCache = objectState->symbolCache();
    TypeCache *typeCache = objectState->typeCache();
    std::vector<flatbuffers::Offset<lyo1::ActionDescriptor>> actions_vector;

    for (auto iterator = objectState->actionsBegin(); iterator != objectState->actionsEnd(); iterator++) {
        auto &actionSymbol = *iterator;
        TU_RETURN_IF_NOT_OK (
            write_action(actionSymbol, typeCache, symbolCache, buffer, actions_vector, symbols_vector));
    }

    // create the actions vector
    actionsOffset = buffer.CreateVector(actions_vector);

    return {};
}
