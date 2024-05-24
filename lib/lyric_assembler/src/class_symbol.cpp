#include <absl/container/flat_hash_set.h>

#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/type_import.h>
#include "lyric_assembler/concept_symbol.h"

lyric_assembler::ClassSymbol::ClassSymbol(
    const lyric_common::SymbolUrl &classUrl,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    bool isAbstract,
    ClassAddress address,
    TypeHandle *classType,
    ClassSymbol *superClass,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : BaseSymbol(address, new ClassSymbolPriv()),
      m_classUrl(classUrl),
      m_state(state)
{
    TU_ASSERT (m_classUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = access;
    priv->derive = derive;
    priv->isAbstract = isAbstract;
    priv->classType = classType;
    priv->classTemplate = nullptr;
    priv->superClass = superClass;
    priv->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    priv->classBlock = std::make_unique<BlockHandle>(classUrl, parentBlock, false);

    TU_ASSERT (priv->classType != nullptr);
    TU_ASSERT (priv->superClass != nullptr);
}

lyric_assembler::ClassSymbol::ClassSymbol(
    const lyric_common::SymbolUrl &classUrl,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    bool isAbstract,
    ClassAddress address,
    TypeHandle *classType,
    TemplateHandle *classTemplate,
    ClassSymbol *superClass,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : ClassSymbol(
        classUrl,
        access,
        derive,
        isAbstract,
        address,
        classType,
        superClass,
        parentBlock,
        state)
{
    auto *priv = getPriv();
    priv->classTemplate = classTemplate;
    TU_ASSERT(priv->classTemplate != nullptr);
    for (auto it = classTemplate->templateParametersBegin(); it != classTemplate->templateParametersEnd(); it++) {
        const auto &tp = *it;
        TU_RAISE_IF_STATUS (priv->classBlock->declareAlias(tp.name, classTemplate->getTemplateUrl(), tp.index));
    }
}

lyric_assembler::ClassSymbol::ClassSymbol(
    const lyric_common::SymbolUrl &classUrl,
    lyric_importer::ClassImport *classImport,
    AssemblyState *state)
    : m_classUrl(classUrl),
      m_classImport(classImport),
      m_state(state)
{
    TU_ASSERT (m_classUrl.isValid());
    TU_ASSERT (m_classImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::ClassSymbolPriv *
lyric_assembler::ClassSymbol::load()
{
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<ClassSymbolPriv>();
    priv->access = lyric_object::AccessType::Public;
    priv->derive = m_classImport->getDerive();
    priv->isAbstract = m_classImport->isAbstract();

    auto *classType = m_classImport->getClassType();
    TU_ASSIGN_OR_RAISE (priv->classType, typeCache->importType(classType));

    auto *classTemplate = m_classImport->getClassTemplate();
    if (classTemplate != nullptr) {
        TU_ASSIGN_OR_RAISE (priv->classTemplate, typeCache->importTemplate(classTemplate));
    }

    auto superClassUrl = m_classImport->getSuperClass();
    if (superClassUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superClass, importCache->importClass(superClassUrl));
    }

    for (auto iterator = m_classImport->membersBegin(); iterator != m_classImport->membersEnd(); iterator++) {
        FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RAISE (fieldSymbol, importCache->importField(iterator->second));

        DataReference memberRef;
        memberRef.symbolUrl = iterator->second;
        memberRef.typeDef = fieldSymbol->getAssignableType();
        memberRef.referenceType = fieldSymbol->isVariable()? ReferenceType::Variable : ReferenceType::Value;
        priv->members[iterator->first] = memberRef;
    }

    for (auto iterator = m_classImport->methodsBegin(); iterator != m_classImport->methodsEnd(); iterator++) {
        CallSymbol *callSymbol;
        TU_ASSIGN_OR_RAISE (callSymbol, importCache->importCall(iterator->second));

        BoundMethod methodBinding;
        methodBinding.methodCall = iterator->second;
        methodBinding.access = callSymbol->getAccessType();
        methodBinding.final = false;    // FIXME: this should come from the call symbol
        priv->methods[iterator->first] = methodBinding;
    }

    auto *implCache = m_state->implCache();
    for (auto iterator = m_classImport->implsBegin(); iterator != m_classImport->implsEnd(); iterator++) {
        ImplHandle *implHandle;
        TU_ASSIGN_OR_RAISE (implHandle, implCache->importImpl(iterator->second));
        auto implUrl = iterator->first.getConcreteUrl();
        priv->impls[implUrl] = implHandle;
    }

    for (auto iterator = m_classImport->sealedTypesBegin(); iterator != m_classImport->sealedTypesEnd(); iterator++) {
        priv->sealedTypes.insert(*iterator);
    }

    priv->allocatorTrap = m_classImport->getAllocator();

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::ClassSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Class;
}

lyric_assembler::SymbolType
lyric_assembler::ClassSymbol::getSymbolType() const
{
    return SymbolType::CLASS;
}

lyric_common::SymbolUrl
lyric_assembler::ClassSymbol::getSymbolUrl() const
{
    return m_classUrl;
}

lyric_common::TypeDef
lyric_assembler::ClassSymbol::getAssignableType() const
{
    auto *priv = getPriv();
    return priv->classType->getTypeDef();
}

lyric_assembler::TypeSignature
lyric_assembler::ClassSymbol::getTypeSignature() const
{
    auto *priv = getPriv();
    return priv->classType->getTypeSignature();
}

void
lyric_assembler::ClassSymbol::touch()
{
    if (getAddress().isValid())
        return;
    m_state->touchClass(this);
}

lyric_object::AccessType
lyric_assembler::ClassSymbol::getAccessType() const
{
    auto *priv = getPriv();
    return priv->access;
}

lyric_object::DeriveType
lyric_assembler::ClassSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
}

bool
lyric_assembler::ClassSymbol::isAbstract() const
{
    auto *priv = getPriv();
    return priv->isAbstract;
}

lyric_assembler::ClassSymbol *
lyric_assembler::ClassSymbol::superClass() const
{
    auto *priv = getPriv();
    return priv->superClass;
}

lyric_assembler::TypeHandle *
lyric_assembler::ClassSymbol::classType() const
{
    auto *priv = getPriv();
    return priv->classType;
}

lyric_assembler::TemplateHandle *
lyric_assembler::ClassSymbol::classTemplate() const
{
    auto *priv = getPriv();
    return priv->classTemplate;
}

lyric_assembler::BlockHandle *
lyric_assembler::ClassSymbol::classBlock() const
{
    auto *priv = getPriv();
    return priv->classBlock.get();
}

bool
lyric_assembler::ClassSymbol::hasMember(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->members.contains(name);
}

Option<lyric_assembler::DataReference>
lyric_assembler::ClassSymbol::getMember(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->members.contains(name))
        return Option<DataReference>(priv->members.at(name));
    return Option<DataReference>();
}

absl::flat_hash_map<std::string,lyric_assembler::DataReference>::const_iterator
lyric_assembler::ClassSymbol::membersBegin() const
{
    auto *priv = getPriv();
    return priv->members.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::DataReference>::const_iterator
lyric_assembler::ClassSymbol::membersEnd() const
{
    auto *priv = getPriv();
    return priv->members.cend();
}

tu_uint32
lyric_assembler::ClassSymbol::numMembers() const
{
    auto *priv = getPriv();
    return static_cast<tu_uint32>(priv->members.size());
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::ClassSymbol::declareMember(
    const std::string &name,
    const lyric_parser::Assignable &memberSpec,
    bool isVariable,
    lyric_object::AccessType access,
    const lyric_common::SymbolUrl &init)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare member on imported class {}", m_classUrl.toString());

    auto *priv = getPriv();

    if (priv->members.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "member {} already defined for class {}", name, m_classUrl.toString());

    AbstractResolver *resolver = priv->classTemplate?
        (AbstractResolver *) priv->classTemplate : priv->classBlock.get();

    lyric_common::TypeDef memberType;
    TU_ASSIGN_OR_RETURN (memberType, resolver->resolveAssignable(memberSpec));
    lyric_assembler::TypeHandle *fieldType;
    TU_ASSIGN_OR_RETURN (fieldType, m_state->typeCache()->getOrMakeType(memberType));

    auto memberPath = m_classUrl.getSymbolPath().getPath();
    memberPath.push_back(name);
    auto memberUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(memberPath));
    auto fieldIndex = m_state->numFields();
    auto address = FieldAddress::near(fieldIndex);

    // construct the field symbol
    FieldSymbol *fieldSymbol;
    if (init.isValid()) {
        fieldSymbol = new FieldSymbol(memberUrl, access, isVariable, init, address, fieldType, m_state);
    } else {
        fieldSymbol = new FieldSymbol(memberUrl, access, isVariable, address, fieldType, m_state);
    }

    auto status = m_state->appendField(fieldSymbol);
    if (status.notOk()) {
        delete fieldSymbol;
        return status;
    }

    m_state->typeCache()->touchType(memberType);

    DataReference ref;
    ref.symbolUrl = memberUrl;
    ref.typeDef = memberType;
    ref.referenceType = isVariable? ReferenceType::Variable : ReferenceType::Value;
    priv->members[name] = ref;

    return ref;
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::ClassSymbol::resolveMember(
    const std::string &name,
    AbstractMemberReifier &reifier,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    if (!priv->members.contains(name)) {
        if (priv->superClass == nullptr)
            return m_state->logAndContinue(AssemblerCondition::kMissingMember,
                tempo_tracing::LogSeverity::kError,
                "missing member {}", name);
        return priv->superClass->resolveMember(name, reifier, receiverType, thisReceiver);
    }

    const auto &member = priv->members.at(name);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(member.symbolUrl));
    if (symbol->getSymbolType() != SymbolType::FIELD)
        m_state->throwAssemblerInvariant("invalid field symbol {}", member.symbolUrl.toString());
    auto *fieldSymbol = cast_symbol_to_field(symbol);
    auto access = fieldSymbol->getAccessType();

    bool thisSymbol = receiverType.getConcreteUrl() == m_classUrl;

    if (thisReceiver) {
        if (access == lyric_object::AccessType::Private && !thisSymbol)
            return m_state->logAndContinue(AssemblerCondition::kInvalidAccess,
                tempo_tracing::LogSeverity::kError,
                "access to private member {} is not allowed", name);
    } else {
        if (access != lyric_object::AccessType::Public)
            return m_state->logAndContinue(AssemblerCondition::kInvalidAccess,
                tempo_tracing::LogSeverity::kError,
                "access to protected member {} is not allowed", name);
    }

//    if (m_classTemplate == nullptr) {
//        MemberReifier reifier(members, m_state);
//        return reifier.reifyMember(name);
//    }
//
//    auto typeParameters = receiverType.getTypeParameters();
//    MemberReifier reifier(members, m_classTemplate->getTemplateUrl(),
//        m_classTemplate->getTemplateParameters(),
//        std::vector<Assignable>(typeParameters.cbegin(), typeParameters.cend()),
//        m_state);

    return reifier.reifyMember(name, fieldSymbol);
}

bool
lyric_assembler::ClassSymbol::isMemberInitialized(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->initializedMembers.contains(name);
}

tempo_utils::Status
lyric_assembler::ClassSymbol::setMemberInitialized(const std::string &name)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't set member initialized on imported class {}", m_classUrl.toString());

    auto *priv = getPriv();
    if (isMemberInitialized(name))
        m_state->throwAssemblerInvariant("member {} was already initialized", name);
    priv->initializedMembers.insert(name);
    return AssemblerStatus::ok();
}

bool
lyric_assembler::ClassSymbol::isCompletelyInitialized() const
{
    auto *priv = getPriv();
    for (const auto &member : priv->members) {
        if (!priv->initializedMembers.contains(member.first))
            return false;
    }
    return true;
}

lyric_common::SymbolUrl
lyric_assembler::ClassSymbol::getCtor() const
{
    auto location = m_classUrl.getAssemblyLocation();
    auto path = m_classUrl.getSymbolPath();
    return lyric_common::SymbolUrl(location, lyric_common::SymbolPath(path.getPath(), "$ctor"));
}

tu_uint32
lyric_assembler::ClassSymbol::getAllocatorTrap() const
{
    auto *priv = getPriv();
    return priv->allocatorTrap;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::ClassSymbol::declareCtor(
    const std::vector<lyric_assembler::ParameterSpec> &parameterSpec,
    const Option<lyric_assembler::ParameterSpec> &restSpec,
    const std::vector<lyric_assembler::ParameterSpec> &ctxSpec,
    lyric_object::AccessType access,
    tu_uint32 allocatorTrap)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare ctor on imported class {}", m_classUrl.toString());

    auto *priv = getPriv();

    auto path = m_classUrl.getSymbolPath().getPath();
    path.push_back("$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(path));

    if (m_state->symbolCache()->hasSymbol(ctorUrl))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "ctor already defined for class {}", m_classUrl.toString());

    // FIXME: should we touch template parameters?
//    if (m_classTemplate) {
//        m_state->touchTemplateParameters(m_classTemplate->getTemplateParameters());
//    }

    //
    auto returnType = getAssignableType();
    m_state->typeCache()->touchType(returnType);

    auto fundamentalClass = m_state->fundamentalCache()->getFundamentalUrl(FundamentalSymbol::Class);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(fundamentalClass));
    symbol->touch();

//    auto deriveTypeResult = m_state->declareParameterizedType(fundamentalClass, {returnType});
//    if (deriveTypeResult.isStatus())
//        return tempo_utils::Result<lyric_common::SymbolUrl>(deriveTypeResult.getStatus());
//    auto ctorClassType = deriveTypeResult.getResult();
//    m_state->touchType(ctorClassType);

    std::vector<lyric_object::Parameter> parameters;
    Option<lyric_object::Parameter> rest;
    absl::flat_hash_set<std::string> names;
    absl::flat_hash_set<std::string> labels;

    AbstractResolver *resolver = priv->classTemplate?
        (AbstractResolver *) priv->classTemplate : priv->classBlock.get();

    for (const auto &p : parameterSpec) {
        auto resolveParamTypeResult = resolver->resolveAssignable(p.type);
        if (resolveParamTypeResult.isStatus())
            return resolveParamTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.name = p.name;
        param.label = !p.label.empty()? p.label : p.name;
        param.placement = !p.label.empty()? lyric_object::PlacementType::Named : lyric_object::PlacementType::List;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE? true : false;
        param.typeDef = resolveParamTypeResult.getResult();

        if (!p.init.isEmpty()) {
            if (param.placement != lyric_object::PlacementType::Named) {
                return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid initializer for positional parameter {}; only named parameters can be default-initialized",
                    p.name);
            } else {
                param.placement = lyric_object::PlacementType::Opt;
            }
        }

        if (names.contains(p.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for ctor on class {}",
                p.name, m_classUrl.toString());
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for ctor on class {}",
                p.name, m_classUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    for (const auto &p : ctxSpec) {
        auto resolveParamTypeResult = resolver->resolveAssignable(p.type);
        if (resolveParamTypeResult.isStatus())
            return resolveParamTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.placement = lyric_object::PlacementType::Ctx;
        param.isVariable = false;
        param.typeDef = resolveParamTypeResult.getResult();

        // if ctx parameter name is not specified, then generate a unique name
        param.name = p.name.empty()? absl::StrCat("$ctx", parameters.size()) : p.name;
        param.label = param.name;

        if (names.contains(param.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for ctor on class {}",
                p.name, m_classUrl.toString());
        names.insert(param.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for ctor on class {}",
                p.name, m_classUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    if (!restSpec.isEmpty()) {
        const auto &p = restSpec.getValue();
        auto resolveRestTypeResult = resolver->resolveAssignable(p.type);
        if (resolveRestTypeResult.isStatus())
            return resolveRestTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.name = p.name;
        param.label = param.name;
        param.placement = lyric_object::PlacementType::Rest;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE? true : false;
        param.typeDef = resolveRestTypeResult.getResult();

        if (names.contains(p.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for ctor on class {}",
                p.name, m_classUrl.toString());
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for ctor on class {}",
                p.name, m_classUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        rest = Option<lyric_object::Parameter>(param);
    }

    auto callIndex = m_state->numCalls();
    auto address = CallAddress::near(callIndex);

    // construct call symbol
    CallSymbol *ctorSymbol;
    if (priv->classTemplate != nullptr) {
        ctorSymbol = new CallSymbol(ctorUrl, parameters, rest, returnType, m_classUrl, access, address,
            lyric_object::CallMode::Constructor, priv->classType, priv->classTemplate, priv->classBlock.get(), m_state);
    } else {
        ctorSymbol = new CallSymbol(ctorUrl, parameters, rest, returnType, m_classUrl, access, address,
            lyric_object::CallMode::Constructor, priv->classType, priv->classBlock.get(), m_state);
    }

    auto status = m_state->appendCall(ctorSymbol);
    if (status.notOk()) {
        delete ctorSymbol;
        return status;
    }

    // add bound method
    BoundMethod method;
    method.methodCall = ctorUrl;
    method.access = access;
    method.final = false;
    priv->methods["$ctor"] = method;

    // set allocator trap
    priv->allocatorTrap = allocatorTrap;

    return ctorUrl;
}

tempo_utils::Result<lyric_assembler::CtorInvoker>
lyric_assembler::ClassSymbol::resolveCtor()
{
    lyric_common::SymbolPath ctorPath = lyric_common::SymbolPath(m_classUrl.getSymbolPath().getPath(), "$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(m_classUrl.getAssemblyLocation(), ctorPath);

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(ctorUrl));
    if (symbol->getSymbolType() != SymbolType::CALL)
        m_state->throwAssemblerInvariant("invalid call symbol {}", ctorUrl.toString());
    auto *call = cast_symbol_to_call(symbol);

    return CtorInvoker(call, this);
}

bool
lyric_assembler::ClassSymbol::hasMethod(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

Option<lyric_assembler::BoundMethod>
lyric_assembler::ClassSymbol::getMethod(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->methods.contains(name))
        return Option<BoundMethod>(priv->methods.at(name));
    return Option<BoundMethod>();
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
lyric_assembler::ClassSymbol::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
lyric_assembler::ClassSymbol::methodsEnd() const
{
    auto *priv = getPriv();
    return priv->methods.cend();
}

tu_uint32
lyric_assembler::ClassSymbol::numMethods() const
{
    auto *priv = getPriv();
    return static_cast<tu_uint32>(priv->methods.size());
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::ClassSymbol::declareMethod(
    const std::string &name,
    const std::vector<lyric_assembler::ParameterSpec> &parameterSpec,
    const Option<lyric_assembler::ParameterSpec> &restSpec,
    const std::vector<lyric_assembler::ParameterSpec> &ctxSpec,
    const lyric_parser::Assignable &returnSpec,
    lyric_object::AccessType access)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare method on imported class {}", m_classUrl.toString());

    auto *priv = getPriv();

    if (priv->methods.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "method {} already defined for class {}", name, m_classUrl.toString());

    AbstractResolver *resolver = priv->classTemplate?
        (AbstractResolver *) priv->classTemplate : priv->classBlock.get();

    std::vector<lyric_object::Parameter> parameters;
    Option<lyric_object::Parameter> rest;
    absl::flat_hash_set<std::string> names;
    absl::flat_hash_set<std::string> labels;

    for (const auto &p : parameterSpec) {
        auto resolveParamTypeResult = resolver->resolveAssignable(p.type);
        if (resolveParamTypeResult.isStatus())
            return resolveParamTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.name = p.name;
        param.label = !p.label.empty()? p.label : p.name;
        param.placement = !p.label.empty()? lyric_object::PlacementType::Named : lyric_object::PlacementType::List;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE? true : false;
        param.typeDef = resolveParamTypeResult.getResult();

        if (!p.init.isEmpty()) {
            if (param.placement != lyric_object::PlacementType::Named) {
                return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid initializer for positional parameter {}; only named parameters can be default-initialized",
                    p.name);
            } else {
                param.placement = lyric_object::PlacementType::Opt;
            }
        }

        if (names.contains(p.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for method {} on class {}",
                p.name, name, m_classUrl.toString());
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for method {} on class {}",
                p.label, name, m_classUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    for (const auto &p : ctxSpec) {
        auto resolveParamTypeResult = resolver->resolveAssignable(p.type);
        if (resolveParamTypeResult.isStatus())
            return resolveParamTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.placement = lyric_object::PlacementType::Ctx;
        param.isVariable = false;
        param.typeDef = resolveParamTypeResult.getResult();

        // if ctx parameter name is not specified, then generate a unique name
        param.name = p.name.empty()? absl::StrCat("$ctx", parameters.size()) : p.name;
        param.label = param.name;

        if (names.contains(param.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for method {} on class {}",
                p.name, name, m_classUrl.toString());
        names.insert(param.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for method {} on class {}",
                p.label, name, m_classUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    if (!restSpec.isEmpty()) {
        const auto &p = restSpec.getValue();
        auto resolveRestTypeResult = resolver->resolveAssignable(p.type);
        if (resolveRestTypeResult.isStatus())
            return resolveRestTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.name = p.name;
        param.label = param.name;
        param.placement = lyric_object::PlacementType::Rest;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE? true : false;
        param.typeDef = resolveRestTypeResult.getResult();

        if (names.contains(p.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for method {} on class {}",
                p.name, name, m_classUrl.toString());
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for method {} on class {}",
                p.label, name, m_classUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        rest = Option<lyric_object::Parameter>(param);
    }

    auto resolveReturnTypeResult = resolver->resolveAssignable(returnSpec);
    if (resolveReturnTypeResult.isStatus())
        return resolveReturnTypeResult.getStatus();
    auto returnType = resolveReturnTypeResult.getResult();
    m_state->typeCache()->touchType(returnType);

    // build reference path to function
    auto methodPath = m_classUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));
    auto callIndex = m_state->numCalls();
    auto address = CallAddress::near(callIndex);

    // construct call symbol
    CallSymbol *callSymbol;
    if (priv->classTemplate != nullptr) {
        callSymbol = new CallSymbol(methodUrl, parameters, rest, returnType, m_classUrl,
            access, address, lyric_object::CallMode::Normal, priv->classType, priv->classTemplate,
            priv->classBlock.get(), m_state);
    } else {
        callSymbol = new CallSymbol(methodUrl, parameters, rest, returnType, m_classUrl,
            access, address, lyric_object::CallMode::Normal, priv->classType, priv->classBlock.get(),
            m_state);
    }

    auto status = m_state->appendCall(callSymbol);
    if (status.notOk()) {
        delete callSymbol;
        return status;
    }

    // add bound method
    BoundMethod method;
    method.methodCall = methodUrl;
    method.access = access;
    method.final = false;
    priv->methods[name] = method;

    return methodUrl;
}

tempo_utils::Result<lyric_assembler::MethodInvoker>
lyric_assembler::ClassSymbol::resolveMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    if (!priv->methods.contains(name)) {
        if (priv->superClass == nullptr)
            return m_state->logAndContinue(AssemblerCondition::kMissingMethod,
                tempo_tracing::LogSeverity::kError,
                "missing method {}", name);
        return priv->superClass->resolveMethod(name, receiverType, thisReceiver);
    }

    const auto &method = priv->methods.at(name);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(method.methodCall));
    if (symbol->getSymbolType() != SymbolType::CALL)
        m_state->throwAssemblerInvariant("invalid call symbol {}", method.methodCall.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);
    auto access = callSymbol->getAccessType();

    bool thisSymbol = receiverType.getConcreteUrl() == m_classUrl;

    if (thisReceiver) {
        if (access == lyric_object::AccessType::Private && !thisSymbol)
            return m_state->logAndContinue(AssemblerCondition::kInvalidAccess,
                tempo_tracing::LogSeverity::kError,
                "invocation of private method {} is not allowed", name);
    } else {
        if (access != lyric_object::AccessType::Public)
            return m_state->logAndContinue(AssemblerCondition::kInvalidAccess,
                tempo_tracing::LogSeverity::kError,
                "invocation of protected method {} is not allowed", name);
    }

    if (callSymbol->isInline())
        return MethodInvoker(callSymbol, callSymbol->callProc());
    if (!callSymbol->isBound())
        m_state->throwAssemblerInvariant("invalid call symbol {}", callSymbol->getSymbolUrl().toString());

    return MethodInvoker(callSymbol);
}

bool
lyric_assembler::ClassSymbol::hasImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    return priv->impls.contains(implUrl);
}

bool
lyric_assembler::ClassSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    return hasImpl(implType.getConcreteUrl());
}

lyric_assembler::ImplHandle *
lyric_assembler::ClassSymbol::getImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    auto iterator = priv->impls.find(implUrl);
    if (iterator != priv->impls.cend())
        return iterator->second;
    return nullptr;
}

lyric_assembler::ImplHandle *
lyric_assembler::ClassSymbol::getImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return nullptr;
    return getImpl(implType.getConcreteUrl());
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ClassSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ClassSymbol::implsEnd() const
{
    auto *priv = getPriv();
    return priv->impls.cend();
}

tu_uint32
lyric_assembler::ClassSymbol::numImpls() const
{
    auto *priv = getPriv();
    return priv->impls.size();
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::ClassSymbol::declareImpl(const lyric_parser::Assignable &implSpec)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare impl on imported class {}", m_classUrl.toString());

    auto *priv = getPriv();

    AbstractResolver *resolver = priv->classTemplate?
        (AbstractResolver *) priv->classTemplate : priv->classBlock.get();

    auto resolveImplTypeResult = resolver->resolveAssignable(implSpec);
    if (resolveImplTypeResult.isStatus())
        return resolveImplTypeResult.getStatus();
    auto implType = resolveImplTypeResult.getResult();

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        m_state->throwAssemblerInvariant("invalid impl type {}", implType.toString());
    auto implUrl = implType.getConcreteUrl();

    if (priv->impls.contains(implUrl))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "impl {} already defined for class {}", implType.toString(), m_classUrl.toString());

    // touch the impl type
    lyric_assembler::TypeHandle *implTypeHandle;
    TU_ASSIGN_OR_RETURN (implTypeHandle, m_state->typeCache()->getOrMakeType(implType));
    implTypeHandle->touch();

    // confirm that the impl concept exists
    auto implConcept = implType.getConcreteUrl();
    if (!m_state->symbolCache()->hasSymbol(implConcept))
        m_state->throwAssemblerInvariant("missing concept symbol {}", implConcept.toString());

    // resolve the concept symbol
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(implConcept));
    if (symbol->getSymbolType() != SymbolType::CONCEPT)
        m_state->throwAssemblerInvariant("invalid concept symbol {}", implConcept.toString());
    auto *conceptSymbol = cast_symbol_to_concept(symbol);

    conceptSymbol->touch();

    auto *implCache = m_state->implCache();

    auto name = absl::StrCat("$impl", priv->impls.size());

    ImplHandle *implHandle;
    if (priv->classTemplate != nullptr) {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
            name, implTypeHandle, conceptSymbol, m_classUrl, priv->classTemplate, priv->classBlock.get()));
    } else {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
            name, implTypeHandle, conceptSymbol, m_classUrl, priv->classBlock.get()));
    }

    priv->impls[implUrl] = implHandle;

    return implType;
}

bool
lyric_assembler::ClassSymbol::hasSealedType(const lyric_common::TypeDef &sealedType) const
{
    auto *priv = getPriv();
    return priv->sealedTypes.contains(sealedType);
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ClassSymbol::sealedTypesBegin() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ClassSymbol::sealedTypesEnd() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cend();
}

tempo_utils::Status
lyric_assembler::ClassSymbol::putSealedType(const lyric_common::TypeDef &sealedType)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't put sealed type on imported class {}", m_classUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "class {} is not sealed", m_classUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "invalid derived type {} for sealed class {}", sealedType.toString(), m_classUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(sealedUrl));
    if (symbol->getSymbolType() != SymbolType::CLASS)
        m_state->throwAssemblerInvariant("invalid class symbol {}", sealedUrl.toString());

    if (cast_symbol_to_class(symbol)->superClass() != this)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "{} does not derive from sealed class {}", sealedType.toString(), m_classUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return {};
}
