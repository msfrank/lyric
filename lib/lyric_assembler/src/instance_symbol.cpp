
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
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/impl_import.h>
#include <lyric_importer/instance_import.h>

lyric_assembler::InstanceSymbol::InstanceSymbol(
    const lyric_common::SymbolUrl &instanceUrl,
    bool isHidden,
    bool isAbstract,
    lyric_object::DeriveType derive,
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
    priv->isHidden = isHidden;
    priv->isAbstract = isAbstract;
    priv->derive = derive;
    priv->isDeclOnly = isDeclOnly;
    priv->instanceType = instanceType;
    priv->superInstance = superInstance;
    priv->instanceBlock = std::make_unique<BlockHandle>(instanceUrl, parentBlock);

    TU_ASSERT (priv->instanceType != nullptr);
    TU_ASSERT (priv->superInstance != nullptr);
}

lyric_assembler::InstanceSymbol::InstanceSymbol(
    const lyric_common::SymbolUrl &instanceUrl,
    std::shared_ptr<lyric_importer::InstanceImport> instanceImport,
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_instanceUrl(instanceUrl),
      m_instanceImport(std::move(instanceImport)),
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

    priv->isHidden = m_instanceImport->isHidden();
    priv->isAbstract = m_instanceImport->isAbstract();
    priv->derive = m_instanceImport->getDerive();
    priv->isDeclOnly = m_instanceImport->isDeclOnly();

    auto typeImport = m_instanceImport->getInstanceType().lock();
    if (typeImport == nullptr)
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kImportError,
            "cannot import instance {}; missing type",
            m_instanceUrl.toString()));
    TU_ASSIGN_OR_RAISE (priv->instanceType, typeCache->importType(typeImport));

    auto superInstanceUrl = m_instanceImport->getSuperInstance();
    if (superInstanceUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superInstance, importCache->importInstance(superInstanceUrl));
    }

    for (auto it = m_instanceImport->membersBegin(); it != m_instanceImport->membersEnd(); it++) {
        FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RAISE (fieldSymbol, importCache->importField(it->second));
        TU_RAISE_IF_NOT_OK (priv->instanceBlock->putBinding(fieldSymbol));
        priv->members[it->first] = fieldSymbol;
    }

    for (auto it = m_instanceImport->methodsBegin(); it != m_instanceImport->methodsEnd(); it++) {
        CallSymbol *callSymbol;
        TU_ASSIGN_OR_RAISE (callSymbol, importCache->importCall(it->second));
        TU_RAISE_IF_NOT_OK (priv->instanceBlock->putBinding(callSymbol));
        priv->methods[it->first] = callSymbol;
    }

    auto *implCache = m_state->implCache();
    for (auto it = m_instanceImport->implsBegin(); it != m_instanceImport->implsEnd(); it++) {
        auto implImport = it->second.lock();
        if (implImport == nullptr)
            throw tempo_utils::StatusException(AssemblerStatus::forCondition(
                AssemblerCondition::kImportError, "invalid impl import"));
        ImplHandle *implHandle;
        TU_ASSIGN_OR_RAISE (implHandle, implCache->importImpl(implImport));
        auto contract = implHandle->getContract();

        auto consumerType = contract.getConsumerType();
        if (priv->impls.contains(consumerType))
            throw tempo_utils::StatusException(AssemblerStatus::forCondition(
                AssemblerCondition::kImportError, "impl {} is already imported for {}",
                    consumerType.toString(), m_instanceUrl.toString()));
        priv->impls[consumerType] = implHandle;

        auto implementationType = contract.getImplementationType();
        if (implementationType == consumerType)
            continue;
        if (priv->impls.contains(implementationType))
            throw tempo_utils::StatusException(AssemblerStatus::forCondition(
                AssemblerCondition::kImportError, "impl {} is already imported for {}",
                    implementationType.toString(), m_instanceUrl.toString()));
        priv->impls[implementationType] = implHandle;
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

lyric_assembler::BlockHandle *
lyric_assembler::InstanceSymbol::derefBlock()
{
    auto *priv = getPriv();
    return priv->instanceBlock.get();
}

bool
lyric_assembler::InstanceSymbol::isHidden() const
{
    auto *priv = getPriv();
    return priv->isHidden;
}

bool
lyric_assembler::InstanceSymbol::isAbstract() const
{
    auto *priv = getPriv();
    return priv->isAbstract;
}

lyric_object::DeriveType
lyric_assembler::InstanceSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
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

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::InstanceSymbol::resolveGlobalMember(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *symbolCache = m_state->symbolCache();
    auto *priv = getPriv();

    auto globalSymbolUrl = priv->instanceBlock->makeSymbolUrl(name);

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(globalSymbolUrl, /* allowMissing= */ true));
    if (symbol == nullptr) {
        if (priv->superInstance == nullptr)
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
                "missing global member {}", name);
        return priv->superInstance->resolveGlobalMember(name, receiverType, thisReceiver);
    }

    DataReference ref;
    bool isHidden;
    switch (symbol->getSymbolType()) {
        case SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(symbol);
            ref.referenceType = ReferenceType::Value;
            isHidden = enumSymbol->isHidden();
            break;
        }
        case SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(symbol);
            ref.referenceType = ReferenceType::Value;
            isHidden = instanceSymbol->isHidden();
            break;
        }
        case SymbolType::STATIC: {
            auto *staticSymbol = cast_symbol_to_static(symbol);
            ref.referenceType = staticSymbol->isVariable()? ReferenceType::Variable : ReferenceType::Value;
            isHidden = staticSymbol->isHidden();
            break;
        }
        default:
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
                "missing member {}", name);
    }

    bool thisSymbol = receiverType.getConcreteUrl() == m_instanceUrl;
    if (isHidden && !(thisReceiver && thisSymbol))
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
            "access to hidden member {} is not allowed", name);
    ref.symbolUrl = globalSymbolUrl;
    ref.typeDef = symbol->getTypeDef();
    return ref;
}

tempo_utils::Status
lyric_assembler::InstanceSymbol::prepareGlobalMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    std::unique_ptr<AbstractCallable> &callable,
    bool thisReceiver) const
{
    auto *symbolCache = m_state->symbolCache();
    auto *priv = getPriv();

    auto globalSymbolUrl = priv->instanceBlock->makeSymbolUrl(name);

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(globalSymbolUrl, /* allowMissing= */ true));
    if (symbol == nullptr) {
        if (priv->superInstance == nullptr)
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMethod,
                "missing global method {}", name);
        return priv->superInstance->prepareGlobalMethod(name, receiverType, callable, thisReceiver);
    }

    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", symbol->getSymbolUrl().toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    if (callSymbol->isHidden()) {
        if (!thisReceiver)
            return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                "cannot access hidden method {} on {}", name, m_instanceUrl.toString());
    }

    if (callSymbol->isBound())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", callSymbol->getSymbolUrl().toString());

    callable = std::make_unique<FunctionCallable>(callSymbol, callSymbol->isInline());
    return {};
}

bool
lyric_assembler::InstanceSymbol::hasMember(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->members.contains(name);
}

lyric_assembler::FieldSymbol *
lyric_assembler::InstanceSymbol::getMember(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->members.find(name);
    if (entry != priv->members.cend())
        return entry->second;
    return nullptr;
}

absl::flat_hash_map<std::string,lyric_assembler::FieldSymbol *>::const_iterator
lyric_assembler::InstanceSymbol::membersBegin() const
{
    auto *priv = getPriv();
    return priv->members.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::FieldSymbol *>::const_iterator
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
    bool isHidden)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare member on imported instance {}", m_instanceUrl.toString());

    auto *priv = getPriv();

    if (priv->members.contains(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "member {} already defined for instance {}", name, m_instanceUrl.toString());

    TypeHandle *fieldType;
    TU_ASSIGN_OR_RETURN (fieldType, m_state->typeCache()->getOrMakeType(memberType));

    auto memberPath = m_instanceUrl.getSymbolPath().getPath();
    memberPath.push_back(name);
    auto memberUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(memberPath));

    // construct the field symbol
    auto fieldSymbol = std::make_unique<FieldSymbol>(memberUrl, isHidden, isVariable,
        fieldType, priv->isDeclOnly, priv->instanceBlock.get(), m_state);

    FieldSymbol *fieldPtr;
    TU_ASSIGN_OR_RETURN (fieldPtr, m_state->appendField(std::move(fieldSymbol)));
    TU_RAISE_IF_NOT_OK (priv->instanceBlock->putBinding(fieldPtr));
    priv->members[name] = fieldPtr;

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

    auto entry = priv->members.find(name);
    if (entry != priv->members.cend()) {
        auto *fieldSymbol = entry->second;

        if (fieldSymbol->isHidden()) {
            bool thisSymbol = receiverType.getConcreteUrl() == m_instanceUrl;
            if (!(thisReceiver && thisSymbol))
                return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                    "access to hidden member {} is not allowed", name);
        }

        return reifier.reifyMember(name, fieldSymbol);
    }

    if (priv->superInstance == nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
            "missing member {}", name);
    return priv->superInstance->resolveMember(name, reifier, receiverType, thisReceiver);
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

std::string
lyric_assembler::InstanceSymbol::getAllocatorTrap() const
{
    auto *priv = getPriv();
    return priv->allocatorTrap;
}

bool
lyric_assembler::InstanceSymbol::hasCtor(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->methods.find(name);
    if (entry == priv->methods.cend())
        return false;
    auto *callSymbol = entry->second;
    return callSymbol->isCtor();
}

lyric_assembler::CallSymbol *
lyric_assembler::InstanceSymbol::getCtor(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->methods.find(name);
    if (entry == priv->methods.cend())
        return {};
    auto *callSymbol = entry->second;
    if (callSymbol->isCtor())
        return callSymbol;
    return nullptr;
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::InstanceSymbol::declareCtor(
    bool isHidden,
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
    auto ctorSymbol = std::make_unique<CallSymbol>(ctorUrl, m_instanceUrl, isHidden,
        lyric_object::CallMode::Constructor, /* isFinal = */ false, priv->isDeclOnly,
        priv->instanceBlock.get(), m_state);

    CallSymbol *ctorPtr;
    TU_ASSIGN_OR_RETURN (ctorPtr, m_state->appendCall(std::move(ctorSymbol)));
    TU_RAISE_IF_NOT_OK (priv->instanceBlock->putBinding(ctorPtr));
    priv->methods["$ctor"] = ctorPtr;

    // set allocator trap
    priv->allocatorTrap = std::move(allocatorTrap);

    return ctorPtr;
}

tempo_utils::Status
lyric_assembler::InstanceSymbol::prepareCtor(std::unique_ptr<AbstractCallable> &callable)
{
    lyric_common::SymbolPath ctorPath = lyric_common::SymbolPath(
        m_instanceUrl.getSymbolPath().getPath(), "$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(m_instanceUrl.getModuleLocation(), ctorPath);

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(ctorUrl));
    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", ctorUrl.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    callable = std::make_unique<CtorConstructable>(callSymbol, this);
    return {};
}

bool
lyric_assembler::InstanceSymbol::hasMethod(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

lyric_assembler::CallSymbol *
lyric_assembler::InstanceSymbol::getMethod(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->methods.find(name);
    if (entry != priv->methods.cend())
        return entry->second;
    return nullptr;
}

absl::flat_hash_map<std::string,lyric_assembler::CallSymbol *>::const_iterator
lyric_assembler::InstanceSymbol::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::CallSymbol *>::const_iterator
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
    bool isHidden,
    bool isFinal)
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
        isHidden, lyric_object::CallMode::Normal, isFinal, priv->isDeclOnly,
        priv->instanceBlock.get(), m_state);

    CallSymbol *callPtr;
    TU_ASSIGN_OR_RETURN (callPtr, m_state->appendCall(std::move(callSymbol)));
    TU_RAISE_IF_NOT_OK (priv->instanceBlock->putBinding(callPtr));
    priv->methods[name] = callPtr;

    return callPtr;
}

tempo_utils::Status
lyric_assembler::InstanceSymbol::prepareMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    std::unique_ptr<AbstractCallable> &callable,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    auto entry = priv->methods.find(name);
    if (entry != priv->methods.cend()) {
        auto *callSymbol = entry->second;

        if (callSymbol->isHidden()) {
            if (!thisReceiver)
                return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                    "cannot access hidden method {} on {}", name, m_instanceUrl.toString());
        }

        if (!callSymbol->isBound())
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid call symbol {}", callSymbol->getSymbolUrl().toString());

        callable = std::make_unique<MethodCallable>(callSymbol, callSymbol->isInline());
        return {};
    }

    if (priv->superInstance == nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingMethod,
            "missing method {}", name);
    return priv->superInstance->prepareMethod(name, receiverType, callable);
}

bool
lyric_assembler::InstanceSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    auto *priv = getPriv();
    return priv->impls.contains(implType);
}

lyric_assembler::ImplHandle *
lyric_assembler::InstanceSymbol::getImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return nullptr;
    auto *priv = getPriv();
    auto iterator = priv->impls.find(implType);
    if (iterator != priv->impls.cend())
        return iterator->second;
    return nullptr;
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::InstanceSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::ImplHandle *>::const_iterator
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

    if (priv->impls.contains(implType))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "impl for concept {} already defined for instance {}", implType.toString(), m_instanceUrl.toString());

    auto *implCache = m_state->implCache();

    ImplHandle *implHandle;
    TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(implType, m_instanceUrl,
        priv->isDeclOnly, priv->instanceBlock.get()));

    priv->impls[implType] = implHandle;

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
