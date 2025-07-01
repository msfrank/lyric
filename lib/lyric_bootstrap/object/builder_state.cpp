
#include <lyric_common/symbol_url.h>
#include <tempo_utils/bytes_appender.h>

#include "builder_state.h"

static void
process_parameters(
    const std::vector<CoreParam> &parameterSpec,
    std::vector<lyo1::ParameterT> &listParameters,
    std::vector<lyo1::ParameterT> &namedParameters,
    Option<lyo1::ParameterT> &restParameter)
{
    absl::flat_hash_set<std::string> usedNames;
    bool encounteredListOptParameter = false;

    // process list and listopt parameters
    auto it = parameterSpec.cbegin();
    for (; it != parameterSpec.cend(); it++) {
        TU_ASSERT (!usedNames.contains(it->paramName));

        lyo1::ParameterT p;
        p.parameter_name = it->paramName;
        p.parameter_type = it->paramType->type_index;
        if (it->isVariable) {
            p.flags |= lyo1::ParameterFlags::Var;
        }

        if (it->paramPlacement == lyric_object::PlacementType::List) {
            TU_ASSERT (it->isCtx == false);
            TU_ASSERT (!encounteredListOptParameter);
            p.initializer_call = lyric_object::INVALID_ADDRESS_U32;
        }
        else if (it->paramPlacement == lyric_object::PlacementType::ListOpt) {
            TU_ASSERT (it->isCtx == false);
            TU_ASSERT (it->paramDefault != nullptr);
            p.initializer_call = it->paramDefault->call_index;
            encounteredListOptParameter = true;
        } else {
            break;
        }

        usedNames.insert(it->paramName);
        listParameters.push_back(p);
    }

    // process named, namedopt, and ctx parameters
    for (; it != parameterSpec.cend(); it++) {
        TU_ASSERT (it->paramPlacement != lyric_object::PlacementType::List);
        TU_ASSERT (it->paramPlacement != lyric_object::PlacementType::ListOpt);
        TU_ASSERT (!usedNames.contains(it->paramName));

        lyo1::ParameterT p;
        p.parameter_name = it->paramName;
        p.parameter_type = it->paramType->type_index;
        if (it->isVariable) {
            p.flags |= lyo1::ParameterFlags::Var;
        }

        if (it->paramPlacement == lyric_object::PlacementType::Named) {
            p.initializer_call = lyric_object::INVALID_ADDRESS_U32;
        } else if (it->paramPlacement == lyric_object::PlacementType::NamedOpt) {
            TU_ASSERT (it->paramDefault != nullptr);
            p.initializer_call = it->paramDefault->call_index;
        } else if (it->paramPlacement == lyric_object::PlacementType::Ctx) {
            p.initializer_call = lyric_object::INVALID_ADDRESS_U32;
            p.flags |= lyo1::ParameterFlags::Ctx;
        } else {
            break;
        }

        usedNames.insert(it->paramName);
        namedParameters.push_back(p);
    }

    // process rest parameter
    if (it != parameterSpec.cend()) {
        TU_ASSERT (it->paramPlacement == lyric_object::PlacementType::Rest);
        TU_ASSERT (it->paramDefault == nullptr);
        TU_ASSERT (it->isVariable == false);
        TU_ASSERT (it->isCtx == false);
        TU_ASSERT (it->paramName.empty());

        lyo1::ParameterT p;
        p.parameter_type = it->paramType->type_index;
        p.initializer_call = lyric_object::INVALID_ADDRESS_U32;
        restParameter = Option(p);

        it++;
    }

    TU_ASSERT (it == parameterSpec.cend());
}

CoreParam make_list_param(std::string_view name, const CoreType *type, bool isVariable)
{
    return {std::string(name), lyric_object::PlacementType::List, type, nullptr, isVariable, false};
}

CoreParam make_named_param(std::string_view name, const CoreType *type, bool isVariable)
{
    return {std::string(name), lyric_object::PlacementType::Named, type, nullptr, isVariable, false};
}

CoreParam make_named_opt_param(std::string_view name, const CoreType *type, const CoreCall *dfl, bool isVariable)
{
    return {std::string(name), lyric_object::PlacementType::NamedOpt, type, dfl, isVariable, false};
}

CoreParam make_ctx_param(std::string_view name, const CoreType *type)
{
    return {std::string(name), lyric_object::PlacementType::Ctx, type, nullptr, false, true};
}

CoreParam make_rest_param(const CoreType *type)
{
    return {{}, lyric_object::PlacementType::Rest, type, nullptr, false, false};
}

BuilderState::BuilderState(
    const lyric_common::ModuleLocation &location,
    std::shared_ptr<const lyric_runtime::TrapIndex> trapIndex)
    : location(location),
      trapIndex(std::move(trapIndex))
{
    TU_ASSERT (this->location.isValid());
    TU_ASSERT (this->trapIndex != nullptr);

    noReturnType = new CoreType();
    noReturnType->type_index = types.size();
    noReturnType->superType = nullptr;
    noReturnType->typeAssignable = lyo1::Assignable::SpecialAssignable;
    noReturnType->specialType = lyo1::SpecialType::NoReturn;

    types.push_back(noReturnType);
}

CoreType *
BuilderState::addConcreteType(
    const CoreType *superType,
    lyo1::TypeSection concreteSection,
    tu_uint32 concreteDescriptor,
    const std::vector<const CoreType *> &parameters)
{
    auto *ConcreteType = new CoreType();
    ConcreteType->type_index = types.size();
    ConcreteType->superType = superType;
    ConcreteType->typeAssignable = lyo1::Assignable::ConcreteAssignable;
    ConcreteType->concreteSection = concreteSection;
    ConcreteType->concreteDescriptor = concreteDescriptor;

    for (const auto *p : parameters) {
        ConcreteType->typeParameters.emplace_back(p->type_index);
    }

    types.push_back(ConcreteType);
    return ConcreteType;
}

CoreType *
BuilderState::addUnionType(const std::vector<const CoreType *> &unionMembers)
{
    auto *UnionType = new CoreType();
    UnionType->type_index = types.size();
    UnionType->superType = nullptr;
    UnionType->typeAssignable = lyo1::Assignable::UnionAssignable;
    UnionType->concreteSection = lyo1::TypeSection::Invalid;
    UnionType->concreteDescriptor = lyric_object::INVALID_ADDRESS_U32;

    for (const auto *p : unionMembers) {
        UnionType->typeParameters.emplace_back(p->type_index);
    }

    types.push_back(UnionType);
    return UnionType;
}

CoreTemplate *
BuilderState::addTemplate(
    const lyric_common::SymbolPath &templatePath,
    const std::vector<CorePlaceholder> &placeholders,
    const std::vector<CoreConstraint> &constraints)
{
    TU_ASSERT (templatePath.isValid());

    absl::flat_hash_map<std::string,uint8_t> placeholdersIndex;

    auto *Template = new CoreTemplate();
    Template->template_index = templates.size();
    Template->templatePath = templatePath;

    for (tu_uint32 i = 0; i < placeholders.size(); i++) {
        const auto &p = placeholders[i];
        auto *PlaceholderType = new CoreType();
        PlaceholderType->type_index = types.size();
        PlaceholderType->superType = nullptr;
        PlaceholderType->typeAssignable = lyo1::Assignable::PlaceholderAssignable;
        PlaceholderType->placeholderTemplate = Template;
        PlaceholderType->placeholderIndex = i;
        types.push_back(PlaceholderType);
        Template->types[p.name] = PlaceholderType;
        uint8_t nameOffset = Template->names.size();
        Template->names.push_back(p.name);
        Template->placeholders.push_back({p.variance, nameOffset});
        placeholdersIndex[p.name] = i;
    }

    for (const auto &c : constraints) {
        auto placeholderOffset = placeholdersIndex[c.name];
        Template->constraints.push_back({placeholderOffset, c.bound, c.type->type_index});
    }

    templates.push_back(Template);
    return Template;
}

void
BuilderState::writeTrap(lyric_object::BytecodeBuilder &code, std::string_view trapName, tu_uint8 flags)
{
    auto trapNumber = trapIndex->lookupTrap(trapName);
    TU_ASSERT (trapNumber != lyric_runtime::INVALID_ADDRESS_U32);
    auto status = code.trap(trapNumber, flags);
    TU_RAISE_IF_NOT_OK (status);
}

CoreExistential *
BuilderState::addExistential(
    const lyric_common::SymbolPath &existentialPath,
    lyo1::IntrinsicType intrinsicMapping,
    lyo1::ExistentialFlags existentialFlags,
    const CoreExistential *superExistential)
{
    TU_ASSERT (!symboltable.contains(existentialPath));

    tu_uint32 existential_index = existentials.size();
    auto *Existential = new CoreExistential();
    existentials.push_back(Existential);

    const CoreType *superType = superExistential? superExistential->existentialType : nullptr;
    auto *Type = addConcreteType(superType, lyo1::TypeSection::Existential, existential_index);

    Existential->existential_index = existential_index;
    Existential->existentialPath = existentialPath;
    Existential->existentialTemplate = nullptr;
    Existential->superExistential = superExistential;
    Existential->intrinsicMapping = intrinsicMapping;
    Existential->existentialType = Type;
    Existential->flags = existentialFlags;

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Existential;
    Symbol->index = Existential->existential_index;
    symbols.push_back(Symbol);
    symboltable[Existential->existentialPath] = symbol_index;

    existentialcache[Existential->existentialPath] = Existential;

    return Existential;
}

CoreExistential *
BuilderState::addGenericExistential(
    const lyric_common::SymbolPath &existentialPath,
    const CoreTemplate *existentialTemplate,
    lyo1::IntrinsicType intrinsicMapping,
    lyo1::ExistentialFlags existentialFlags,
    const CoreExistential *superExistential)
{
    TU_ASSERT (!symboltable.contains(existentialPath));
    TU_ASSERT (existentialTemplate != nullptr);

    tu_uint32 existential_index = existentials.size();
    auto *Existential = new CoreExistential();
    existentials.push_back(Existential);

    const CoreType *superType = superExistential? superExistential->existentialType : nullptr;
    std::vector<const CoreType *> typeParameters;
    for (const auto &name : existentialTemplate->names) {
        typeParameters.push_back(existentialTemplate->types.at(name));
    }
    auto *Type = addConcreteType(superType, lyo1::TypeSection::Existential, existential_index, typeParameters);

    Existential->existential_index = existential_index;
    Existential->existentialPath = existentialPath;
    Existential->existentialTemplate = existentialTemplate;
    Existential->superExistential = superExistential;
    Existential->intrinsicMapping = intrinsicMapping;
    Existential->existentialType = Type;
    Existential->flags = existentialFlags;

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Existential;
    Symbol->index = Existential->existential_index;
    symbols.push_back(Symbol);
    symboltable[Existential->existentialPath] = symbol_index;

    existentialcache[Existential->existentialPath] = Existential;

    return Existential;
}

CoreCall *
BuilderState::addExistentialMethod(
    const std::string &methodName,
    const CoreExistential *receiver,
    lyo1::CallFlags callFlags,
    const std::vector<CoreParam> &parameters,
    const lyric_object::BytecodeBuilder &code,
    const CoreType *returnType,
    bool isInline)
{
    TU_ASSERT (!methodName.empty());
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (existentialcache.contains(receiver->existentialPath));
    TU_ASSERT (returnType != nullptr);
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    auto *ReceiverExistential = existentialcache[receiver->existentialPath];
    lyric_common::SymbolPath callPath(ReceiverExistential->existentialPath.getPath(), methodName);
    TU_ASSERT (!symboltable.contains(callPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    // ensure Bound is set
    callFlags |= lyo1::CallFlags::Bound;

    // FIXME: isInline is ignored

    // if returnType is noReturnType then set NoReturn
    if (returnType == noReturnType) {
        callFlags |= lyo1::CallFlags::NoReturn;
    }

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = callPath;
    Call->callTemplate = ReceiverExistential->existentialTemplate;
    Call->callType = FunctionClass->classType;
    Call->receiver_symbol_index = getSymbolIndex(receiver->existentialPath);
    Call->flags = callFlags;
    process_parameters(parameters, Call->listParameters, Call->namedParameters, Call->restParameter);
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    ReceiverExistential->methods.push_back(Call);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols.push_back(Symbol);
    symboltable[Call->callPath] = symbol_index;

    return Call;
}

void
BuilderState::addExistentialSealedSubtype(const CoreExistential *receiver, const CoreExistential *subtypeExistential)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (existentialcache.contains(receiver->existentialPath));
    TU_ASSERT (subtypeExistential != nullptr);
    TU_ASSERT (existentialcache.contains(subtypeExistential->existentialPath));

    auto *ReceiverExistential = existentialcache[receiver->existentialPath];
    TU_ASSERT (bool(ReceiverExistential->flags & lyo1::ExistentialFlags::Sealed));
    auto *SubTypeExistential = existentialcache[subtypeExistential->existentialPath];
    TU_ASSERT (bool(SubTypeExistential->flags & lyo1::ExistentialFlags::Final));
    TU_ASSERT (SubTypeExistential->superExistential == ReceiverExistential);
    ReceiverExistential->sealedSubtypes.push_back(SubTypeExistential->existentialType->type_index);
}

CoreCall *
BuilderState::addFunction(
    const lyric_common::SymbolPath &functionPath,
    const std::vector<CoreParam> &parameters,
    const lyric_object::BytecodeBuilder &code,
    const CoreType *returnType,
    bool isInline)
{
    TU_ASSERT (functionPath.isValid());
    TU_ASSERT (!symboltable.contains(functionPath));
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    auto callFlags = lyo1::CallFlags::NONE;

    if (isInline) {
        callFlags |= lyo1::CallFlags::Inline;
    }

    // if returnType is noReturnType then set NoReturn
    if (returnType == noReturnType) {
        callFlags |= lyo1::CallFlags::NoReturn;
    }

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = functionPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiver_symbol_index = lyric_object::INVALID_ADDRESS_U32;
    Call->flags = callFlags;
    process_parameters(parameters, Call->listParameters, Call->namedParameters, Call->restParameter);
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols.push_back(Symbol);
    symboltable[Call->callPath] = symbol_index;

    return Call;
}

CoreCall *
BuilderState::addGenericFunction(
    const lyric_common::SymbolPath &functionPath,
    const CoreTemplate *functionTemplate,
    const std::vector<CoreParam> &parameters,
    const lyric_object::BytecodeBuilder &code,
    const CoreType *returnType,
    bool isInline)
{
    TU_ASSERT (functionPath.isValid());
    TU_ASSERT (!symboltable.contains(functionPath));
    TU_ASSERT (functionTemplate != nullptr);
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    auto callFlags = lyo1::CallFlags::NONE;
    if (isInline)
        callFlags |= lyo1::CallFlags::Inline;

    // if returnType is noReturnType then set NoReturn
    if (returnType == noReturnType) {
        callFlags |= lyo1::CallFlags::NoReturn;
    }

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = functionPath;
    Call->callTemplate = functionTemplate;
    Call->callType = FunctionClass->classType;
    Call->receiver_symbol_index = lyric_object::INVALID_ADDRESS_U32;
    Call->flags = callFlags;
    process_parameters(parameters, Call->listParameters, Call->namedParameters, Call->restParameter);
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols.push_back(Symbol);
    symboltable[Call->callPath] = symbol_index;

    return Call;
}

CoreConcept *
BuilderState::addConcept(
    const lyric_common::SymbolPath &conceptPath,
    lyo1::ConceptFlags conceptFlags,
    const CoreConcept *superConcept)
{
    TU_ASSERT (!symboltable.contains(conceptPath));

    tu_uint32 concept_index = concepts.size();
    auto *Concept = new CoreConcept();
    concepts.push_back(Concept);

    const CoreType *superType = superConcept? superConcept->conceptType : nullptr;
    auto *Type = addConcreteType(superType, lyo1::TypeSection::Concept, concept_index);

    Concept->concept_index = concept_index;
    Concept->conceptPath = conceptPath;
    Concept->conceptTemplate = nullptr;
    Concept->superConcept = superConcept;
    Concept->conceptType = Type;
    Concept->flags = conceptFlags;

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Concept;
    Symbol->index = Concept->concept_index;
    symbols.push_back(Symbol);
    symboltable[Concept->conceptPath] = symbol_index;

    conceptcache[Concept->conceptPath] = Concept;

    return Concept;
}

CoreConcept *
BuilderState::addGenericConcept(
    const lyric_common::SymbolPath &conceptPath,
    const CoreTemplate *conceptTemplate,
    lyo1::ConceptFlags conceptFlags,
    const CoreConcept *superConcept)
{
    TU_ASSERT (!symboltable.contains(conceptPath));
    TU_ASSERT (conceptTemplate != nullptr);

    tu_uint32 concept_index = concepts.size();
    auto *Concept = new CoreConcept();
    concepts.push_back(Concept);

    const CoreType *superType = superConcept? superConcept->conceptType : nullptr;
    std::vector<const CoreType *> typeParameters;
    for (const auto &name : conceptTemplate->names) {
        typeParameters.push_back(conceptTemplate->types.at(name));
    }
    auto *Type = addConcreteType(superType, lyo1::TypeSection::Concept, concept_index, typeParameters);

    Concept->concept_index = concept_index;
    Concept->conceptPath = conceptPath;
    Concept->conceptTemplate = conceptTemplate;
    Concept->superConcept = superConcept;
    Concept->conceptType = Type;
    Concept->flags = conceptFlags;

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Concept;
    Symbol->index = Concept->concept_index;
    symbols.push_back(Symbol);
    symboltable[Concept->conceptPath] = symbol_index;

    conceptcache[Concept->conceptPath] = Concept;

    return Concept;
}

CoreAction *
BuilderState::addConceptAction(
    const std::string &actionName,
    const CoreConcept *receiver,
    const std::vector<CoreParam> &parameters,
    const CoreType *returnType)
{
    TU_ASSERT (!actionName.empty());
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (conceptcache.contains(receiver->conceptPath));
    TU_ASSERT (returnType != nullptr);

    auto *ReceiverConcept = conceptcache[receiver->conceptPath];
    lyric_common::SymbolPath actionPath(ReceiverConcept->conceptPath.getPath(), actionName);
    TU_ASSERT (!symboltable.contains(actionPath));

    auto *Action = new CoreAction();
    Action->action_index = actions.size();
    Action->actionPath = actionPath;
    Action->actionTemplate = ReceiverConcept->conceptTemplate;
    Action->receiver_symbol_index = getSymbolIndex(receiver->conceptPath);
    Action->returnType = returnType;
    Action->flags = lyo1::ActionFlags::NONE;
    process_parameters(parameters, Action->listParameters, Action->namedParameters, Action->restParameter);
    actions.push_back(Action);

    ReceiverConcept->actions.push_back(Action);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Action;
    Symbol->index = Action->action_index;
    symbols.push_back(Symbol);
    symboltable[Action->actionPath] = symbol_index;

    return Action;
}

void
BuilderState::addConceptSealedSubtype(const CoreConcept *receiver, const CoreConcept *subtypeConcept)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (conceptcache.contains(receiver->conceptPath));
    TU_ASSERT (subtypeConcept != nullptr);
    TU_ASSERT (conceptcache.contains(subtypeConcept->conceptPath));

    auto *ReceiverConcept = conceptcache[receiver->conceptPath];
    TU_ASSERT (bool(ReceiverConcept->flags & lyo1::ConceptFlags::Sealed));
    auto *SubTypeConcept = conceptcache[subtypeConcept->conceptPath];
    TU_ASSERT (bool(SubTypeConcept->flags & lyo1::ConceptFlags::Final));
    TU_ASSERT (SubTypeConcept->superConcept == ReceiverConcept);
    ReceiverConcept->sealedSubtypes.push_back(SubTypeConcept->conceptType->type_index);
}

CoreClass *
BuilderState::addClass(
    const lyric_common::SymbolPath &classPath,
    lyo1::ClassFlags classFlags,
    const CoreClass *superClass)
{
    TU_ASSERT (!symboltable.contains(classPath));

    tu_uint32 class_index = classes.size();
    auto *Class = new CoreClass();
    classes.push_back(Class);

    const CoreType *superType = superClass? superClass->classType : nullptr;
    auto *Type = addConcreteType(superType, lyo1::TypeSection::Class, class_index);

    Class->class_index = class_index;
    Class->classPath = classPath;
    Class->classTemplate = nullptr;
    Class->superClass = superClass;
    Class->classType = Type;
    Class->flags = classFlags;
    Class->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    Class->classCtor = nullptr;

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Class;
    Symbol->index = Class->class_index;
    symbols.push_back(Symbol);
    symboltable[Class->classPath] = symbol_index;

    classcache[Class->classPath] = Class;

    return Class;
}

CoreClass *
BuilderState::addGenericClass(
    const lyric_common::SymbolPath &classPath,
    const CoreTemplate *classTemplate,
    lyo1::ClassFlags classFlags,
    const CoreClass *superClass)
{
    TU_ASSERT (!symboltable.contains(classPath));
    TU_ASSERT (classTemplate != nullptr);

    tu_uint32 class_index = classes.size();
    auto *Class = new CoreClass();
    classes.push_back(Class);

    const CoreType *superType = superClass? superClass->classType : nullptr;
    std::vector<const CoreType *> typeParameters;
    for (const auto &name : classTemplate->names) {
        typeParameters.push_back(classTemplate->types.at(name));
    }
    auto *Type = addConcreteType(superType, lyo1::TypeSection::Class, class_index, typeParameters);

    Class->class_index = class_index;
    Class->classPath = classPath;
    Class->classTemplate = classTemplate;
    Class->superClass = superClass;
    Class->classType = Type;
    Class->flags = classFlags;
    Class->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    Class->classCtor = nullptr;

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Class;
    Symbol->index = Class->class_index;
    symbols.push_back(Symbol);
    symboltable[Class->classPath] = symbol_index;

    classcache[Class->classPath] = Class;

    return Class;
}

void
BuilderState::setClassAllocator(const CoreClass *receiver, std::string_view trapName)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (classcache.contains(receiver->classPath));

    auto *ReceiverClass = classcache[receiver->classPath];
    TU_ASSERT (ReceiverClass->allocatorTrap == lyric_object::INVALID_ADDRESS_U32);
    auto trapNumber = trapIndex->lookupTrap(trapName);
    TU_ASSERT (trapNumber != lyric_runtime::INVALID_ADDRESS_U32);
    ReceiverClass->allocatorTrap = trapNumber;
}

CoreCall *
BuilderState::addClassCtor(
    const CoreClass *receiver,
    const std::vector<CoreParam> &parameters,
    const lyric_object::BytecodeBuilder &code)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (receiver->classCtor == nullptr);
    TU_ASSERT (classcache.contains(receiver->classPath));
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    auto *ReceiverClass = classcache[receiver->classPath];
    lyric_common::SymbolPath ctorPath(ReceiverClass->classPath.getPath(), "$ctor");
    TU_ASSERT (!symboltable.contains(ctorPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];


    auto ctorFlags = lyo1::CallFlags::Bound | lyo1::CallFlags::Ctor;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = ctorPath;
    Call->callTemplate = ReceiverClass->classTemplate;
    Call->callType = FunctionClass->classType;
    Call->receiver_symbol_index = getSymbolIndex(receiver->classPath);
    Call->flags = ctorFlags;
    process_parameters(parameters, Call->listParameters, Call->namedParameters, Call->restParameter);
    Call->code = code;
    Call->returnType = ReceiverClass->classType;
    calls.push_back(Call);

    ReceiverClass->classCtor = Call;
    ReceiverClass->methods.push_back(Call);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols.push_back(Symbol);
    symboltable[Call->callPath] = symbol_index;

    return Call;
}

CoreField *
BuilderState::addClassMember(
    const std::string &memberName,
    const CoreClass *receiver,
    lyo1::FieldFlags fieldFlags,
    const CoreType *memberType)
{
    TU_ASSERT (!memberName.empty());
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (classcache.contains(receiver->classPath));
    TU_ASSERT (memberType != nullptr);

    auto *ReceiverClass = classcache[receiver->classPath];
    lyric_common::SymbolPath fieldPath(ReceiverClass->classPath.getPath(), memberName);
    TU_ASSERT (!symboltable.contains(fieldPath));

    auto *Field = new CoreField();
    Field->field_index = fields.size();
    Field->fieldPath = fieldPath;
    Field->fieldType = memberType;
    Field->flags = fieldFlags;
    fields.push_back(Field);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Field;
    Symbol->index = Field->field_index;
    symbols.push_back(Symbol);
    symboltable[Field->fieldPath] = symbol_index;

    ReceiverClass->members.push_back(Field);

    return Field;
}

CoreCall *
BuilderState::addClassMethod(
    const std::string &methodName,
    const CoreClass *receiver,
    lyo1::CallFlags callFlags,
    const std::vector<CoreParam> &parameters,
    const lyric_object::BytecodeBuilder &code,
    const CoreType *returnType,
    bool isInline)
{
    TU_ASSERT (!methodName.empty());
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (classcache.contains(receiver->classPath));
    TU_ASSERT (returnType != nullptr);
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    auto *ReceiverClass = classcache[receiver->classPath];
    lyric_common::SymbolPath callPath(ReceiverClass->classPath.getPath(), methodName);
    TU_ASSERT (!symboltable.contains(callPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    // ensure Bound is set
    callFlags |= lyo1::CallFlags::Bound;

    // FIXME: isInline is ignored

    // if returnType is noReturnType then set NoReturn
    if (returnType == noReturnType) {
        callFlags |= lyo1::CallFlags::NoReturn;
    }

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = callPath;
    Call->callTemplate = ReceiverClass->classTemplate;
    Call->callType = FunctionClass->classType;
    Call->receiver_symbol_index = getSymbolIndex(receiver->classPath);
    Call->flags = callFlags;
    process_parameters(parameters, Call->listParameters, Call->namedParameters, Call->restParameter);
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    ReceiverClass->methods.push_back(Call);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols.push_back(Symbol);
    symboltable[Call->callPath] = symbol_index;

    return Call;
}

void
BuilderState::addClassSealedSubtype(const CoreClass *receiver, const CoreClass *subtypeClass)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (classcache.contains(receiver->classPath));
    TU_ASSERT (subtypeClass != nullptr);
    TU_ASSERT (classcache.contains(subtypeClass->classPath));

    auto *ReceiverClass = classcache[receiver->classPath];
    TU_ASSERT (bool(ReceiverClass->flags & lyo1::ClassFlags::Sealed));
    auto *SubTypeClass = classcache[subtypeClass->classPath];
    TU_ASSERT (bool(SubTypeClass->flags & lyo1::ClassFlags::Final));
    TU_ASSERT (SubTypeClass->superClass == ReceiverClass);
    ReceiverClass->sealedSubtypes.push_back(SubTypeClass->classType->type_index);
}

CoreStruct *
BuilderState::addStruct(
    const lyric_common::SymbolPath &structPath,
    lyo1::StructFlags structFlags,
    const CoreStruct *superStruct)
{
    TU_ASSERT (!symboltable.contains(structPath));

    tu_uint32 struct_index = structs.size();
    auto *Struct = new CoreStruct();
    structs.push_back(Struct);

    const CoreType *superType = superStruct? superStruct->structType : nullptr;
    auto *Type = addConcreteType(superType, lyo1::TypeSection::Struct, struct_index);

    Struct->struct_index = struct_index;
    Struct->structPath = structPath;
    Struct->superStruct = superStruct;
    Struct->structType = Type;
    Struct->flags = structFlags;
    Struct->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    Struct->structCtor = nullptr;

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Struct;
    Symbol->index = Struct->struct_index;
    symbols.push_back(Symbol);
    symboltable[Struct->structPath] = symbol_index;

    structcache[Struct->structPath] = Struct;

    return Struct;
}

void
BuilderState::setStructAllocator(const CoreStruct *receiver, std::string_view trapName)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (structcache.contains(receiver->structPath));

    auto *ReceiverStruct = structcache[receiver->structPath];
    TU_ASSERT (ReceiverStruct->allocatorTrap == lyric_object::INVALID_ADDRESS_U32);
    auto trapNumber = trapIndex->lookupTrap(trapName);
    TU_ASSERT (trapNumber != lyric_runtime::INVALID_ADDRESS_U32);
    ReceiverStruct->allocatorTrap = trapNumber;
}

CoreCall *
BuilderState::addStructCtor(
    const CoreStruct *receiver,
    const std::vector<CoreParam> &parameters,
    const lyric_object::BytecodeBuilder &code)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (receiver->structCtor == nullptr);
    TU_ASSERT (structcache.contains(receiver->structPath));
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    auto *ReceiverStruct = structcache[receiver->structPath];
    lyric_common::SymbolPath ctorPath(ReceiverStruct->structPath.getPath(), "$ctor");
    TU_ASSERT (!symboltable.contains(ctorPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    auto ctorFlags = lyo1::CallFlags::Bound | lyo1::CallFlags::Ctor;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = ctorPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiver_symbol_index = getSymbolIndex(receiver->structPath);
    Call->flags = ctorFlags;
    process_parameters(parameters, Call->listParameters, Call->namedParameters, Call->restParameter);
    Call->code = code;
    Call->returnType = ReceiverStruct->structType;
    calls.push_back(Call);

    ReceiverStruct->structCtor = Call;
    ReceiverStruct->methods.push_back(Call);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols.push_back(Symbol);
    symboltable[Call->callPath] = symbol_index;

    return Call;
}

CoreField *
BuilderState::addStructMember(
    const std::string &memberName,
    const CoreStruct *receiver,
    lyo1::FieldFlags fieldFlags,
    const CoreType *memberType)
{
    TU_ASSERT (!memberName.empty());
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (structcache.contains(receiver->structPath));
    TU_ASSERT (memberType != nullptr);

    auto *ReceiverStruct = structcache[receiver->structPath];
    lyric_common::SymbolPath fieldPath(ReceiverStruct->structPath.getPath(), memberName);
    TU_ASSERT (!symboltable.contains(fieldPath));

    auto *Field = new CoreField();
    Field->field_index = fields.size();
    Field->fieldPath = fieldPath;
    Field->fieldType = memberType;
    Field->flags = fieldFlags;
    fields.push_back(Field);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Field;
    Symbol->index = Field->field_index;
    symbols.push_back(Symbol);
    symboltable[Field->fieldPath] = symbol_index;

    ReceiverStruct->members.push_back(Field);

    return Field;
}

CoreCall *
BuilderState::addStructMethod(
    const std::string &methodName,
    const CoreStruct *receiver,
    lyo1::CallFlags callFlags,
    const std::vector<CoreParam> &parameters,
    const lyric_object::BytecodeBuilder &code,
    const CoreType *returnType,
    bool isInline)
{
    TU_ASSERT (!methodName.empty());
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (structcache.contains(receiver->structPath));
    TU_ASSERT (returnType != nullptr);
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    auto *ReceiverStruct = structcache[receiver->structPath];
    lyric_common::SymbolPath callPath(ReceiverStruct->structPath.getPath(), methodName);
    TU_ASSERT (!symboltable.contains(callPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    // ensure Bound is set
    callFlags |= lyo1::CallFlags::Bound;

    // FIXME: isInline is ignored

    // if returnType is noReturnType then set NoReturn
    if (returnType == noReturnType) {
        callFlags |= lyo1::CallFlags::NoReturn;
    }

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = callPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiver_symbol_index = getSymbolIndex(receiver->structPath);
    Call->flags = callFlags;
    process_parameters(parameters, Call->listParameters, Call->namedParameters, Call->restParameter);
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    ReceiverStruct->methods.push_back(Call);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols.push_back(Symbol);
    symboltable[Call->callPath] = symbol_index;

    return Call;
}

void
BuilderState::addStructSealedSubtype(const CoreStruct *receiver, const CoreStruct *subtypeStruct)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (structcache.contains(receiver->structPath));
    TU_ASSERT (subtypeStruct != nullptr);
    TU_ASSERT (structcache.contains(subtypeStruct->structPath));

    auto *ReceiverStruct = structcache[receiver->structPath];
    TU_ASSERT (bool(ReceiverStruct->flags & lyo1::StructFlags::Sealed));
    auto *SubTypeStruct = structcache[subtypeStruct->structPath];
    TU_ASSERT (bool(SubTypeStruct->flags & lyo1::StructFlags::Final));
    TU_ASSERT (SubTypeStruct->superStruct == ReceiverStruct);
    ReceiverStruct->sealedSubtypes.push_back(SubTypeStruct->structType->type_index);
}

CoreInstance *
BuilderState::addInstance(
    const lyric_common::SymbolPath &instancePath,
    lyo1::InstanceFlags instanceFlags,
    const CoreInstance *superInstance)
{
    TU_ASSERT (!symboltable.contains(instancePath));

    tu_uint32 instance_index = instances.size();
    auto *Instance = new CoreInstance();
    instances.push_back(Instance);

    const CoreType *superType = superInstance? superInstance->instanceType : nullptr;
    auto *Type = addConcreteType(superType, lyo1::TypeSection::Instance, instance_index);

    Instance->instance_index = instance_index;
    Instance->instancePath = instancePath;
    Instance->flags = instanceFlags;
    Instance->instanceType = Type;
    Instance->superInstance = superInstance;
    Instance->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    Instance->instanceCtor = nullptr;

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Instance;
    Symbol->index = Instance->instance_index;
    symbols.push_back(Symbol);
    symboltable[Instance->instancePath] = symbol_index;

    instancecache[Instance->instancePath] = Instance;

    return Instance;
}

void
BuilderState::setInstanceAllocator(const CoreInstance *receiver, std::string_view trapName)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (instancecache.contains(receiver->instancePath));

    auto *ReceiverInstance = instancecache[receiver->instancePath];
    TU_ASSERT (ReceiverInstance->allocatorTrap == lyric_object::INVALID_ADDRESS_U32);
    auto trapNumber = trapIndex->lookupTrap(trapName);
    TU_ASSERT (trapNumber != lyric_runtime::INVALID_ADDRESS_U32);
    ReceiverInstance->allocatorTrap = trapNumber;
}

CoreCall *
BuilderState::addInstanceCtor(
    const CoreInstance *receiver,
    const std::vector<CoreParam> &parameters,
    const lyric_object::BytecodeBuilder &code)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (receiver->instanceCtor == nullptr);
    TU_ASSERT (instancecache.contains(receiver->instancePath));
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    auto *ReceiverInstance = instancecache[receiver->instancePath];
    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    lyric_common::SymbolPath ctorPath(ReceiverInstance->instancePath.getPath(), "$ctor");
    TU_ASSERT (!symboltable.contains(ctorPath));

    auto ctorFlags = lyo1::CallFlags::Bound | lyo1::CallFlags::Ctor;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = ctorPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiver_symbol_index = getSymbolIndex(receiver->instancePath);
    Call->flags = ctorFlags;
    process_parameters(parameters, Call->listParameters, Call->namedParameters, Call->restParameter);
    Call->code = code;
    Call->returnType = ReceiverInstance->instanceType;
    calls.push_back(Call);

    ReceiverInstance->instanceCtor = Call;
    ReceiverInstance->methods.push_back(Call);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols.push_back(Symbol);
    symboltable[Call->callPath] = symbol_index;

    return Call;
}

CoreField *
BuilderState::addInstanceMember(
    const std::string &memberName,
    const CoreInstance *receiver,
    lyo1::FieldFlags fieldFlags,
    const CoreType *memberType)
{
    TU_ASSERT (!memberName.empty());
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (instancecache.contains(receiver->instancePath));
    TU_ASSERT (memberType != nullptr);

    auto *ReceiverInstance = instancecache[receiver->instancePath];

    lyric_common::SymbolPath fieldPath(ReceiverInstance->instancePath.getPath(), memberName);
    TU_ASSERT (!symboltable.contains(fieldPath));

    auto *Field = new CoreField();
    Field->field_index = fields.size();
    Field->fieldPath = fieldPath;
    Field->fieldType = memberType;
    Field->flags = fieldFlags;
    fields.push_back(Field);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Field;
    Symbol->index = Field->field_index;
    symbols.push_back(Symbol);
    symboltable[Field->fieldPath] = symbol_index;

    ReceiverInstance->members.push_back(Field);

    return Field;
}

CoreCall *
BuilderState::addInstanceMethod(
    const std::string &methodName,
    const CoreInstance *receiver,
    lyo1::CallFlags callFlags,
    const std::vector<CoreParam> &parameters,
    const lyric_object::BytecodeBuilder &code,
    const CoreType *returnType,
    bool isInline)
{
    TU_ASSERT (!methodName.empty());
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (instancecache.contains(receiver->instancePath));
    TU_ASSERT (returnType != nullptr);
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    auto *ReceiverInstance = instancecache[receiver->instancePath];

    lyric_common::SymbolPath callPath(ReceiverInstance->instancePath.getPath(), methodName);
    TU_ASSERT (!symboltable.contains(callPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    // ensure Bound is set
    callFlags = lyo1::CallFlags::Bound;

    // FIXME: isInline is ignored

    // if returnType is noReturnType then set NoReturn
    if (returnType == noReturnType) {
        callFlags |= lyo1::CallFlags::NoReturn;
    }

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = callPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiver_symbol_index = getSymbolIndex(receiver->instancePath);
    Call->flags = callFlags;
    process_parameters(parameters, Call->listParameters, Call->namedParameters, Call->restParameter);
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    ReceiverInstance->methods.push_back(Call);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols.push_back(Symbol);
    symboltable[Call->callPath] = symbol_index;

    return Call;
}

void
BuilderState::addInstanceSealedSubtype(const CoreInstance *receiver, const CoreInstance *subtypeInstance)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (instancecache.contains(receiver->instancePath));
    TU_ASSERT (subtypeInstance != nullptr);
    TU_ASSERT (instancecache.contains(subtypeInstance->instancePath));

    auto *ReceiverInstance = instancecache[receiver->instancePath];
    TU_ASSERT (bool(ReceiverInstance->flags & lyo1::InstanceFlags::Sealed));
    auto *SubTypeInstance = instancecache[subtypeInstance->instancePath];
    TU_ASSERT (bool(SubTypeInstance->flags & lyo1::InstanceFlags::Final));
    TU_ASSERT (SubTypeInstance->superInstance == ReceiverInstance);
    ReceiverInstance->sealedSubtypes.push_back(SubTypeInstance->instanceType->type_index);
}

CoreImpl *
BuilderState::addImpl(
    const lyric_common::SymbolPath &receiverPath,
    const CoreType *implType,
    const CoreConcept *implConcept)
{
    TU_ASSERT (symboltable.contains(receiverPath));
    TU_ASSERT (implType != nullptr);
    TU_ASSERT (implConcept != nullptr);

    auto *Impl = new CoreImpl();
    Impl->impl_index = impls.size();
    impls.push_back(Impl);

    Impl->receiverPath = receiverPath;
    Impl->implType = implType;
    Impl->implConcept = implConcept;
    Impl->flags = lyo1::ImplFlags::NONE;

    auto receiver_symbol_index = symboltable.at(receiverPath);
    auto *symbol = symbols.at(receiver_symbol_index);
    switch (symbol->section) {

        case lyo1::DescriptorSection::Class: {
            Impl->receiver_symbol_index = receiver_symbol_index;
            auto *receiver = classes.at(symbol->index);
            receiver->impls.push_back(Impl);
            break;
        }

        case lyo1::DescriptorSection::Concept: {
            Impl->receiver_symbol_index = receiver_symbol_index;
            auto *receiver = concepts.at(symbol->index);
            receiver->impls.push_back(Impl);
            break;
        }

        case lyo1::DescriptorSection::Enum: {
            Impl->receiver_symbol_index = receiver_symbol_index;
            auto *receiver = enums.at(symbol->index);
            receiver->impls.push_back(Impl);
            break;
        }

        case lyo1::DescriptorSection::Existential: {
            Impl->receiver_symbol_index = receiver_symbol_index;
            auto *receiver = existentials.at(symbol->index);
            receiver->impls.push_back(Impl);
            break;
        }

        case lyo1::DescriptorSection::Instance: {
            Impl->receiver_symbol_index = receiver_symbol_index;
            auto *receiver = instances.at(symbol->index);
            receiver->impls.push_back(Impl);
            break;
        }

        case lyo1::DescriptorSection::Struct: {
            Impl->receiver_symbol_index = receiver_symbol_index;
            auto *receiver = structs.at(symbol->index);
            receiver->impls.push_back(Impl);
            break;
        }

        default:
            TU_UNREACHABLE();
    }


    return Impl;
}

CoreCall *
BuilderState::addImplExtension(
    const std::string &extensionName,
    const CoreImpl *receiver,
    const std::vector<CoreParam> &parameters,
    const lyric_object::BytecodeBuilder &code,
    const CoreType *returnType,
    bool isInline)
{
    TU_ASSERT (!extensionName.empty());
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (returnType != nullptr);
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    lyric_common::SymbolPath callPath(receiver->receiverPath.getPath(),
        absl::StrCat("$impl", receiver->impl_index, "$", extensionName));
    TU_ASSERT (!symboltable.contains(callPath));

    auto *ImplConcept = receiver->implConcept;
    lyric_common::SymbolPath actionPath(ImplConcept->conceptPath.getPath(), extensionName);
    TU_ASSERT (symboltable.contains(actionPath));
    auto action_index = symboltable.at(actionPath);

    auto *ConceptAction = symbols[action_index];
    TU_ASSERT (ConceptAction->section == lyo1::DescriptorSection::Action);

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];
    auto *MutableImpl = impls.at(receiver->impl_index);

    auto callFlags = lyo1::CallFlags::Bound | lyo1::CallFlags::GlobalVisibility;

    if (isInline) {
        callFlags |= lyo1::CallFlags::Inline;
    }

    // if returnType is noReturnType then set NoReturn
    if (returnType == noReturnType) {
        callFlags |= lyo1::CallFlags::NoReturn;
    }

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = callPath;
    Call->callTemplate = ImplConcept->conceptTemplate;
    Call->callType = FunctionClass->classType;
    Call->receiver_symbol_index = receiver->receiver_symbol_index;
    Call->flags = callFlags;
    process_parameters(parameters, Call->listParameters, Call->namedParameters, Call->restParameter);
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    MutableImpl->extensions.emplace_back(lyo1::ImplExtension(ConceptAction->index, Call->call_index));

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols.push_back(Symbol);
    symboltable[Call->callPath] = symbol_index;

    return Call;
}

CoreEnum *
BuilderState::addEnum(
    const lyric_common::SymbolPath &enumPath,
    lyo1::EnumFlags instanceFlags,
    const CoreEnum *superEnum)
{
    TU_ASSERT (!symboltable.contains(enumPath));

    tu_uint32 enum_index = enums.size();
    auto *Enum = new CoreEnum();
    enums.push_back(Enum);

    const CoreType *superType = superEnum? superEnum->enumType : nullptr;
    auto *Type = addConcreteType(superType, lyo1::TypeSection::Enum, enum_index);

    Enum->enum_index = enum_index;
    Enum->enumPath = enumPath;
    Enum->flags = instanceFlags;
    Enum->enumType = Type;
    Enum->superEnum = superEnum;
    Enum->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    Enum->enumCtor = nullptr;

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Enum;
    Symbol->index = Enum->enum_index;
    symbols.push_back(Symbol);
    symboltable[Enum->enumPath] = symbol_index;

    enumcache[Enum->enumPath] = Enum;

    return Enum;
}

void
BuilderState::setEnumAllocator(const CoreEnum *receiver, std::string_view trapName)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (enumcache.contains(receiver->enumPath));

    auto *ReceiverEnum = enumcache[receiver->enumPath];
    TU_ASSERT (ReceiverEnum->allocatorTrap == lyric_object::INVALID_ADDRESS_U32);
    auto trapNumber = trapIndex->lookupTrap(trapName);
    TU_ASSERT (trapNumber != lyric_runtime::INVALID_ADDRESS_U32);
    ReceiverEnum->allocatorTrap = trapNumber;
}

CoreCall *
BuilderState::addEnumCtor(
    const CoreEnum *receiver,
    const std::vector<CoreParam> &parameters,
    const lyric_object::BytecodeBuilder &code)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (receiver->enumCtor == nullptr);
    TU_ASSERT (enumcache.contains(receiver->enumPath));
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    auto *ReceiverEnum = enumcache[receiver->enumPath];
    lyric_common::SymbolPath ctorPath(ReceiverEnum->enumPath.getPath(), "$ctor");
    TU_ASSERT (!symboltable.contains(ctorPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    auto ctorFlags = lyo1::CallFlags::Bound | lyo1::CallFlags::Ctor;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = ctorPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiver_symbol_index = getSymbolIndex(receiver->enumPath);
    Call->flags = ctorFlags;
    process_parameters(parameters, Call->listParameters, Call->namedParameters, Call->restParameter);
    Call->code = code;
    Call->returnType = ReceiverEnum->enumType;
    calls.push_back(Call);

    ReceiverEnum->enumCtor = Call;
    ReceiverEnum->methods.push_back(Call);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols.push_back(Symbol);
    symboltable[Call->callPath] = symbol_index;

    return Call;
}

CoreField *
BuilderState::addEnumMember(
    const std::string &memberName,
    const CoreEnum *receiver,
    lyo1::FieldFlags fieldFlags,
    const CoreType *memberType)
{
    TU_ASSERT (!memberName.empty());
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (enumcache.contains(receiver->enumPath));
    TU_ASSERT (memberType != nullptr);

    auto *ReceiverEnum = enumcache[receiver->enumPath];
    lyric_common::SymbolPath fieldPath(ReceiverEnum->enumPath.getPath(), memberName);
    TU_ASSERT (!symboltable.contains(fieldPath));

    auto *Field = new CoreField();
    Field->field_index = fields.size();
    Field->fieldPath = fieldPath;
    Field->fieldType = memberType;
    Field->flags = fieldFlags;
    fields.push_back(Field);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Field;
    Symbol->index = Field->field_index;
    symbols.push_back(Symbol);
    symboltable[Field->fieldPath] = symbol_index;

    ReceiverEnum->members.push_back(Field);

    return Field;
}

CoreCall *
BuilderState::addEnumMethod(
    const std::string &methodName,
    const CoreEnum *receiver,
    lyo1::CallFlags callFlags,
    const std::vector<CoreParam> &parameters,
    const lyric_object::BytecodeBuilder &code,
    const CoreType *returnType,
    bool isInline)
{
    TU_ASSERT (!methodName.empty());
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (enumcache.contains(receiver->enumPath));
    TU_ASSERT (returnType != nullptr);
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    auto *ReceiverEnum = enumcache[receiver->enumPath];
    lyric_common::SymbolPath callPath(ReceiverEnum->enumPath.getPath(), methodName);
    TU_ASSERT (!symboltable.contains(callPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    // ensure Bound is set
    callFlags = lyo1::CallFlags::Bound;

    // FIXME: isInline is ignored

    // if returnType is noReturnType then set NoReturn
    if (returnType == noReturnType) {
        callFlags |= lyo1::CallFlags::NoReturn;
    }

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = callPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiver_symbol_index = getSymbolIndex(receiver->enumPath);
    Call->flags = callFlags;
    process_parameters(parameters, Call->listParameters, Call->namedParameters, Call->restParameter);
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    ReceiverEnum->methods.push_back(Call);

    tu_uint32 symbol_index = symbols.size();
    auto *Symbol = new CoreSymbol();
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols.push_back(Symbol);
    symboltable[Call->callPath] = symbol_index;

    return Call;
}

void
BuilderState::addEnumSealedSubtype(const CoreEnum *receiver, const CoreEnum *subtypeEnum)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (enumcache.contains(receiver->enumPath));
    TU_ASSERT (subtypeEnum != nullptr);
    TU_ASSERT (enumcache.contains(subtypeEnum->enumPath));

    auto *ReceiverEnum = enumcache[receiver->enumPath];
    TU_ASSERT (bool(ReceiverEnum->flags & lyo1::EnumFlags::Sealed));
    auto *SubTypeEnum = enumcache[subtypeEnum->enumPath];
    TU_ASSERT (bool(SubTypeEnum->flags & lyo1::EnumFlags::Final));
    TU_ASSERT (SubTypeEnum->superEnum == ReceiverEnum);
    ReceiverEnum->sealedSubtypes.push_back(SubTypeEnum->enumType->type_index);
}

tu_uint32
BuilderState::getSymbolIndex(const lyric_common::SymbolPath &symbolPath) const
{
    TU_ASSERT (symboltable.contains(symbolPath));
    auto symbol_index = symboltable.at(symbolPath);
    TU_ASSERT (symbol_index < symbols.size());
    return symbol_index;
}

inline flatbuffers::Offset<flatbuffers::Vector<tu_uint32>>
build_actions_vector(flatbuffers::FlatBufferBuilder &buffer, const std::vector<CoreAction *> &actions)
{
    std::vector<tu_uint32> action_offsets;
    for (const auto *Action : actions) {
        action_offsets.push_back(Action->action_index);
    }
    return buffer.CreateVector(action_offsets);
}

inline flatbuffers::Offset<flatbuffers::Vector<tu_uint32>>
build_fields_vector(flatbuffers::FlatBufferBuilder &buffer, const std::vector<CoreField *> &fields)
{
    std::vector<tu_uint32> field_offsets;
    for (const auto *Field : fields) {
        field_offsets.push_back(Field->field_index);
    }
    return buffer.CreateVector(field_offsets);
}

inline flatbuffers::Offset<flatbuffers::Vector<tu_uint32>>
build_calls_vector(flatbuffers::FlatBufferBuilder &buffer, const std::vector<CoreCall *> &calls)
{
    std::vector<tu_uint32> call_offsets;
    for (const auto *Call : calls) {
        call_offsets.push_back(Call->call_index);
    }
    return buffer.CreateVector(call_offsets);
}

inline flatbuffers::Offset<flatbuffers::Vector<tu_uint32>>
build_impls_vector(flatbuffers::FlatBufferBuilder &buffer, const std::vector<CoreImpl *> &impls)
{
    std::vector<tu_uint32> impl_offsets;
    for (const auto *Impl : impls) {
        impl_offsets.push_back(Impl->impl_index);
    }
    return buffer.CreateVector(impl_offsets);
}

std::shared_ptr<const std::string>
BuilderState::toBytes() const
{
    flatbuffers::FlatBufferBuilder buffer;
    std::vector<flatbuffers::Offset<lyo1::TypeDescriptor>> types_vector;
    std::vector<flatbuffers::Offset<lyo1::TemplateDescriptor>> templates_vector;
    std::vector<flatbuffers::Offset<lyo1::ExistentialDescriptor>> existentials_vector;
    std::vector<flatbuffers::Offset<lyo1::FieldDescriptor>> fields_vector;
    std::vector<flatbuffers::Offset<lyo1::CallDescriptor>> calls_vector;
    std::vector<flatbuffers::Offset<lyo1::ImplDescriptor>> impls_vector;
    std::vector<flatbuffers::Offset<lyo1::ActionDescriptor>> actions_vector;
    std::vector<flatbuffers::Offset<lyo1::ConceptDescriptor>> concepts_vector;
    std::vector<flatbuffers::Offset<lyo1::ClassDescriptor>> classes_vector;
    std::vector<flatbuffers::Offset<lyo1::StructDescriptor>> structs_vector;
    std::vector<flatbuffers::Offset<lyo1::InstanceDescriptor>> instances_vector;
    std::vector<flatbuffers::Offset<lyo1::EnumDescriptor>> enums_vector;
    std::vector<flatbuffers::Offset<lyo1::StaticDescriptor>> statics_vector;
    std::vector<flatbuffers::Offset<lyo1::SymbolDescriptor>> symbols_vector;
    std::vector<flatbuffers::Offset<lyo1::SortedSymbolIdentifier>> sorted_symbol_identifiers_vector;
    std::vector<uint8_t> bytecode;

    // write the type descriptors
    for (const auto *Type : types) {
        tu_uint32 superType = Type->superType? Type->superType->type_index : lyric_object::INVALID_ADDRESS_U32;
        flatbuffers::Offset<void> typeUnion;
        switch (Type->typeAssignable) {
            case lyo1::Assignable::ConcreteAssignable: {
                auto assignable = lyo1::CreateConcreteAssignable(buffer, Type->concreteSection,
                    Type->concreteDescriptor, buffer.CreateVector(Type->typeParameters));
                typeUnion = assignable.Union();
                break;
            }
            case lyo1::Assignable::PlaceholderAssignable: {
                TU_ASSERT (Type->placeholderTemplate->template_index != lyric_object::INVALID_ADDRESS_U32);
                auto assignable = lyo1::CreatePlaceholderAssignable(buffer,
                   Type->placeholderTemplate->template_index, Type->placeholderIndex,
                   buffer.CreateVector(Type->typeParameters));
                typeUnion = assignable.Union();
                break;
            }
            case lyo1::Assignable::UnionAssignable: {
                auto assignable = lyo1::CreateUnionAssignable(buffer, buffer.CreateVector(Type->typeParameters));
                typeUnion = assignable.Union();
                break;
            }
            case lyo1::Assignable::SpecialAssignable: {
                auto assignable = lyo1::CreateSpecialAssignable(buffer, Type->specialType);
                typeUnion = assignable.Union();
                break;
            }
            default:
                TU_UNREACHABLE();
        }
        types_vector.push_back(lyo1::CreateTypeDescriptor(buffer,
            Type->typeAssignable, typeUnion, superType));
    }

    // write the template descriptors
    for (const auto *Template : templates) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Template->templatePath.toString());
        auto fb_placeholders = buffer.CreateVectorOfStructs(Template->placeholders);
        auto fb_constraints = buffer.CreateVectorOfStructs(Template->constraints);
        auto fb_names = buffer.CreateVectorOfStrings(Template->names);
        tu_uint32 superTemplate = lyric_object::INVALID_ADDRESS_U32;
        templates_vector.push_back(lyo1::CreateTemplateDescriptor(buffer, fb_fullyQualifiedName, superTemplate,
            fb_placeholders, fb_constraints, fb_names));
    }

    // write the existential descriptors
    for (const auto *Existential : existentials) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Existential->existentialPath.toString());
        auto fb_methods = build_calls_vector(buffer, Existential->methods);
        auto fb_impls = build_impls_vector(buffer, Existential->impls);
        auto fb_sealedSubtypes = buffer.CreateVector(Existential->sealedSubtypes);

        tu_uint32 superExistential = Existential->superExistential?
            Existential->superExistential->existential_index : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 existentialTemplate = Existential->existentialTemplate?
            Existential->existentialTemplate->template_index : lyric_object::INVALID_ADDRESS_U32;
        existentials_vector.push_back(lyo1::CreateExistentialDescriptor(buffer,
            fb_fullyQualifiedName, superExistential, existentialTemplate,
            Existential->existentialType->type_index, Existential->intrinsicMapping,
            Existential->flags, fb_methods, fb_impls, fb_sealedSubtypes));
    }

    // write the action descriptors
    for (const auto *Action : actions) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Action->actionPath.toString());

        std::vector<flatbuffers::Offset<lyo1::Parameter>> list_parameters;
        for (const auto &p : Action->listParameters) {
            list_parameters.push_back(lyo1::CreateParameter(buffer, &p));
        }
        auto fb_list_parameters = buffer.CreateVector(list_parameters);

        std::vector<flatbuffers::Offset<lyo1::Parameter>> named_parameters;
        for (const auto &p : Action->namedParameters) {
            named_parameters.push_back(lyo1::CreateParameter(buffer, &p));
        }
        auto fb_named_parameters = buffer.CreateVector(named_parameters);

        flatbuffers::Offset<lyo1::Parameter> fb_rest_parameter = 0;
        if (!Action->restParameter.isEmpty()) {
            auto p = Action->restParameter.getValue();
            fb_rest_parameter = lyo1::CreateParameter(buffer, &p);
        }

        tu_uint32 actionTemplate = Action->actionTemplate? Action->actionTemplate->template_index
            : lyric_object::INVALID_ADDRESS_U32;

        actions_vector.push_back(lyo1::CreateActionDescriptor(buffer,
            fb_fullyQualifiedName, actionTemplate, Action->receiver_symbol_index,
            Action->flags, fb_list_parameters, fb_named_parameters, fb_rest_parameter,
            Action->returnType->type_index));
    }

    // write the field descriptors
    for (const auto *Field : fields) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Field->fieldPath.toString());
        tu_uint32 fieldType = Field->fieldType->type_index;
        fields_vector.push_back(lyo1::CreateFieldDescriptor(buffer, fb_fullyQualifiedName,
            fieldType, Field->flags));
    }

    // write the call descriptors
    for (const auto *Call : calls) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Call->callPath.toString());

        std::vector<flatbuffers::Offset<lyo1::Parameter>> list_parameters;
        for (const auto &p : Call->listParameters) {
            list_parameters.push_back(lyo1::CreateParameter(buffer, &p));
        }
        auto fb_list_parameters = buffer.CreateVector(list_parameters);

        std::vector<flatbuffers::Offset<lyo1::Parameter>> named_parameters;
        for (const auto &p : Call->namedParameters) {
            named_parameters.push_back(lyo1::CreateParameter(buffer, &p));
        }
        auto fb_named_parameters = buffer.CreateVector(named_parameters);

        flatbuffers::Offset<lyo1::Parameter> fb_rest_parameter = 0;
        if (!Call->restParameter.isEmpty()) {
            auto p = Call->restParameter.getValue();
            fb_rest_parameter = lyo1::CreateParameter(buffer, &p);
        }

        tu_uint32 callTemplate = Call->callTemplate? Call->callTemplate->template_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 returnType = Call->returnType->type_index;

        // build the proc prologue
        auto numArguments = static_cast<tu_uint16>(list_parameters.size() + named_parameters.size());
        tempo_utils::BytesAppender prologue;
        prologue.appendU16(numArguments);                                       // append numArguments
        prologue.appendU16(0);                                                  // append numLocals
        prologue.appendU16(0);                                                  // append numLexicals

        // build the proc header
        tempo_utils::BytesAppender header;
        header.appendU32(prologue.getSize() + Call->code.bytecodeSize());       // append procSize

        // write the proc header, prologue, and bytecode
        auto bytecodeOffset = static_cast<tu_uint32>(bytecode.size());
        bytecode.insert(bytecode.cend(), header.bytesBegin(), header.bytesEnd());
        bytecode.insert(bytecode.cend(), prologue.bytesBegin(), prologue.bytesEnd());
        bytecode.insert(bytecode.cend(), Call->code.bytecodeBegin(), Call->code.bytecodeEnd());

        calls_vector.push_back(lyo1::CreateCallDescriptor(buffer, fb_fullyQualifiedName,
            callTemplate, Call->receiver_symbol_index, bytecodeOffset,
            Call->flags, fb_list_parameters, fb_named_parameters, fb_rest_parameter,
            returnType));
    }

    // write the impl descriptors
    for (const auto *Impl : impls) {
        auto fb_extensions = buffer.CreateVectorOfStructs(Impl->extensions);
        impls_vector.push_back(lyo1::CreateImplDescriptor(buffer, Impl->implType->type_index,
            Impl->implConcept->concept_index, Impl->receiver_symbol_index, Impl->flags,
            fb_extensions));
    }

    // write the class descriptors
    for (const auto *Class : classes) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Class->classPath.toString());
        auto fb_members = build_fields_vector(buffer, Class->members);
        auto fb_methods = build_calls_vector(buffer, Class->methods);
        auto fb_impls = build_impls_vector(buffer, Class->impls);
        auto fb_sealedSubtypes = buffer.CreateVector(Class->sealedSubtypes);

        tu_uint32 superClass = Class->superClass? Class->superClass->class_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 classTemplate = Class->classTemplate? Class->classTemplate->template_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 classCtor = Class->classCtor->call_index;

        classes_vector.push_back(lyo1::CreateClassDescriptor(buffer,
            fb_fullyQualifiedName, superClass, classTemplate, Class->classType->type_index, Class->flags,
            fb_members, fb_methods, fb_impls, Class->allocatorTrap, classCtor, fb_sealedSubtypes));
    }

    // write the struct descriptors
    for (const auto *Struct : structs) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Struct->structPath.toString());
        auto fb_members = build_fields_vector(buffer, Struct->members);
        auto fb_methods = build_calls_vector(buffer, Struct->methods);
        auto fb_impls = build_impls_vector(buffer, Struct->impls);
        auto fb_sealedSubtypes = buffer.CreateVector(Struct->sealedSubtypes);

        tu_uint32 superStruct = Struct->superStruct? Struct->superStruct->struct_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 structCtor = Struct->structCtor->call_index;

        structs_vector.push_back(lyo1::CreateStructDescriptor(buffer,
            fb_fullyQualifiedName, superStruct, Struct->structType->type_index, Struct->flags,
            fb_members, fb_methods, fb_impls, Struct->allocatorTrap, structCtor, fb_sealedSubtypes));
    }

    // write the concept descriptors
    for (const auto *Concept : concepts) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Concept->conceptPath.toString());
        auto fb_actions = build_actions_vector(buffer, Concept->actions);
        auto fb_impls = build_impls_vector(buffer, Concept->impls);
        auto fb_sealedSubtypes = buffer.CreateVector(Concept->sealedSubtypes);

        tu_uint32 superConcept = Concept->superConcept? Concept->superConcept->concept_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 conceptTemplate = Concept->conceptTemplate? Concept->conceptTemplate->template_index
            : lyric_object::INVALID_ADDRESS_U32;

        concepts_vector.push_back(lyo1::CreateConceptDescriptor(buffer,
            fb_fullyQualifiedName, superConcept, conceptTemplate, Concept->conceptType->type_index,
            Concept->flags, fb_actions, fb_impls, fb_sealedSubtypes));
    }

    // write the static descriptors
    for (const auto *Static : statics) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Static->staticPath.toString());
        tu_uint32 staticType = Static->staticType->type_index;
        statics_vector.push_back(lyo1::CreateStaticDescriptor(buffer, fb_fullyQualifiedName,
            staticType, Static->flags));
    }

    // write the instance descriptors
    for (const auto *Instance : instances) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Instance->instancePath.toString());
        auto fb_members = build_fields_vector(buffer, Instance->members);
        auto fb_methods = build_calls_vector(buffer, Instance->methods);
        auto fb_impls = build_impls_vector(buffer, Instance->impls);
        auto fb_sealedSubtypes = buffer.CreateVector(Instance->sealedSubtypes);

        tu_uint32 superInstance = Instance->superInstance? Instance->superInstance->instance_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 instanceCtor = Instance->instanceCtor->call_index;

        instances_vector.push_back(lyo1::CreateInstanceDescriptor(buffer,
            fb_fullyQualifiedName, superInstance, Instance->instanceType->type_index, lyo1::InstanceFlags::NONE,
            fb_members, fb_methods, fb_impls, Instance->allocatorTrap, instanceCtor, fb_sealedSubtypes));
    }

    // write the enum descriptors
    for (const auto *Enum : enums) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Enum->enumPath.toString());
        auto fb_members = build_fields_vector(buffer, Enum->members);
        auto fb_methods = build_calls_vector(buffer, Enum->methods);
        auto fb_impls = build_impls_vector(buffer, Enum->impls);
        auto fb_sealedSubtypes = buffer.CreateVector(Enum->sealedSubtypes);

        tu_uint32 superEnum = Enum->superEnum? Enum->superEnum->enum_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 enumCtor = Enum->enumCtor->call_index;

        enums_vector.push_back(lyo1::CreateEnumDescriptor(buffer,
            fb_fullyQualifiedName, superEnum, Enum->enumType->type_index, lyo1::EnumFlags::NONE,
            fb_members, fb_methods, fb_impls, Enum->allocatorTrap, enumCtor, fb_sealedSubtypes));
    }

    // write the symbol descriptors
    for (auto iterator = symbols.cbegin(); iterator != symbols.cend(); iterator++) {
        const auto *symbol = *iterator;
        symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, symbol->section, symbol->index));
    }

    // write the symbol table
    for (auto iterator = symboltable.cbegin(); iterator != symboltable.cend(); iterator++) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(iterator->first.toString());
        sorted_symbol_identifiers_vector.push_back(lyo1::CreateSortedSymbolIdentifier(
            buffer, fb_fullyQualifiedName, iterator->second));
    }

    // write the plugin descriptor if plugin exists
    flatbuffers::Offset<lyo1::PluginDescriptor> optionalPluginOffset = 0;
    if (trapIndex != nullptr) {
        auto pluginLocation = location.getPath();
        auto fb_pluginLocation = buffer.CreateSharedString(pluginLocation.toString());
        optionalPluginOffset = lyo1::CreatePluginDescriptor(buffer, fb_pluginLocation);
    }

    // serialize vectors
    auto typesOffset = buffer.CreateVector(types_vector);
    auto templatesOffset = buffer.CreateVector(templates_vector);
    auto existentialsOffset = buffer.CreateVector(existentials_vector);
    auto staticsOffset = buffer.CreateVector(statics_vector);
    auto fieldsOffset = buffer.CreateVector(fields_vector);
    auto callsOffset = buffer.CreateVector(calls_vector);
    auto implsOffset = buffer.CreateVector(impls_vector);
    auto actionsOffset = buffer.CreateVector(actions_vector);
    auto conceptsOffset = buffer.CreateVector(concepts_vector);
    auto classesOffset = buffer.CreateVector(classes_vector);
    auto structsOffset = buffer.CreateVector(structs_vector);
    auto instancesOffset = buffer.CreateVector(instances_vector);
    auto enumsOffset = buffer.CreateVector(enums_vector);
    auto symbolsOffset = buffer.CreateVector(symbols_vector);
    auto bytecodeOffset = buffer.CreateVector(bytecode);

    // serialize the symbol table
    auto sortedSymbolIdentifiersOffset = buffer.CreateVectorOfSortedTables(&sorted_symbol_identifiers_vector);
    lyo1::SortedSymbolTableBuilder sorted_symbol_table(buffer);
    sorted_symbol_table.add_identifiers(sortedSymbolIdentifiersOffset);
    auto symbolTableOffset = sorted_symbol_table.Finish();

    // serialize object and mark the buffer as finished
    lyo1::ObjectBuilder objectBuilder(buffer);
    objectBuilder.add_abi(lyo1::ObjectVersion::Version1);
    objectBuilder.add_version_major(1);
    objectBuilder.add_version_minor(0);
    objectBuilder.add_version_patch(0);
    objectBuilder.add_types(typesOffset);
    objectBuilder.add_templates(templatesOffset);
    objectBuilder.add_existentials(existentialsOffset);
    objectBuilder.add_statics(staticsOffset);
    objectBuilder.add_fields(fieldsOffset);
    objectBuilder.add_calls(callsOffset);
    objectBuilder.add_impls(implsOffset);
    objectBuilder.add_actions(actionsOffset);
    objectBuilder.add_concepts(conceptsOffset);
    objectBuilder.add_classes(classesOffset);
    objectBuilder.add_structs(structsOffset);
    objectBuilder.add_instances(instancesOffset);
    objectBuilder.add_enums(enumsOffset);
    objectBuilder.add_symbols(symbolsOffset);
    objectBuilder.add_plugin(optionalPluginOffset);
    objectBuilder.add_symbol_table_type(lyo1::SymbolTable::SortedSymbolTable);
    objectBuilder.add_symbol_table(symbolTableOffset.Union());
    objectBuilder.add_bytecode(bytecodeOffset);
    auto object = objectBuilder.Finish();
    buffer.Finish(object, lyo1::ObjectIdentifier());

    // return a byte array with a deep copy of the buffer
    return std::make_shared<const std::string>((const char *) buffer.GetBufferPointer(),
        static_cast<tu_uint32>(buffer.GetSize()));
}
