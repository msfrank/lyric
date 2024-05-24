
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/impl_import.h>
#include <lyric_importer/instance_import.h>

lyric_assembler::InstanceSymbol::InstanceSymbol(
    const lyric_common::SymbolUrl &instanceUrl,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    bool isAbstract,
    InstanceAddress address,
    TypeHandle *instanceType,
    InstanceSymbol *superInstance,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : BaseSymbol(address, new InstanceSymbolPriv()),
      m_instanceUrl(instanceUrl),
      m_state(state)
{
    TU_ASSERT (m_instanceUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = access;
    priv->derive = derive;
    priv->isAbstract = isAbstract;
    priv->instanceType = instanceType;
    priv->superInstance = superInstance;
    priv->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    priv->instanceBlock = std::make_unique<BlockHandle>(instanceUrl, parentBlock, false);

    TU_ASSERT (priv->instanceType != nullptr);
    TU_ASSERT (priv->superInstance != nullptr);
}

lyric_assembler::InstanceSymbol::InstanceSymbol(
    const lyric_common::SymbolUrl &instanceUrl,
    lyric_importer::InstanceImport *instanceImport,
    AssemblyState *state)
    : m_instanceUrl(instanceUrl),
      m_instanceImport(instanceImport),
      m_state(state)
{
    TU_ASSERT (m_instanceUrl.isValid());
    TU_ASSERT (m_instanceImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::InstanceSymbolPriv *
lyric_assembler::InstanceSymbol::load()
{
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<InstanceSymbolPriv>();
    priv->access = lyric_object::AccessType::Public;
    priv->derive = m_instanceImport->getDerive();
    priv->isAbstract = m_instanceImport->isAbstract();

    auto *instanceType = m_instanceImport->getInstanceType();
    TU_ASSIGN_OR_RAISE (priv->instanceType, typeCache->importType(instanceType));

    auto superInstanceUrl = m_instanceImport->getSuperInstance();
    if (superInstanceUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superInstance, importCache->importInstance(superInstanceUrl));
    }

    for (auto iterator = m_instanceImport->membersBegin(); iterator != m_instanceImport->membersEnd(); iterator++) {
        FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RAISE (fieldSymbol, importCache->importField(iterator->second));

        DataReference memberRef;
        memberRef.symbolUrl = iterator->second;
        memberRef.typeDef = fieldSymbol->getAssignableType();
        memberRef.referenceType = fieldSymbol->isVariable()? ReferenceType::Variable : ReferenceType::Value;
        priv->members[iterator->first] = memberRef;
    }

    for (auto iterator = m_instanceImport->methodsBegin(); iterator != m_instanceImport->methodsEnd(); iterator++) {
        CallSymbol *callSymbol;
        TU_ASSIGN_OR_RAISE (callSymbol, importCache->importCall(iterator->second));

        BoundMethod methodBinding;
        methodBinding.methodCall = iterator->second;
        methodBinding.access = callSymbol->getAccessType();
        methodBinding.final = false;    // FIXME: this should come from the call symbol
        priv->methods[iterator->first] = methodBinding;
    }

    auto *implCache = m_state->implCache();
    for (auto iterator = m_instanceImport->implsBegin(); iterator != m_instanceImport->implsEnd(); iterator++) {
        ImplHandle *implHandle;
        TU_ASSIGN_OR_RAISE (implHandle, implCache->importImpl(iterator->second));
        auto implUrl = iterator->first.getConcreteUrl();
        priv->impls[implUrl] = implHandle;
    }

    for (auto iterator = m_instanceImport->sealedTypesBegin(); iterator != m_instanceImport->sealedTypesEnd(); iterator++) {
        priv->sealedTypes.insert(*iterator);
    }

    priv->allocatorTrap = m_instanceImport->getAllocator();

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::InstanceSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Instance;
}

lyric_assembler::SymbolType
lyric_assembler::InstanceSymbol::getSymbolType() const
{
    return SymbolType::INSTANCE;
}

lyric_common::SymbolUrl
lyric_assembler::InstanceSymbol::getSymbolUrl() const
{
    return m_instanceUrl;
}

lyric_common::TypeDef
lyric_assembler::InstanceSymbol::getAssignableType() const
{
    auto *priv = getPriv();
    return priv->instanceType->getTypeDef();
}

lyric_assembler::TypeSignature
lyric_assembler::InstanceSymbol::getTypeSignature() const
{
    auto *priv = getPriv();
    return priv->instanceType->getTypeSignature();
}

void
lyric_assembler::InstanceSymbol::touch()
{
    if (getAddress().isValid())
        return;
    m_state->touchInstance(this);
}

lyric_object::AccessType
lyric_assembler::InstanceSymbol::getAccessType() const
{
    auto *priv = getPriv();
    return priv->access;
}

lyric_object::DeriveType
lyric_assembler::InstanceSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
}

bool
lyric_assembler::InstanceSymbol::isAbstract() const
{
    auto *priv = getPriv();
    return priv->isAbstract;
}

lyric_assembler::TypeHandle *
lyric_assembler::InstanceSymbol::instanceType() const
{
    auto *priv = getPriv();
    return priv->instanceType;
}

lyric_assembler::InstanceSymbol *
lyric_assembler::InstanceSymbol::superInstance() const
{
    auto *priv = getPriv();
    return priv->superInstance;
}

lyric_assembler::BlockHandle *
lyric_assembler::InstanceSymbol::instanceBlock() const
{
    auto *priv = getPriv();
    return priv->instanceBlock.get();
}

bool
lyric_assembler::InstanceSymbol::hasMember(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->members.contains(name);
}

Option<lyric_assembler::DataReference>
lyric_assembler::InstanceSymbol::getMember(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->members.contains(name))
        return Option<DataReference>(priv->members.at(name));
    return {};
}

absl::flat_hash_map<std::string,lyric_assembler::DataReference>::const_iterator
lyric_assembler::InstanceSymbol::membersBegin() const
{
    auto *priv = getPriv();
    return priv->members.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::DataReference>::const_iterator
lyric_assembler::InstanceSymbol::membersEnd() const
{
    auto *priv = getPriv();
    return priv->members.cend();
}

tu_uint32
lyric_assembler::InstanceSymbol::numMembers() const
{
    auto *priv = getPriv();
    return priv->members.size();
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::InstanceSymbol::declareMember(
    const std::string &name,
    const lyric_parser::Assignable &memberSpec,
    bool isVariable,
    lyric_object::AccessType access,
    const lyric_common::SymbolUrl &init)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare member on imported instance {}", m_instanceUrl.toString());

    auto *priv = getPriv();

    if (priv->members.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "member {} already defined for instance {}", name, m_instanceUrl.toString());

    lyric_common::TypeDef memberType;
    TU_ASSIGN_OR_RETURN (memberType, priv->instanceBlock->resolveAssignable(memberSpec));
    lyric_assembler::TypeHandle *fieldType;
    TU_ASSIGN_OR_RETURN (fieldType, m_state->typeCache()->getOrMakeType(memberType));

    auto memberPath = m_instanceUrl.getSymbolPath().getPath();
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
lyric_assembler::InstanceSymbol::resolveMember(
    const std::string &name,
    AbstractMemberReifier &reifier,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    if (!priv->members.contains(name)) {
        if (priv->superInstance == nullptr)
            return m_state->logAndContinue(AssemblerCondition::kMissingMember,
                tempo_tracing::LogSeverity::kError,
                "missing member {}", name);
        return priv->superInstance->resolveMember(name, reifier, receiverType, thisReceiver);
    }

    const auto &member = priv->members.at(name);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(member.symbolUrl));
    if (symbol->getSymbolType() != SymbolType::FIELD)
        m_state->throwAssemblerInvariant("invalid field symbol {}", member.symbolUrl.toString());
    auto *fieldSymbol = cast_symbol_to_field(symbol);
    auto access = fieldSymbol->getAccessType();

    bool thisSymbol = receiverType.getConcreteUrl() == m_instanceUrl;

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

    return reifier.reifyMember(name, fieldSymbol);
}

bool
lyric_assembler::InstanceSymbol::isMemberInitialized(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->initializedMembers.contains(name);
}

tempo_utils::Status
lyric_assembler::InstanceSymbol::setMemberInitialized(const std::string &name)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't set member initialized on imported instance {}", m_instanceUrl.toString());

    auto *priv = getPriv();

    if (isMemberInitialized(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidBinding,
            "member {} is already initialized", name);
    priv->initializedMembers.insert(name);
    return AssemblerStatus::ok();
}

bool
lyric_assembler::InstanceSymbol::isCompletelyInitialized() const
{
    auto *priv = getPriv();
    for (const auto &member : priv->members) {
        if (!priv->initializedMembers.contains(member.first))
            return false;
    }
    return true;
}

lyric_common::SymbolUrl
lyric_assembler::InstanceSymbol::getCtor() const
{
    auto location = m_instanceUrl.getAssemblyLocation();
    auto path = m_instanceUrl.getSymbolPath();
    return lyric_common::SymbolUrl(location, lyric_common::SymbolPath(path.getPath(), "$ctor"));
}

tu_uint32
lyric_assembler::InstanceSymbol::getAllocatorTrap() const
{
    auto *priv = getPriv();
    return priv->allocatorTrap;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::InstanceSymbol::declareCtor(lyric_object::AccessType access, tu_uint32 allocatorTrap)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare ctor on imported instance {}", m_instanceUrl.toString());

    auto *priv = getPriv();

    auto path = m_instanceUrl.getSymbolPath().getPath();
    path.push_back("$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(path));

    if (m_state->symbolCache()->hasSymbol(ctorUrl))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "ctor already defined for instance {}", m_instanceUrl.toString());

    //
    auto returnType = getAssignableType();
    m_state->typeCache()->touchType(returnType);

    auto fundamentalInstance = m_state->fundamentalCache()->getFundamentalUrl(FundamentalSymbol::Instance);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(fundamentalInstance));
    symbol->touch();

//    auto deriveTypeResult = m_state->declareParameterizedType(fundamentalInstance, {returnType});
//    if (deriveTypeResult.isStatus())
//        return tempo_utils::Result<lyric_common::SymbolUrl>(deriveTypeResult.getStatus());
//    auto ctorClassType = deriveTypeResult.getResult();
//    m_state->touchType(ctorClassType);

    std::vector<lyric_object::Parameter> parameters;
    auto callIndex = m_state->numCalls();
    auto address = CallAddress::near(callIndex);

    // construct call symbol
    auto *ctorSymbol = new CallSymbol(ctorUrl, parameters, {}, returnType, m_instanceUrl, access,
        address, lyric_object::CallMode::Constructor, priv->instanceType, priv->instanceBlock.get(), m_state);

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
lyric_assembler::InstanceSymbol::resolveCtor()
{
    lyric_common::SymbolPath ctorPath = lyric_common::SymbolPath(m_instanceUrl.getSymbolPath().getPath(), "$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(m_instanceUrl.getAssemblyLocation(), ctorPath);

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(ctorUrl));
    if (symbol->getSymbolType() != SymbolType::CALL)
        m_state->throwAssemblerInvariant("invalid call symbol {}", ctorUrl.toString());
    auto *call = cast_symbol_to_call(symbol);

    return CtorInvoker(call, this);
}

bool
lyric_assembler::InstanceSymbol::hasMethod(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

Option<lyric_assembler::BoundMethod>
lyric_assembler::InstanceSymbol::getMethod(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->methods.contains(name))
        return Option<BoundMethod>(priv->methods.at(name));
    return {};
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
lyric_assembler::InstanceSymbol::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
lyric_assembler::InstanceSymbol::methodsEnd() const
{
    auto *priv = getPriv();
    return priv->methods.cend();
}

tu_uint32
lyric_assembler::InstanceSymbol::numMethods() const
{
    auto *priv = getPriv();
    return priv->methods.size();
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::InstanceSymbol::declareMethod(
    const std::string &name,
    const std::vector<lyric_assembler::ParameterSpec> &parameterSpec,
    const Option<lyric_assembler::ParameterSpec> &restSpec,
    const std::vector<lyric_assembler::ParameterSpec> &ctxSpec,
    const lyric_parser::Assignable &returnSpec,
    lyric_object::AccessType access)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare method on imported instance {}", m_instanceUrl.toString());

    auto *priv = getPriv();

    if (priv->methods.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "method {} already defined for instance {}", name, m_instanceUrl.toString());

    std::vector<lyric_object::Parameter> parameters;
    Option<lyric_object::Parameter> rest;
    absl::flat_hash_set<std::string> names;
    absl::flat_hash_set<std::string> labels;

    for (const auto &p : parameterSpec) {
        auto resolveParamTypeResult = priv->instanceBlock->resolveAssignable(p.type);
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
                "parameter {} already defined for method {} on instance {}",
                p.name, name, m_instanceUrl.toString());
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for method {} on instance {}",
                p.label, name, m_instanceUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    for (const auto &p : ctxSpec) {
        auto resolveParamTypeResult = priv->instanceBlock->resolveAssignable(p.type);
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
                "parameter {} already defined for method {} on instance {}",
                p.name, name, m_instanceUrl.toString());
        names.insert(param.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for method {} on instance {}",
                p.label, name, m_instanceUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    if (!restSpec.isEmpty()) {
        const auto &p = restSpec.getValue();
        auto resolveRestTypeResult = priv->instanceBlock->resolveAssignable(p.type);
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
                "parameter {} already defined for method {} on instance {}",
                p.name, name, m_instanceUrl.toString());
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for method {} on instance {}",
                p.label, name, m_instanceUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        rest = Option<lyric_object::Parameter>(param);
    }

    auto resolveReturnTypeResult = priv->instanceBlock->resolveAssignable(returnSpec);
    if (resolveReturnTypeResult.isStatus())
        return resolveReturnTypeResult.getStatus();
    auto returnType = resolveReturnTypeResult.getResult();
    m_state->typeCache()->touchType(returnType);

    // build reference path to function
    auto methodPath = m_instanceUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));
    auto callIndex = m_state->numCalls();
    auto address = CallAddress::near(callIndex);

    // construct call symbol
    auto *callSymbol = new CallSymbol(methodUrl, parameters, rest, returnType, m_instanceUrl,
        access, address, lyric_object::CallMode::Normal, priv->instanceType, priv->instanceBlock.get(), m_state);

    auto status = m_state->appendCall(callSymbol);
    if (status.notOk()) {
        delete callSymbol;
        return status;
    }

    // add bound method
    priv->methods[name] = { methodUrl, access, true /* final */ };

    return methodUrl;
}

tempo_utils::Result<lyric_assembler::MethodInvoker>
lyric_assembler::InstanceSymbol::resolveMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    if (!priv->methods.contains(name)) {
        if (priv->superInstance == nullptr)
            return m_state->logAndContinue(AssemblerCondition::kMissingMethod,
                tempo_tracing::LogSeverity::kError,
                "missing method {}", name);
        return priv->superInstance->resolveMethod(name, receiverType);
    }

    const auto &method = priv->methods.at(name);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(method.methodCall));
    if (symbol->getSymbolType() != SymbolType::CALL)
        m_state->throwAssemblerInvariant("invalid call symbol {}", method.methodCall.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);
    auto access = callSymbol->getAccessType();

    bool thisSymbol = receiverType.getConcreteUrl() == m_instanceUrl;

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
lyric_assembler::InstanceSymbol::hasImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    return priv->impls.contains(implUrl);
}

bool
lyric_assembler::InstanceSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    return hasImpl(implType.getConcreteUrl());
}

lyric_assembler::ImplHandle *
lyric_assembler::InstanceSymbol::getImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    auto iterator = priv->impls.find(implUrl);
    if (iterator != priv->impls.cend())
        return iterator->second;
    return nullptr;
}

lyric_assembler::ImplHandle *
lyric_assembler::InstanceSymbol::getImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return nullptr;
    return getImpl(implType.getConcreteUrl());
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::InstanceSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::InstanceSymbol::implsEnd() const
{
    auto *priv = getPriv();
    return priv->impls.cend();
}

tu_uint32
lyric_assembler::InstanceSymbol::numImpls() const
{
    auto *priv = getPriv();
    return priv->impls.size();
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::InstanceSymbol::declareImpl(const lyric_parser::Assignable &implSpec)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare impl on imported instance {}", m_instanceUrl.toString());

    auto *priv = getPriv();

    auto resolveImplTypeResult = priv->instanceBlock->resolveAssignable(implSpec);
    if (resolveImplTypeResult.isStatus())
        return resolveImplTypeResult.getStatus();
    auto implType = resolveImplTypeResult.getResult();

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        m_state->throwAssemblerInvariant("invalid impl type {}", implType.toString());
    auto implUrl = implType.getConcreteUrl();

    if (priv->impls.contains(implUrl))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "impl for concept {} already defined for instance {}", implUrl.toString(), m_instanceUrl.toString());

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
    TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
        name, implTypeHandle, conceptSymbol, m_instanceUrl, priv->instanceBlock.get()));

    priv->impls[implUrl] = implHandle;

    return implType;
}

bool
lyric_assembler::InstanceSymbol::hasSealedType(const lyric_common::TypeDef &sealedType) const
{
    auto *priv = getPriv();
    return priv->sealedTypes.contains(sealedType);
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::InstanceSymbol::sealedTypesBegin() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::InstanceSymbol::sealedTypesEnd() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cend();
}

tempo_utils::Status
lyric_assembler::InstanceSymbol::putSealedType(const lyric_common::TypeDef &sealedType)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't put sealed type on imported instance {}", m_instanceUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "instance {} is not sealed", m_instanceUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "invalid derived type {} for sealed instance {}", sealedType.toString(), m_instanceUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(sealedUrl));
    if (symbol->getSymbolType() != SymbolType::INSTANCE)
        m_state->throwAssemblerInvariant("invalid instance symbol {}", sealedUrl.toString());

    if (cast_symbol_to_instance(symbol)->superInstance() != this)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "{} does not derive from sealed instance {}", sealedType.toString(), m_instanceUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return {};
}
