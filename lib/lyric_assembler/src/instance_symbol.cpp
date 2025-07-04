
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/ctor_constructable.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/method_callable.h>
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
    TypeHandle *instanceType,
    InstanceSymbol *superInstance,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(new InstanceSymbolPriv()),
      m_instanceUrl(instanceUrl),
      m_state(state)
{
    TU_ASSERT (m_instanceUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = access;
    priv->derive = derive;
    priv->isAbstract = isAbstract;
    priv->isDeclOnly = isDeclOnly;
    priv->instanceType = instanceType;
    priv->superInstance = superInstance;
    priv->instanceBlock = std::make_unique<BlockHandle>(instanceUrl, parentBlock);

    TU_ASSERT (priv->instanceType != nullptr);
    TU_ASSERT (priv->superInstance != nullptr);
}

lyric_assembler::InstanceSymbol::InstanceSymbol(
    const lyric_common::SymbolUrl &instanceUrl,
    lyric_importer::InstanceImport *instanceImport,
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_instanceUrl(instanceUrl),
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

    priv->instanceBlock = std::make_unique<BlockHandle>(
        m_instanceUrl, absl::flat_hash_map<std::string, SymbolBinding>(), m_state);

    priv->access = lyric_object::AccessType::Public;
    priv->derive = m_instanceImport->getDerive();
    priv->isAbstract = m_instanceImport->isAbstract();
    priv->isDeclOnly = m_instanceImport->isDeclOnly();

    auto *instanceType = m_instanceImport->getInstanceType();
    TU_ASSIGN_OR_RAISE (priv->instanceType, typeCache->importType(instanceType));

    auto superInstanceUrl = m_instanceImport->getSuperInstance();
    if (superInstanceUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superInstance, importCache->importInstance(superInstanceUrl));
    }

    for (auto iterator = m_instanceImport->membersBegin(); iterator != m_instanceImport->membersEnd(); iterator++) {
        FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RAISE (fieldSymbol, importCache->importField(iterator->second));
        TU_RAISE_IF_NOT_OK (priv->instanceBlock->putBinding(fieldSymbol));

        DataReference memberRef;
        memberRef.symbolUrl = iterator->second;
        memberRef.typeDef = fieldSymbol->getTypeDef();
        memberRef.referenceType = fieldSymbol->isVariable()? ReferenceType::Variable : ReferenceType::Value;
        priv->members[iterator->first] = memberRef;
    }

    for (auto iterator = m_instanceImport->methodsBegin(); iterator != m_instanceImport->methodsEnd(); iterator++) {
        CallSymbol *callSymbol;
        TU_ASSIGN_OR_RAISE (callSymbol, importCache->importCall(iterator->second));
        TU_RAISE_IF_NOT_OK (priv->instanceBlock->putBinding(callSymbol));

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
lyric_assembler::InstanceSymbol::getTypeDef() const
{
    auto *priv = getPriv();
    return priv->instanceType->getTypeDef();
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

bool
lyric_assembler::InstanceSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
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

tempo_utils::Result<lyric_assembler::FieldSymbol *>
lyric_assembler::InstanceSymbol::declareMember(
    const std::string &name,
    const lyric_common::TypeDef &memberType,
    bool isVariable,
    lyric_object::AccessType access)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare member on imported instance {}", m_instanceUrl.toString());

    auto *priv = getPriv();

    if (priv->members.contains(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "member {} already defined for instance {}", name, m_instanceUrl.toString());

    lyric_assembler::TypeHandle *fieldType;
    TU_ASSIGN_OR_RETURN (fieldType, m_state->typeCache()->getOrMakeType(memberType));

    auto memberPath = m_instanceUrl.getSymbolPath().getPath();
    memberPath.push_back(name);
    auto memberUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(memberPath));

    // construct the field symbol
    auto fieldSymbol = std::make_unique<FieldSymbol>(memberUrl, access, isVariable,
        fieldType, priv->isDeclOnly, priv->instanceBlock.get(), m_state);

    FieldSymbol *fieldPtr;
    TU_ASSIGN_OR_RETURN (fieldPtr, m_state->appendField(std::move(fieldSymbol)));
    TU_RAISE_IF_NOT_OK (priv->instanceBlock->putBinding(fieldPtr));

    DataReference ref;
    ref.symbolUrl = memberUrl;
    ref.typeDef = memberType;
    ref.referenceType = isVariable? ReferenceType::Variable : ReferenceType::Value;
    priv->members[name] = ref;

    return fieldPtr;
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
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
                "missing member {}", name);
        return priv->superInstance->resolveMember(name, reifier, receiverType, thisReceiver);
    }

    const auto &member = priv->members.at(name);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(member.symbolUrl));
    if (symbol->getSymbolType() != SymbolType::FIELD)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid field symbol {}", member.symbolUrl.toString());
    auto *fieldSymbol = cast_symbol_to_field(symbol);
    auto access = fieldSymbol->getAccessType();

    bool thisSymbol = receiverType.getConcreteUrl() == m_instanceUrl;

    if (thisReceiver) {
        if (access == lyric_object::AccessType::Private && !thisSymbol)
            return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                "access to private member {} is not allowed", name);
    } else {
        if (access != lyric_object::AccessType::Public)
            return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
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
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't set member initialized on imported instance {}", m_instanceUrl.toString());

    auto *priv = getPriv();

    if (isMemberInitialized(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidBinding,
            "member {} is already initialized", name);
    priv->initializedMembers.insert(name);
    return {};
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
    auto location = m_instanceUrl.getModuleLocation();
    auto path = m_instanceUrl.getSymbolPath();
    return lyric_common::SymbolUrl(location, lyric_common::SymbolPath(path.getPath(), "$ctor"));
}

std::string
lyric_assembler::InstanceSymbol::getAllocatorTrap() const
{
    auto *priv = getPriv();
    return priv->allocatorTrap;
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::InstanceSymbol::declareCtor(
    lyric_object::AccessType access,
    std::string allocatorTrap)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare ctor on imported instance {}", m_instanceUrl.toString());

    auto *priv = getPriv();

    auto path = m_instanceUrl.getSymbolPath().getPath();
    path.emplace_back("$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(path));

    if (m_state->symbolCache()->hasSymbol(ctorUrl))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "ctor already defined for instance {}", m_instanceUrl.toString());

    // construct call symbol
    auto ctorSymbol = std::make_unique<CallSymbol>(ctorUrl, m_instanceUrl, access,
        lyric_object::CallMode::Constructor, priv->isDeclOnly, priv->instanceBlock.get(), m_state);

    CallSymbol *ctorPtr;
    TU_ASSIGN_OR_RETURN (ctorPtr, m_state->appendCall(std::move(ctorSymbol)));
    TU_RAISE_IF_NOT_OK (priv->instanceBlock->putBinding(ctorPtr));

    // add bound method
    BoundMethod method;
    method.methodCall = ctorUrl;
    method.access = access;
    method.final = false;
    priv->methods["$ctor"] = method;

    // set allocator trap
    priv->allocatorTrap = std::move(allocatorTrap);

    return ctorPtr;
}

tempo_utils::Status
lyric_assembler::InstanceSymbol::prepareCtor(ConstructableInvoker &invoker)
{
    lyric_common::SymbolPath ctorPath = lyric_common::SymbolPath(m_instanceUrl.getSymbolPath().getPath(), "$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(m_instanceUrl.getModuleLocation(), ctorPath);

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(ctorUrl));
    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", ctorUrl.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    auto constructable = std::make_unique<CtorConstructable>(callSymbol, this);
    return invoker.initialize(std::move(constructable));
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

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::InstanceSymbol::declareMethod(
    const std::string &name,
    lyric_object::AccessType access)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare method on imported instance {}", m_instanceUrl.toString());

    auto *priv = getPriv();

    if (priv->methods.contains(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "method {} already defined for instance {}", name, m_instanceUrl.toString());

    // build reference path to function
    auto methodPath = m_instanceUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));

    // construct call symbol
    auto callSymbol = std::make_unique<CallSymbol>(methodUrl, m_instanceUrl,
        access, lyric_object::CallMode::Normal, priv->isDeclOnly, priv->instanceBlock.get(), m_state);

    CallSymbol *callPtr;
    TU_ASSIGN_OR_RETURN (callPtr, m_state->appendCall(std::move(callSymbol)));
    TU_RAISE_IF_NOT_OK (priv->instanceBlock->putBinding(callPtr));

    // add bound method
    priv->methods[name] = { methodUrl, access, true /* final */ };

    return callPtr;
}

tempo_utils::Status
lyric_assembler::InstanceSymbol::prepareMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    CallableInvoker &invoker,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    if (!priv->methods.contains(name)) {
        if (priv->superInstance == nullptr)
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMethod,
                "missing method {}", name);
        return priv->superInstance->prepareMethod(name, receiverType, invoker);
    }

    const auto &method = priv->methods.at(name);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(method.methodCall));
    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", method.methodCall.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    auto access = callSymbol->getAccessType();

    bool thisSymbol = receiverType.getConcreteUrl() == m_instanceUrl;

    if (thisReceiver) {
        if (access == lyric_object::AccessType::Private && !thisSymbol)
            return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                "invocation of private method {} is not allowed", name);
    } else {
        if (access != lyric_object::AccessType::Public)
            return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                "invocation of protected method {} is not allowed", name);
    }

    if (callSymbol->isInline()) {
        auto callable = std::make_unique<MethodCallable>(callSymbol, callSymbol->callProc());
        return invoker.initialize(std::move(callable));
    }

    if (!callSymbol->isBound())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", callSymbol->getSymbolUrl().toString());

    auto callable = std::make_unique<MethodCallable>(callSymbol, /* isInlined= */ false);
    return invoker.initialize(std::move(callable));
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

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::InstanceSymbol::declareImpl(const lyric_common::TypeDef &implType)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare impl on imported instance {}", m_instanceUrl.toString());

    auto *priv = getPriv();

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid impl type {}", implType.toString());
    auto implUrl = implType.getConcreteUrl();

    if (priv->impls.contains(implUrl))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "impl for concept {} already defined for instance {}", implUrl.toString(), m_instanceUrl.toString());

    // touch the impl type
    TypeHandle *implTypeHandle;
    TU_ASSIGN_OR_RETURN (implTypeHandle, m_state->typeCache()->getOrMakeType(implType));
    auto implConcept = implType.getConcreteUrl();

    // resolve the concept symbol
    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(implConcept));
    if (symbol->getSymbolType() != SymbolType::CONCEPT)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid concept symbol {}", implConcept.toString());
    auto *conceptSymbol = cast_symbol_to_concept(symbol);

    auto *implCache = m_state->implCache();

    auto name = absl::StrCat("$impl", priv->impls.size());

    ImplHandle *implHandle;
    TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
        name, implTypeHandle, conceptSymbol, m_instanceUrl, priv->isDeclOnly, priv->instanceBlock.get()));

    priv->impls[implUrl] = implHandle;

    return implHandle;
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
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't put sealed type on imported instance {}", m_instanceUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "instance {} is not sealed", m_instanceUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "invalid derived type {} for sealed instance {}", sealedType.toString(), m_instanceUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(sealedUrl));
    if (symbol->getSymbolType() != SymbolType::INSTANCE)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid instance symbol {}", sealedUrl.toString());

    if (cast_symbol_to_instance(symbol)->superInstance() != this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "{} does not derive from sealed instance {}", sealedType.toString(), m_instanceUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return {};
}
