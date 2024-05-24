
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

    std::vector<lyo1::Parameter> parameters;
    std::vector<std::string> names;

    for (const auto &param : actionSymbol->getParameters()) {
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
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid placement type");
        }
        if (param.isVariable)
            flags |= lyo1::ParameterFlags::Var;
        lyric_assembler::TypeHandle *paramTypeHandle;
        TU_ASSIGN_OR_RETURN (paramTypeHandle, typeCache->getOrMakeType(param.typeDef));
        tu_uint32 paramType = paramTypeHandle->getAddress().getAddress();
        tu_uint32 paramDefault = lyric_runtime::INVALID_ADDRESS_U32;
        if (actionSymbol->hasInitializer(param.name)) {
            if (param.placement != lyric_object::PlacementType::Opt)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant, "invalid placement");
            auto initializerUrl = actionSymbol->getInitializer(param.name);
            lyric_assembler::AbstractSymbol *initializer;
            TU_ASSIGN_OR_RETURN (initializer, symbolCache->getOrImportSymbol(initializerUrl));
            if (initializer->getSymbolType() != lyric_assembler::SymbolType::CALL)
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "invalid initializer {}", initializerUrl.toString());
            auto *initSymbol = cast_symbol_to_call(initializer);
            paramDefault = initSymbol->getAddress().getAddress();
        }
        uint8_t nameOffset = names.size();
        names.emplace_back(param.name);
        parameters.emplace_back(lyo1::Parameter(flags, paramType,
            paramDefault, nameOffset, lyric_runtime::INVALID_OFFSET_U8));
    }

    if (!typeCache->hasType(actionSymbol->getReturnType()))
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kAssemblerInvariant, "missing action result type");
    lyric_assembler::TypeHandle *returnTypeHandle;
    TU_ASSIGN_OR_RETURN (returnTypeHandle, typeCache->getOrMakeType(actionSymbol->getReturnType()));
    tu_uint32 resultType = returnTypeHandle->getAddress().getAddress();

    auto restParam = actionSymbol->getRest();
    if (!restParam.isEmpty()) {
        const auto &param = restParam.getValue();
        lyo1::ParameterFlags flags = lyo1::ParameterFlags::Rest;
        lyric_assembler::TypeHandle *paramTypeHandle;
        TU_ASSIGN_OR_RETURN (paramTypeHandle, typeCache->getOrMakeType(param.typeDef));
        tu_uint32 paramType = paramTypeHandle->getAddress().getAddress();
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
        actions_vector.push_back(lyo1::CreateActionDescriptor(buffer, fullyQualifiedName,
            actionTemplate, receiverSection, receiverDescriptor, actionFlags,
            buffer.CreateVectorOfStructs(parameters), &rest,
            buffer.CreateVectorOfStrings(names), resultType));
    } else {
        actions_vector.push_back(lyo1::CreateActionDescriptor(buffer, fullyQualifiedName,
            actionTemplate, receiverSection, receiverDescriptor, actionFlags,
            buffer.CreateVectorOfStructs(parameters), nullptr,
            buffer.CreateVectorOfStrings(names), resultType));
    }

    // add symbol descriptor
    symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fullyQualifiedName,
        lyo1::DescriptorSection::Action, index));

    return {};
}

tempo_utils::Status
lyric_assembler::internal::write_actions(
    const AssemblyState *assemblyState,
    flatbuffers::FlatBufferBuilder &buffer,
    ActionsOffset &actionsOffset,
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> &symbols_vector)
{
    TU_ASSERT (assemblyState != nullptr);

    SymbolCache *symbolCache = assemblyState->symbolCache();
    TypeCache *typeCache = assemblyState->typeCache();
    std::vector<flatbuffers::Offset<lyo1::ActionDescriptor>> actions_vector;

    for (auto iterator = assemblyState->actionsBegin(); iterator != assemblyState->actionsEnd(); iterator++) {
        auto &actionSymbol = *iterator;
        TU_RETURN_IF_NOT_OK (
            write_action(actionSymbol, typeCache, symbolCache, buffer, actions_vector, symbols_vector));
    }

    // create the actions vector
    actionsOffset = buffer.CreateVector(actions_vector);

    return {};
}
