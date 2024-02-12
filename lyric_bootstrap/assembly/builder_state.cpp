
#include <lyric_common/symbol_url.h>
#include <tempo_utils/bytes_appender.h>

#include "builder_state.h"

BuilderState::BuilderState(const absl::flat_hash_map<std::string,std::string> &plugins)
    : plugins(plugins)
{
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

CoreExistential *
BuilderState::addExistential(
    const lyric_common::SymbolPath &existentialPath,
    lyo1::IntrinsicType intrinsicMapping,
    lyo1::ExistentialFlags existentialFlags,
    const CoreExistential *superExistential)
{
    TU_ASSERT (!symbols.contains(existentialPath));

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

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Existential->existentialPath;
    Symbol->section = lyo1::DescriptorSection::Existential;
    Symbol->index = Existential->existential_index;

    symbols[Symbol->symbolPath] = Symbol;
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
    TU_ASSERT (!symbols.contains(existentialPath));
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

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Existential->existentialPath;
    Symbol->section = lyo1::DescriptorSection::Existential;
    Symbol->index = Existential->existential_index;

    symbols[Symbol->symbolPath] = Symbol;
    existentialcache[Existential->existentialPath] = Existential;

    return Existential;
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
    TU_ASSERT (!symbols.contains(functionPath));
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    auto callFlags = lyo1::CallFlags::NONE;
    if (isInline)
        callFlags |= lyo1::CallFlags::Inline;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = functionPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiverSection = lyo1::TypeSection::Invalid;
    Call->receiverDescriptor = lyric_object::INVALID_ADDRESS_U32;
    Call->flags = callFlags;
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    absl::flat_hash_set<std::string> usedNames;
    bool restIsLastParameter = true;

    for (const auto &param : parameters) {
        TU_ASSERT (restIsLastParameter);
        TU_ASSERT (param.paramType != nullptr);

        uint8_t name_offset = lyric_object::INVALID_OFFSET_U8;
        if (!param.paramName.empty()) {
            const auto &name = param.paramName;
            TU_ASSERT (!usedNames.contains(name));
            usedNames.insert(name);
            name_offset = Call->names.size();
            Call->names.emplace_back(name);
        }

        tu_uint32 param_dfl = lyric_object::INVALID_ADDRESS_U32;
        if (param.paramDefault)
            param_dfl = param.paramDefault->call_index;

        if (bool(param.flags & lyo1::ParameterFlags::Rest)) {
            TU_ASSERT (param_dfl == lyric_object::INVALID_ADDRESS_U32);
            Call->rest = Option<lyo1::Parameter>(lyo1::Parameter(param.flags,
                param.paramType->type_index, lyric_object::INVALID_ADDRESS_U32, name_offset, lyric_object::INVALID_OFFSET_U8));
            restIsLastParameter = false;
        } else {
            TU_ASSERT (name_offset != lyric_object::INVALID_OFFSET_U8);
            Call->parameters.emplace_back(lyo1::Parameter(param.flags, param.paramType->type_index,
                param_dfl, name_offset, lyric_object::INVALID_OFFSET_U8));
        }
    }

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Call->callPath;
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols[Symbol->symbolPath] = Symbol;

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
    TU_ASSERT (!symbols.contains(functionPath));
    TU_ASSERT (functionTemplate != nullptr);
    TU_ASSERT (parameters.size() <= functionclasspaths.size());

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    auto callFlags = lyo1::CallFlags::NONE;
    if (isInline)
        callFlags |= lyo1::CallFlags::Inline;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = functionPath;
    Call->callTemplate = functionTemplate;
    Call->callType = FunctionClass->classType;
    Call->receiverSection = lyo1::TypeSection::Invalid;
    Call->receiverDescriptor = lyric_object::INVALID_ADDRESS_U32;
    Call->flags = callFlags;
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    absl::flat_hash_set<std::string> usedNames;
    bool restIsLastParameter = true;

    for (const auto &param : parameters) {
        TU_ASSERT (restIsLastParameter);
        TU_ASSERT (param.paramType != nullptr);

        uint8_t name_offset = lyric_object::INVALID_OFFSET_U8;
        if (!param.paramName.empty()) {
            const auto &name = param.paramName;
            TU_ASSERT (!usedNames.contains(name));
            usedNames.insert(name);
            name_offset = Call->names.size();
            Call->names.emplace_back(name);
        }

        tu_uint32 param_dfl = lyric_object::INVALID_ADDRESS_U32;
        if (param.paramDefault)
            param_dfl = param.paramDefault->call_index;

        if (bool(param.flags & lyo1::ParameterFlags::Rest)) {
            TU_ASSERT (param_dfl == lyric_object::INVALID_ADDRESS_U32);
            Call->rest = Option<lyo1::Parameter>(lyo1::Parameter(param.flags,
                param.paramType->type_index, lyric_object::INVALID_ADDRESS_U32, name_offset, lyric_object::INVALID_OFFSET_U8));
            restIsLastParameter = false;
        } else {
            TU_ASSERT (name_offset != lyric_object::INVALID_OFFSET_U8);
            Call->parameters.emplace_back(lyo1::Parameter(param.flags, param.paramType->type_index,
                param_dfl, name_offset, lyric_object::INVALID_OFFSET_U8));
        }
    }

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Call->callPath;
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols[Symbol->symbolPath] = Symbol;

    return Call;
}

CoreConcept *
BuilderState::addConcept(
    const lyric_common::SymbolPath &conceptPath,
    lyo1::ConceptFlags conceptFlags,
    const CoreConcept *superConcept)
{
    TU_ASSERT (!symbols.contains(conceptPath));

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

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Concept->conceptPath;
    Symbol->section = lyo1::DescriptorSection::Concept;
    Symbol->index = Concept->concept_index;

    symbols[Symbol->symbolPath] = Symbol;
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
    TU_ASSERT (!symbols.contains(conceptPath));
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

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Concept->conceptPath;
    Symbol->section = lyo1::DescriptorSection::Concept;
    Symbol->index = Concept->concept_index;

    symbols[Symbol->symbolPath] = Symbol;
    conceptcache[Concept->conceptPath] = Concept;

    return Concept;
}

CoreAction *
BuilderState::addConceptAction(
    const std::string &actionName,
    const CoreConcept *receiver,
    const std::vector<std::pair<std::string, const CoreType *>> &parameters,
    const Option<std::pair<std::string, const CoreType *>> &rest,
    const CoreType *returnType)
{
    TU_ASSERT (!actionName.empty());
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (conceptcache.contains(receiver->conceptPath));
    TU_ASSERT (returnType != nullptr);

    auto *ReceiverConcept = conceptcache[receiver->conceptPath];
    lyric_common::SymbolPath actionPath(ReceiverConcept->conceptPath.getPath(), actionName);
    TU_ASSERT (!symbols.contains(actionPath));

    auto *Action = new CoreAction();
    Action->action_index = actions.size();
    Action->actionPath = actionPath;
    Action->actionTemplate = ReceiverConcept->conceptTemplate;
    Action->receiverSection = lyo1::TypeSection::Concept;
    Action->receiverDescriptor = ReceiverConcept->concept_index;
    Action->returnType = returnType;
    Action->flags = lyo1::ActionFlags::NONE;
    actions.push_back(Action);

    absl::flat_hash_set<std::string> usedNames;
    for (const auto &param : parameters) {
        const auto &paramName = param.first;
        const auto *ParamType = param.second;
        TU_ASSERT (!usedNames.contains(paramName));
        Action->parameters.emplace_back(lyo1::Parameter(lyo1::ParameterFlags::NONE,
            ParamType->type_index, lyric_object::INVALID_ADDRESS_U32, Action->names.size(),
            lyric_object::INVALID_OFFSET_U8));
        Action->names.emplace_back(paramName);
        usedNames.insert(paramName);
    }

    if (!rest.isEmpty()) {
        const auto &p = rest.getValue();
        const auto &name = p.first;
        const auto *ParamType = p.second;
        TU_ASSERT (!usedNames.contains(name));
        Action->rest = Option<lyo1::Parameter>(lyo1::Parameter(lyo1::ParameterFlags::Rest,
            ParamType->type_index, lyric_object::INVALID_ADDRESS_U32, Action->names.size(),
            lyric_object::INVALID_OFFSET_U8));
        Action->names.emplace_back(name);
        usedNames.insert(name);
    }

    ReceiverConcept->actions.push_back(Action);

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Action->actionPath;
    Symbol->section = lyo1::DescriptorSection::Action;
    Symbol->index = Action->action_index;
    symbols[Symbol->symbolPath] = Symbol;

    return Action;
}

CoreClass *
BuilderState::addClass(
    const lyric_common::SymbolPath &classPath,
    lyo1::ClassFlags classFlags,
    const CoreClass *superClass)
{
    TU_ASSERT (!symbols.contains(classPath));

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

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Class->classPath;
    Symbol->section = lyo1::DescriptorSection::Class;
    Symbol->index = Class->class_index;

    symbols[Symbol->symbolPath] = Symbol;
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
    TU_ASSERT (!symbols.contains(classPath));
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

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Class->classPath;
    Symbol->section = lyo1::DescriptorSection::Class;
    Symbol->index = Class->class_index;

    symbols[Symbol->symbolPath] = Symbol;
    classcache[Class->classPath] = Class;

    return Class;
}

void
BuilderState::setClassAllocator(const CoreClass *receiver, lyric_bootstrap::internal::BootstrapTrap allocatorTrap)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (classcache.contains(receiver->classPath));

    auto *ReceiverClass = classcache[receiver->classPath];
    TU_ASSERT (ReceiverClass->allocatorTrap == lyric_object::INVALID_ADDRESS_U32);
    ReceiverClass->allocatorTrap = static_cast<tu_uint32>(allocatorTrap);
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
    TU_ASSERT (!symbols.contains(ctorPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];


    auto ctorFlags = lyo1::CallFlags::Bound | lyo1::CallFlags::Ctor;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = ctorPath;
    Call->callTemplate = ReceiverClass->classTemplate;
    Call->callType = FunctionClass->classType;
    Call->receiverSection = lyo1::TypeSection::Class;
    Call->receiverDescriptor = receiver->class_index;
    Call->flags = ctorFlags;
    Call->code = code;
    Call->returnType = ReceiverClass->classType;
    calls.push_back(Call);
    ReceiverClass->classCtor = Call;

    absl::flat_hash_set<std::string> usedNames;
    bool restIsLastParameter = true;

    for (const auto &param : parameters) {
        TU_ASSERT (restIsLastParameter);
        TU_ASSERT (param.paramType != nullptr);

        uint8_t name_offset = lyric_object::INVALID_OFFSET_U8;
        if (!param.paramName.empty()) {
            const auto &name = param.paramName;
            TU_ASSERT (!usedNames.contains(name));
            usedNames.insert(name);
            name_offset = Call->names.size();
            Call->names.emplace_back(name);
        }

        tu_uint32 param_dfl = lyric_object::INVALID_ADDRESS_U32;
        if (param.paramDefault)
            param_dfl = param.paramDefault->call_index;

        if (bool(param.flags & lyo1::ParameterFlags::Rest)) {
            TU_ASSERT (param_dfl == lyric_object::INVALID_ADDRESS_U32);
            Call->rest = Option<lyo1::Parameter>(lyo1::Parameter(param.flags, param.paramType->type_index,
                lyric_object::INVALID_ADDRESS_U32, name_offset, lyric_object::INVALID_OFFSET_U8));
            restIsLastParameter = false;
        } else {
            TU_ASSERT (name_offset != lyric_object::INVALID_OFFSET_U8);
            Call->parameters.emplace_back(lyo1::Parameter(param.flags, param.paramType->type_index,
                param_dfl, name_offset, lyric_object::INVALID_OFFSET_U8));
        }
    }

    ReceiverClass->methods.push_back(Call);

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Call->callPath;
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols[Symbol->symbolPath] = Symbol;

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
    TU_ASSERT (!symbols.contains(fieldPath));

    auto *Field = new CoreField();
    Field->field_index = fields.size();
    Field->fieldPath = fieldPath;
    Field->fieldType = memberType;
    Field->flags = fieldFlags;
    fields.push_back(Field);

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Field->fieldPath;
    Symbol->section = lyo1::DescriptorSection::Field;
    Symbol->index = Field->field_index;
    symbols[Symbol->symbolPath] = Symbol;

    ReceiverClass->members.push_back(Field);

    return Field;
}

CoreCall *
BuilderState::addClassMethod(
    const std::string &methodName,
    const CoreClass *receiver,
    lyo1::CallFlags callFlags,
    const std::vector<std::pair<std::string, const CoreType *>> &parameters,
    const Option<std::pair<std::string, const CoreType *>> &rest,
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
    TU_ASSERT (!symbols.contains(callPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    // ensure Bound is set
    callFlags |= lyo1::CallFlags::Bound;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = callPath;
    Call->callTemplate = ReceiverClass->classTemplate;
    Call->callType = FunctionClass->classType;
    Call->receiverSection = lyo1::TypeSection::Class;
    Call->receiverDescriptor = receiver->class_index;
    Call->flags = callFlags;
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    absl::flat_hash_set<std::string> usedNames;
    for (const auto &param : parameters) {
        const auto &paramName = param.first;
        const auto *ParamType = param.second;
        TU_ASSERT (!usedNames.contains(paramName));
        Call->parameters.emplace_back(lyo1::Parameter(lyo1::ParameterFlags::NONE,
            ParamType->type_index, lyric_object::INVALID_ADDRESS_U32, Call->names.size(),
            lyric_object::INVALID_OFFSET_U8));
        Call->names.emplace_back(paramName);
        usedNames.insert(paramName);
    }

    if (!rest.isEmpty()) {
        const auto &p = rest.getValue();
        const auto &name = p.first;
        const auto *ParamType = p.second;
        TU_ASSERT (!usedNames.contains(name));
        Call->rest = Option<lyo1::Parameter>(lyo1::Parameter(lyo1::ParameterFlags::Rest,
            ParamType->type_index, lyric_object::INVALID_ADDRESS_U32, Call->names.size(),
            lyric_object::INVALID_OFFSET_U8));
        Call->names.emplace_back(name);
        usedNames.insert(name);
    }

    ReceiverClass->methods.push_back(Call);

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Call->callPath;
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols[Symbol->symbolPath] = Symbol;

    return Call;
}

CoreStruct *
BuilderState::addStruct(
    const lyric_common::SymbolPath &structPath,
    lyo1::StructFlags structFlags,
    const CoreStruct *superStruct)
{
    TU_ASSERT (!symbols.contains(structPath));

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

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Struct->structPath;
    Symbol->section = lyo1::DescriptorSection::Struct;
    Symbol->index = Struct->struct_index;

    symbols[Symbol->symbolPath] = Symbol;
    structcache[Struct->structPath] = Struct;

    return Struct;
}

void
BuilderState::setStructAllocator(const CoreStruct *receiver, lyric_bootstrap::internal::BootstrapTrap allocatorTrap)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (structcache.contains(receiver->structPath));

    auto *ReceiverStruct = structcache[receiver->structPath];
    TU_ASSERT (ReceiverStruct->allocatorTrap == lyric_object::INVALID_ADDRESS_U32);
    ReceiverStruct->allocatorTrap = static_cast<tu_uint32>(allocatorTrap);
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
    TU_ASSERT (!symbols.contains(ctorPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    auto ctorFlags = lyo1::CallFlags::Bound | lyo1::CallFlags::Ctor;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = ctorPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiverSection = lyo1::TypeSection::Struct;
    Call->receiverDescriptor = receiver->struct_index;
    Call->flags = ctorFlags;
    Call->code = code;
    Call->returnType = ReceiverStruct->structType;
    calls.push_back(Call);
    ReceiverStruct->structCtor = Call;

    absl::flat_hash_set<std::string> usedNames;
    bool restIsLastParameter = true;

    for (const auto &param : parameters) {
        TU_ASSERT (restIsLastParameter);
        TU_ASSERT (param.paramType != nullptr);

        uint8_t name_offset = lyric_object::INVALID_OFFSET_U8;
        if (!param.paramName.empty()) {
            const auto &name = param.paramName;
            TU_ASSERT (!usedNames.contains(name));
            usedNames.insert(name);
            name_offset = Call->names.size();
            Call->names.emplace_back(name);
        }

        tu_uint32 param_dfl = lyric_object::INVALID_ADDRESS_U32;
        if (param.paramDefault)
            param_dfl = param.paramDefault->call_index;

        if (bool(param.flags & lyo1::ParameterFlags::Rest)) {
            TU_ASSERT (param_dfl == lyric_object::INVALID_ADDRESS_U32);
            Call->rest = Option<lyo1::Parameter>(lyo1::Parameter(param.flags, param.paramType->type_index,
                lyric_object::INVALID_ADDRESS_U32, name_offset, lyric_object::INVALID_OFFSET_U8));
            restIsLastParameter = false;
        } else {
            TU_ASSERT (name_offset != lyric_object::INVALID_OFFSET_U8);
            Call->parameters.emplace_back(lyo1::Parameter(param.flags, param.paramType->type_index,
                param_dfl, name_offset, lyric_object::INVALID_OFFSET_U8));
        }
    }

    ReceiverStruct->methods.push_back(Call);

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Call->callPath;
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols[Symbol->symbolPath] = Symbol;

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
    TU_ASSERT (!symbols.contains(fieldPath));

    auto *Field = new CoreField();
    Field->field_index = fields.size();
    Field->fieldPath = fieldPath;
    Field->fieldType = memberType;
    Field->flags = fieldFlags;
    fields.push_back(Field);

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Field->fieldPath;
    Symbol->section = lyo1::DescriptorSection::Field;
    Symbol->index = Field->field_index;
    symbols[Symbol->symbolPath] = Symbol;

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
    TU_ASSERT (!symbols.contains(callPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    // ensure Bound is set
    callFlags |= lyo1::CallFlags::Bound;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = callPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiverSection = lyo1::TypeSection::Struct;
    Call->receiverDescriptor = receiver->struct_index;
    Call->flags = callFlags;
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    absl::flat_hash_set<std::string> usedNames;
    bool restIsLastParameter = true;

    for (const auto &param : parameters) {
        TU_ASSERT (restIsLastParameter);
        TU_ASSERT (param.paramType != nullptr);

        uint8_t name_offset = lyric_object::INVALID_OFFSET_U8;
        if (!param.paramName.empty()) {
            const auto &name = param.paramName;
            TU_ASSERT (!usedNames.contains(name));
            usedNames.insert(name);
            name_offset = Call->names.size();
            Call->names.emplace_back(name);
        }

        tu_uint32 param_dfl = lyric_object::INVALID_ADDRESS_U32;
        if (param.paramDefault)
            param_dfl = param.paramDefault->call_index;

        if (bool(param.flags & lyo1::ParameterFlags::Rest)) {
            TU_ASSERT (param_dfl == lyric_object::INVALID_ADDRESS_U32);
            Call->rest = Option<lyo1::Parameter>(lyo1::Parameter(param.flags,
                param.paramType->type_index, lyric_object::INVALID_ADDRESS_U32, name_offset, lyric_object::INVALID_OFFSET_U8));
            restIsLastParameter = false;
        } else {
            TU_ASSERT (name_offset != lyric_object::INVALID_OFFSET_U8);
            Call->parameters.emplace_back(lyo1::Parameter(param.flags, param.paramType->type_index,
                param_dfl, name_offset, lyric_object::INVALID_OFFSET_U8));
        }
    }

    ReceiverStruct->methods.push_back(Call);

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Call->callPath;
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols[Symbol->symbolPath] = Symbol;

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
    TU_ASSERT (!symbols.contains(instancePath));

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

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Instance->instancePath;
    Symbol->section = lyo1::DescriptorSection::Instance;
    Symbol->index = Instance->instance_index;

    symbols[Symbol->symbolPath] = Symbol;
    instancecache[Instance->instancePath] = Instance;

    return Instance;
}

void
BuilderState::setInstanceAllocator(const CoreInstance *receiver, lyric_bootstrap::internal::BootstrapTrap allocatorTrap)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (instancecache.contains(receiver->instancePath));

    auto *ReceiverInstance = instancecache[receiver->instancePath];
    TU_ASSERT (ReceiverInstance->allocatorTrap == lyric_object::INVALID_ADDRESS_U32);
    ReceiverInstance->allocatorTrap = static_cast<tu_uint32>(allocatorTrap);
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
    TU_ASSERT (!symbols.contains(ctorPath));

    auto ctorFlags = lyo1::CallFlags::Bound | lyo1::CallFlags::Ctor;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = ctorPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiverSection = lyo1::TypeSection::Instance;
    Call->receiverDescriptor = receiver->instance_index;
    Call->flags = ctorFlags;
    Call->code = code;
    Call->returnType = ReceiverInstance->instanceType;
    calls.push_back(Call);
    ReceiverInstance->instanceCtor = Call;

    absl::flat_hash_set<std::string> usedNames;
    bool restIsLastParameter = true;

    for (const auto &param : parameters) {
        TU_ASSERT (restIsLastParameter);
        TU_ASSERT (param.paramType != nullptr);

        uint8_t name_offset = lyric_object::INVALID_OFFSET_U8;
        if (!param.paramName.empty()) {
            const auto &name = param.paramName;
            TU_ASSERT (!usedNames.contains(name));
            usedNames.insert(name);
            name_offset = Call->names.size();
            Call->names.emplace_back(name);
        }

        tu_uint32 param_dfl = lyric_object::INVALID_ADDRESS_U32;
        if (param.paramDefault)
            param_dfl = param.paramDefault->call_index;

        if (bool(param.flags & lyo1::ParameterFlags::Rest)) {
            TU_ASSERT (param_dfl == lyric_object::INVALID_ADDRESS_U32);
            Call->rest = Option<lyo1::Parameter>(lyo1::Parameter(param.flags, param.paramType->type_index,
                lyric_object::INVALID_ADDRESS_U32, name_offset, lyric_object::INVALID_OFFSET_U8));
            restIsLastParameter = false;
        } else {
            TU_ASSERT (name_offset != lyric_object::INVALID_OFFSET_U8);
            Call->parameters.emplace_back(lyo1::Parameter(param.flags, param.paramType->type_index,
                param_dfl, name_offset, lyric_object::INVALID_OFFSET_U8));
        }
    }

    ReceiverInstance->methods.push_back(Call);

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Call->callPath;
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols[Symbol->symbolPath] = Symbol;

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
    TU_ASSERT (!symbols.contains(fieldPath));

    auto *Field = new CoreField();
    Field->field_index = fields.size();
    Field->fieldPath = fieldPath;
    Field->fieldType = memberType;
    Field->flags = fieldFlags;
    fields.push_back(Field);

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Field->fieldPath;
    Symbol->section = lyo1::DescriptorSection::Field;
    Symbol->index = Field->field_index;
    symbols[Symbol->symbolPath] = Symbol;

    ReceiverInstance->members.push_back(Field);

    return Field;
}

CoreCall *
BuilderState::addInstanceMethod(
    const std::string &methodName,
    const CoreInstance *receiver,
    lyo1::CallFlags callFlags,
    const std::vector<std::pair<std::string, const CoreType *>> &parameters,
    const Option<std::pair<std::string, const CoreType *>> &rest,
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
    TU_ASSERT (!symbols.contains(callPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    // ensure Bound is set
    callFlags = lyo1::CallFlags::Bound;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = callPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiverSection = lyo1::TypeSection::Instance;
    Call->receiverDescriptor = receiver->instance_index;
    Call->flags = callFlags;
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    absl::flat_hash_set<std::string> usedNames;
    for (const auto &param : parameters) {
        const auto &paramName = param.first;
        const auto *ParamType = param.second;
        TU_ASSERT (!usedNames.contains(paramName));
        Call->parameters.emplace_back(lyo1::Parameter(lyo1::ParameterFlags::NONE,
            ParamType->type_index, lyric_object::INVALID_ADDRESS_U32, Call->names.size(),
            lyric_object::INVALID_OFFSET_U8));
        Call->names.emplace_back(paramName);
        usedNames.insert(paramName);
    }

    if (!rest.isEmpty()) {
        const auto &p = rest.getValue();
        const auto &name = p.first;
        const auto *ParamType = p.second;
        TU_ASSERT (!usedNames.contains(name));
        Call->rest = Option<lyo1::Parameter>(lyo1::Parameter(lyo1::ParameterFlags::Rest,
            ParamType->type_index, lyric_object::INVALID_ADDRESS_U32, Call->names.size(),
            lyric_object::INVALID_OFFSET_U8));
        Call->names.emplace_back(name);
        usedNames.insert(name);
    }

    ReceiverInstance->methods.push_back(Call);

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Call->callPath;
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols[Symbol->symbolPath] = Symbol;

    return Call;
}

CoreImpl *
BuilderState::addImpl(
    const lyric_common::SymbolPath &receiverPath,
    const CoreType *implType,
    const CoreConcept *implConcept)
{
    TU_ASSERT (symbols.contains(receiverPath));
    TU_ASSERT (implType != nullptr);
    TU_ASSERT (implConcept != nullptr);

    auto *Impl = new CoreImpl();
    Impl->impl_index = impls.size();
    impls.push_back(Impl);

    Impl->receiverPath = receiverPath;
    Impl->implType = implType;
    Impl->implConcept = implConcept;

    auto *symbol = symbols.at(receiverPath);
    switch (symbol->section) {

        case lyo1::DescriptorSection::Instance: {
            Impl->receiverSection = lyo1::TypeSection::Instance;
            Impl->receiverDescriptor = symbol->index;
            auto *receiver = instances.at(symbol->index);
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
    const std::vector<std::pair<std::string, const CoreType *>> &parameters,
    const Option<std::pair<std::string, const CoreType *>> &rest,
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
    TU_ASSERT (!symbols.contains(callPath));

    auto *ImplConcept = receiver->implConcept;
    lyric_common::SymbolPath actionPath(ImplConcept->conceptPath.getPath(), extensionName);
    TU_ASSERT (symbols.contains(actionPath));

    auto *ConceptAction = symbols[actionPath];
    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];
    auto *MutableImpl = impls.at(receiver->impl_index);

    auto callFlags = lyo1::CallFlags::Bound;
    if (isInline)
        callFlags |= lyo1::CallFlags::Inline;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = callPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiverSection = receiver->receiverSection;
    Call->receiverDescriptor = receiver->receiverDescriptor;
    Call->flags = callFlags;
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    absl::flat_hash_set<std::string> usedNames;
    for (const auto &param : parameters) {
        const auto &paramName = param.first;
        const auto *ParamType = param.second;
        TU_ASSERT (!usedNames.contains(paramName));
        Call->parameters.emplace_back(lyo1::Parameter(lyo1::ParameterFlags::NONE,
            ParamType->type_index, lyric_object::INVALID_ADDRESS_U32, Call->names.size(),
            lyric_object::INVALID_OFFSET_U8));
        Call->names.emplace_back(paramName);
        usedNames.insert(paramName);
    }

    if (!rest.isEmpty()) {
        const auto &p = rest.getValue();
        const auto &name = p.first;
        const auto *ParamType = p.second;
        TU_ASSERT (!usedNames.contains(name));
        Call->rest = Option<lyo1::Parameter>(lyo1::Parameter(lyo1::ParameterFlags::Rest,
            ParamType->type_index, lyric_object::INVALID_ADDRESS_U32, Call->names.size(),
            lyric_object::INVALID_OFFSET_U8));
        Call->names.emplace_back(name);
        usedNames.insert(name);
    }

    MutableImpl->extensions.emplace_back(lyo1::ImplExtension(ConceptAction->index, Call->call_index));

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Call->callPath;
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols[Symbol->symbolPath] = Symbol;

    return Call;
}

CoreEnum *
BuilderState::addEnum(
    const lyric_common::SymbolPath &enumPath,
    lyo1::EnumFlags instanceFlags,
    const CoreEnum *superEnum)
{
    TU_ASSERT (!symbols.contains(enumPath));

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

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Enum->enumPath;
    Symbol->section = lyo1::DescriptorSection::Enum;
    Symbol->index = Enum->enum_index;

    symbols[Symbol->symbolPath] = Symbol;
    enumcache[Enum->enumPath] = Enum;

    return Enum;
}

void
BuilderState::setEnumAllocator(const CoreEnum *receiver, lyric_bootstrap::internal::BootstrapTrap allocatorTrap)
{
    TU_ASSERT (receiver != nullptr);
    TU_ASSERT (enumcache.contains(receiver->enumPath));

    auto *ReceiverEnum = enumcache[receiver->enumPath];
    TU_ASSERT (ReceiverEnum->allocatorTrap == lyric_object::INVALID_ADDRESS_U32);
    ReceiverEnum->allocatorTrap = static_cast<tu_uint32>(allocatorTrap);
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
    TU_ASSERT (!symbols.contains(ctorPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    auto ctorFlags = lyo1::CallFlags::Bound | lyo1::CallFlags::Ctor;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = ctorPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiverSection = lyo1::TypeSection::Enum;
    Call->receiverDescriptor = receiver->enum_index;
    Call->flags = ctorFlags;
    Call->code = code;
    Call->returnType = ReceiverEnum->enumType;
    calls.push_back(Call);
    ReceiverEnum->enumCtor = Call;

    absl::flat_hash_set<std::string> usedNames;
    bool restIsLastParameter = true;

    for (const auto &param : parameters) {
        TU_ASSERT (restIsLastParameter);
        TU_ASSERT (param.paramType != nullptr);

        uint8_t name_offset = lyric_object::INVALID_OFFSET_U8;
        if (!param.paramName.empty()) {
            const auto &name = param.paramName;
            TU_ASSERT (!usedNames.contains(name));
            usedNames.insert(name);
            name_offset = Call->names.size();
            Call->names.emplace_back(name);
        }

        tu_uint32 param_dfl = lyric_object::INVALID_ADDRESS_U32;
        if (param.paramDefault)
            param_dfl = param.paramDefault->call_index;

        if (bool(param.flags & lyo1::ParameterFlags::Rest)) {
            TU_ASSERT (param_dfl == lyric_object::INVALID_ADDRESS_U32);
            Call->rest = Option<lyo1::Parameter>(lyo1::Parameter(param.flags, param.paramType->type_index,
                lyric_object::INVALID_ADDRESS_U32, name_offset, lyric_object::INVALID_OFFSET_U8));
            restIsLastParameter = false;
        } else {
            TU_ASSERT (name_offset != lyric_object::INVALID_OFFSET_U8);
            Call->parameters.emplace_back(lyo1::Parameter(param.flags, param.paramType->type_index,
                param_dfl, name_offset, lyric_object::INVALID_OFFSET_U8));
        }
    }

    ReceiverEnum->methods.push_back(Call);

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Call->callPath;
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols[Symbol->symbolPath] = Symbol;

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
    TU_ASSERT (!symbols.contains(fieldPath));

    auto *Field = new CoreField();
    Field->field_index = fields.size();
    Field->fieldPath = fieldPath;
    Field->fieldType = memberType;
    Field->flags = fieldFlags;
    fields.push_back(Field);

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Field->fieldPath;
    Symbol->section = lyo1::DescriptorSection::Field;
    Symbol->index = Field->field_index;
    symbols[Symbol->symbolPath] = Symbol;

    ReceiverEnum->members.push_back(Field);

    return Field;
}

CoreCall *
BuilderState::addEnumMethod(
    const std::string &methodName,
    const CoreEnum *receiver,
    lyo1::CallFlags callFlags,
    const std::vector<std::pair<std::string, const CoreType *>> &parameters,
    const Option<std::pair<std::string, const CoreType *>> &rest,
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
    TU_ASSERT (!symbols.contains(callPath));

    auto *FunctionClass = classcache[functionclasspaths[parameters.size()]];

    // ensure Bound is set
    callFlags = lyo1::CallFlags::Bound;

    auto *Call = new CoreCall();
    Call->call_index = calls.size();
    Call->callPath = callPath;
    Call->callTemplate = nullptr;
    Call->callType = FunctionClass->classType;
    Call->receiverSection = lyo1::TypeSection::Instance;
    Call->receiverDescriptor = receiver->enum_index;
    Call->flags = callFlags;
    Call->code = code;
    Call->returnType = returnType;
    calls.push_back(Call);

    absl::flat_hash_set<std::string> usedNames;
    for (const auto &param : parameters) {
        const auto &paramName = param.first;
        const auto *ParamType = param.second;
        TU_ASSERT (!usedNames.contains(paramName));
        Call->parameters.emplace_back(lyo1::Parameter(lyo1::ParameterFlags::NONE,
            ParamType->type_index, lyric_object::INVALID_ADDRESS_U32, Call->names.size(),
            lyric_object::INVALID_OFFSET_U8));
        Call->names.emplace_back(paramName);
        usedNames.insert(paramName);
    }

    if (!rest.isEmpty()) {
        const auto &p = rest.getValue();
        const auto &name = p.first;
        const auto *ParamType = p.second;
        TU_ASSERT (!usedNames.contains(name));
        Call->rest = Option<lyo1::Parameter>(lyo1::Parameter(lyo1::ParameterFlags::Rest,
            ParamType->type_index, lyric_object::INVALID_ADDRESS_U32, Call->names.size(),
            lyric_object::INVALID_OFFSET_U8));
        Call->names.emplace_back(name);
        usedNames.insert(name);
    }

    ReceiverEnum->methods.push_back(Call);

    auto *Symbol = new CoreSymbol();
    Symbol->symbolPath = Call->callPath;
    Symbol->section = lyo1::DescriptorSection::Call;
    Symbol->index = Call->call_index;
    symbols[Symbol->symbolPath] = Symbol;

    return Call;
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
    std::vector<flatbuffers::Offset<lyo1::PluginDescriptor>> plugins_vector;
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
        templates_vector.push_back(lyo1::CreateTemplateDescriptor(buffer, fb_fullyQualifiedName,
            fb_placeholders, fb_constraints, fb_names));
    }

    // write the existential descriptors
    for (const auto *Existential : existentials) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Existential->existentialPath.toString());
        auto fb_impls = build_impls_vector(buffer, {});

        tu_uint32 superExistential = Existential->superExistential?
            Existential->superExistential->existential_index : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 existentialTemplate = Existential->existentialTemplate?
            Existential->existentialTemplate->template_index : lyric_object::INVALID_ADDRESS_U32;
        existentials_vector.push_back(lyo1::CreateExistentialDescriptor(buffer,
            fb_fullyQualifiedName, superExistential, existentialTemplate,
            Existential->existentialType->type_index, Existential->intrinsicMapping,
            Existential->flags, fb_impls, 0 /* sealedSubtypes */));
    }

    // write the action descriptors
    for (const auto *Action : actions) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Action->actionPath.toString());
        auto fb_names = buffer.CreateVectorOfStrings(Action->names);
        auto fb_parameters = buffer.CreateVectorOfStructs(Action->parameters);

        tu_uint32 actionTemplate = Action->actionTemplate? Action->actionTemplate->template_index
            : lyric_object::INVALID_ADDRESS_U32;

        if (!Action->rest.isEmpty()) {
            const auto rest = Action->rest.getValue();
            actions_vector.push_back(lyo1::CreateActionDescriptor(buffer,
                fb_fullyQualifiedName, actionTemplate, Action->receiverSection, Action->receiverDescriptor,
                Action->flags, fb_parameters, &rest, fb_names, Action->returnType->type_index));
        } else {
            actions_vector.push_back(lyo1::CreateActionDescriptor(buffer,
                fb_fullyQualifiedName, actionTemplate, Action->receiverSection, Action->receiverDescriptor,
                Action->flags, fb_parameters, nullptr, fb_names, Action->returnType->type_index));
        }
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
        auto fb_names = buffer.CreateVectorOfStrings(Call->names);
        auto fb_parameters = buffer.CreateVectorOfStructs(Call->parameters);

        tu_uint32 callTemplate = Call->callTemplate? Call->callTemplate->template_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 returnType = Call->returnType->type_index;

        // build the proc prologue
        tempo_utils::BytesAppender prologue;
        prologue.appendU16(static_cast<tu_uint16>(Call->parameters.size()));    // append numArguments
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

        if (!Call->rest.isEmpty()) {
            const auto rest = Call->rest.getValue();
            calls_vector.push_back(lyo1::CreateCallDescriptor(buffer, fb_fullyQualifiedName,
                callTemplate, Call->receiverSection, Call->receiverDescriptor, Call->callType->type_index,
                bytecodeOffset, Call->flags, fb_parameters, &rest, fb_names, returnType));
        } else {
            calls_vector.push_back(lyo1::CreateCallDescriptor(buffer, fb_fullyQualifiedName,
                callTemplate, Call->receiverSection, Call->receiverDescriptor, Call->callType->type_index,
                bytecodeOffset, Call->flags, fb_parameters, nullptr /* rest */, fb_names, returnType));
        }
    }

    // write the impl descriptors
    for (const auto *Impl : impls) {
        auto fb_extensions = buffer.CreateVectorOfStructs(Impl->extensions);
        impls_vector.push_back(lyo1::CreateImplDescriptor(buffer, Impl->implType->type_index,
            Impl->implConcept->concept_index, Impl->receiverSection, Impl->receiverDescriptor, fb_extensions));
    }

    // write the class descriptors
    for (const auto *Class : classes) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Class->classPath.toString());
        auto fb_members = build_fields_vector(buffer, Class->members);
        auto fb_methods = build_calls_vector(buffer, Class->methods);
        auto fb_impls = build_impls_vector(buffer, {});

        tu_uint32 superClass = Class->superClass? Class->superClass->class_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 classTemplate = Class->classTemplate? Class->classTemplate->template_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 classCtor = Class->classCtor->call_index;

        classes_vector.push_back(lyo1::CreateClassDescriptor(buffer,
            fb_fullyQualifiedName, superClass, classTemplate, Class->classType->type_index, Class->flags,
            fb_members, fb_methods, fb_impls, Class->allocatorTrap, classCtor));
    }

    // write the struct descriptors
    for (const auto *Struct : structs) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Struct->structPath.toString());
        auto fb_members = build_fields_vector(buffer, Struct->members);
        auto fb_methods = build_calls_vector(buffer, Struct->methods);
        auto fb_impls = build_impls_vector(buffer, {});

        tu_uint32 superStruct = Struct->superStruct? Struct->superStruct->struct_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 structCtor = Struct->structCtor->call_index;

        structs_vector.push_back(lyo1::CreateStructDescriptor(buffer,
            fb_fullyQualifiedName, superStruct, Struct->structType->type_index, Struct->flags,
            fb_members, fb_methods, fb_impls, Struct->allocatorTrap, structCtor));
    }

    // write the concept descriptors
    for (const auto *Concept : concepts) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Concept->conceptPath.toString());
        auto fb_actions = build_actions_vector(buffer, Concept->actions);

        tu_uint32 superConcept = Concept->superConcept? Concept->superConcept->concept_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 conceptTemplate = Concept->conceptTemplate? Concept->conceptTemplate->template_index
            : lyric_object::INVALID_ADDRESS_U32;

        concepts_vector.push_back(lyo1::CreateConceptDescriptor(buffer,
            fb_fullyQualifiedName, superConcept, conceptTemplate, Concept->conceptType->type_index,
            Concept->flags, fb_actions));
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

        tu_uint32 superInstance = Instance->superInstance? Instance->superInstance->instance_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 instanceCtor = Instance->instanceCtor->call_index;

        instances_vector.push_back(lyo1::CreateInstanceDescriptor(buffer,
            fb_fullyQualifiedName, superInstance, Instance->instanceType->type_index, lyo1::InstanceFlags::NONE,
            fb_members, fb_methods, fb_impls, Instance->allocatorTrap, instanceCtor));
    }

    // write the enum descriptors
    for (const auto *Enum : enums) {
        auto fb_fullyQualifiedName = buffer.CreateSharedString(Enum->enumPath.toString());
        auto fb_members = build_fields_vector(buffer, Enum->members);
        auto fb_methods = build_calls_vector(buffer, Enum->methods);
        auto fb_impls = build_impls_vector(buffer, {});

        tu_uint32 superEnum = Enum->superEnum? Enum->superEnum->enum_index
            : lyric_object::INVALID_ADDRESS_U32;
        tu_uint32 enumCtor = Enum->enumCtor->call_index;

        enums_vector.push_back(lyo1::CreateEnumDescriptor(buffer,
            fb_fullyQualifiedName, superEnum, Enum->enumType->type_index, lyo1::EnumFlags::NONE,
            fb_members, fb_methods, fb_impls, Enum->allocatorTrap, enumCtor));
    }

    // write the plugin descriptors
    for (auto iterator = plugins.cbegin(); iterator != plugins.cend(); iterator++) {
        auto fb_pluginPlatform = buffer.CreateSharedString(iterator->first);
        auto fb_pluginPath = buffer.CreateSharedString(iterator->second);
        plugins_vector.push_back(lyo1::CreatePluginDescriptor(buffer, fb_pluginPath, fb_pluginPlatform));
    }

    // write the symbol descriptors
    for (auto iterator = symbols.cbegin(); iterator != symbols.cend(); iterator++) {
        const auto *symbol = iterator->second;
        auto fb_fullyQualifiedName = buffer.CreateSharedString(symbol->symbolPath.toString());
        symbols_vector.push_back(lyo1::CreateSymbolDescriptor(buffer, fb_fullyQualifiedName,
            symbol->section, symbol->index));
    }

    // serialize vectors
    auto fb_types = buffer.CreateVector(types_vector);
    auto fb_templates = buffer.CreateVector(templates_vector);
    auto fb_existentials = buffer.CreateVector(existentials_vector);
    auto fb_statics = buffer.CreateVector(statics_vector);
    auto fb_fields = buffer.CreateVector(fields_vector);
    auto fb_calls = buffer.CreateVector(calls_vector);
    auto fb_impls = buffer.CreateVector(impls_vector);
    auto fb_actions = buffer.CreateVector(actions_vector);
    auto fb_concepts = buffer.CreateVector(concepts_vector);
    auto fb_classes = buffer.CreateVector(classes_vector);
    auto fb_structs = buffer.CreateVector(structs_vector);
    auto fb_instances = buffer.CreateVector(instances_vector);
    auto fb_enums = buffer.CreateVector(enums_vector);
    auto fb_symbols = buffer.CreateVectorOfSortedTables(&symbols_vector);
    auto fb_plugins = buffer.CreateVector(plugins_vector);
    auto fb_bytecode = buffer.CreateVector(bytecode);

    // serialize object and mark the buffer as finished
    lyo1::ObjectBuilder objectBuilder(buffer);
    objectBuilder.add_abi(lyo1::ObjectVersion::Version1);
    objectBuilder.add_version_major(1);
    objectBuilder.add_version_minor(0);
    objectBuilder.add_version_patch(0);
    objectBuilder.add_types(fb_types);
    objectBuilder.add_templates(fb_templates);
    objectBuilder.add_existentials(fb_existentials);
    objectBuilder.add_statics(fb_statics);
    objectBuilder.add_fields(fb_fields);
    objectBuilder.add_calls(fb_calls);
    objectBuilder.add_impls(fb_impls);
    objectBuilder.add_actions(fb_actions);
    objectBuilder.add_concepts(fb_concepts);
    objectBuilder.add_classes(fb_classes);
    objectBuilder.add_structs(fb_structs);
    objectBuilder.add_instances(fb_instances);
    objectBuilder.add_enums(fb_enums);
    objectBuilder.add_symbols(fb_symbols);
    objectBuilder.add_plugins(fb_plugins);
    objectBuilder.add_bytecode(fb_bytecode);
    auto object = objectBuilder.Finish();
    buffer.Finish(object, lyo1::ObjectIdentifier());

    // return a byte array with a deep copy of the buffer
    return std::make_shared<const std::string>((const char *) buffer.GetBufferPointer(),
        static_cast<tu_uint32>(buffer.GetSize()));
}
