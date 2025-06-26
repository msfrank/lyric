
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/ctor_constructable.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/method_callable.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/enum_import.h>

lyric_assembler::EnumSymbol::EnumSymbol(
    const lyric_common::SymbolUrl &enumUrl,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    bool isAbstract,
    TypeHandle *enumType,
    EnumSymbol *superEnum,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(new EnumSymbolPriv()),
      m_enumUrl(enumUrl),
      m_state(state)
{
    TU_ASSERT (m_enumUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = access;
    priv->derive = derive;
    priv->isAbstract = isAbstract;
    priv->isDeclOnly = isDeclOnly;
    priv->enumType = enumType;
    priv->superEnum = superEnum;
    priv->enumBlock = std::make_unique<BlockHandle>(enumUrl, parentBlock);

    TU_ASSERT (priv->enumType != nullptr);
    TU_ASSERT (priv->superEnum != nullptr);
}

lyric_assembler::EnumSymbol::EnumSymbol(
    const lyric_common::SymbolUrl &enumUrl,
    lyric_importer::EnumImport *enumImport,
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_enumUrl(enumUrl),
      m_enumImport(enumImport),
      m_state(state)
{
    TU_ASSERT (m_enumUrl.isValid());
    TU_ASSERT (m_enumImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::EnumSymbolPriv *
lyric_assembler::EnumSymbol::load()
{
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<EnumSymbolPriv>();

    priv->enumBlock = std::make_unique<BlockHandle>(
        m_enumUrl, absl::flat_hash_map<std::string, SymbolBinding>(), m_state);

    priv->access = lyric_object::AccessType::Public;
    priv->derive = m_enumImport->getDerive();
    priv->isAbstract = m_enumImport->isAbstract();
    priv->isDeclOnly = m_enumImport->isDeclOnly();

    auto *enumType = m_enumImport->getEnumType();
    TU_ASSIGN_OR_RAISE (priv->enumType, typeCache->importType(enumType));

    auto superEnumUrl = m_enumImport->getSuperEnum();
    if (superEnumUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superEnum, importCache->importEnum(superEnumUrl));
    }

    for (auto iterator = m_enumImport->membersBegin(); iterator != m_enumImport->membersEnd(); iterator++) {
        FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RAISE (fieldSymbol, importCache->importField(iterator->second));
        TU_RAISE_IF_NOT_OK (priv->enumBlock->putBinding(fieldSymbol));

        DataReference memberRef;
        memberRef.symbolUrl = iterator->second;
        memberRef.typeDef = fieldSymbol->getTypeDef();
        memberRef.referenceType = fieldSymbol->isVariable()? ReferenceType::Variable : ReferenceType::Value;
        priv->members[iterator->first] = memberRef;
    }

    for (auto iterator = m_enumImport->methodsBegin(); iterator != m_enumImport->methodsEnd(); iterator++) {
        CallSymbol *callSymbol;
        TU_ASSIGN_OR_RAISE (callSymbol, importCache->importCall(iterator->second));
        TU_RAISE_IF_NOT_OK (priv->enumBlock->putBinding(callSymbol));

        BoundMethod methodBinding;
        methodBinding.methodCall = iterator->second;
        methodBinding.access = callSymbol->getAccessType();
        methodBinding.final = false;    // FIXME: this should come from the call symbol
        priv->methods[iterator->first] = methodBinding;
    }

    auto *implCache = m_state->implCache();
    for (auto iterator = m_enumImport->implsBegin(); iterator != m_enumImport->implsEnd(); iterator++) {
        ImplHandle *implHandle;
        TU_ASSIGN_OR_RAISE (implHandle, implCache->importImpl(iterator->second));
        auto implUrl = iterator->first.getConcreteUrl();
        priv->impls[implUrl] = implHandle;
    }

    for (auto iterator = m_enumImport->sealedTypesBegin(); iterator != m_enumImport->sealedTypesEnd(); iterator++) {
        priv->sealedTypes.insert(*iterator);
    }

    priv->allocatorTrap = m_enumImport->getAllocator();

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::EnumSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Enum;
}

lyric_assembler::SymbolType
lyric_assembler::EnumSymbol::getSymbolType() const
{
    return SymbolType::ENUM;
}

lyric_common::SymbolUrl
lyric_assembler::EnumSymbol::getSymbolUrl() const
{
    return m_enumUrl;
}

lyric_common::TypeDef
lyric_assembler::EnumSymbol::getTypeDef() const
{
    auto *priv = getPriv();
    return priv->enumType->getTypeDef();
}

lyric_object::AccessType
lyric_assembler::EnumSymbol::getAccessType() const
{
    auto *priv = getPriv();
    return priv->access;
}

lyric_object::DeriveType
lyric_assembler::EnumSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
}

bool
lyric_assembler::EnumSymbol::isAbstract() const
{
    auto *priv = getPriv();
    return priv->isAbstract;
}

bool
lyric_assembler::EnumSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

lyric_assembler::TypeHandle *
lyric_assembler::EnumSymbol::enumType() const
{
    auto *priv = getPriv();
    return priv->enumType;
}

lyric_assembler::EnumSymbol *
lyric_assembler::EnumSymbol::superEnum() const
{
    auto *priv = getPriv();
    return priv->superEnum;
}

lyric_assembler::BlockHandle *
lyric_assembler::EnumSymbol::enumBlock() const
{
    auto *priv = getPriv();
    return priv->enumBlock.get();
}

bool
lyric_assembler::EnumSymbol::hasMember(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->members.contains(name);
}

Option<lyric_assembler::DataReference>
lyric_assembler::EnumSymbol::getMember(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->members.contains(name))
        return Option<DataReference>(priv->members.at(name));
    return {};
}

absl::flat_hash_map<std::string,lyric_assembler::DataReference>::const_iterator
lyric_assembler::EnumSymbol::membersBegin() const
{
    auto *priv = getPriv();
    return priv->members.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::DataReference>::const_iterator
lyric_assembler::EnumSymbol::membersEnd() const
{
    auto *priv = getPriv();
    return priv->members.cend();
}

tu_uint32
lyric_assembler::EnumSymbol::numMembers() const
{
    auto *priv = getPriv();
    return priv->members.size();
}

tempo_utils::Result<lyric_assembler::FieldSymbol *>
lyric_assembler::EnumSymbol::declareMember(
    const std::string &name,
    const lyric_common::TypeDef &memberType,
    bool isVariable,
    lyric_object::AccessType access)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare member on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();

    if (priv->members.contains(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "member {} already defined for enum {}", name, m_enumUrl.toString());

    lyric_assembler::TypeHandle *fieldType;
    TU_ASSIGN_OR_RETURN (fieldType, m_state->typeCache()->getOrMakeType(memberType));

    auto memberPath = m_enumUrl.getSymbolPath().getPath();
    memberPath.push_back(name);
    auto memberUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(memberPath));

    // construct the field symbol
    auto fieldSymbol = std::make_unique<FieldSymbol>(memberUrl, access, isVariable,
        fieldType, priv->isDeclOnly, priv->enumBlock.get(), m_state);

    FieldSymbol *fieldPtr;
    TU_ASSIGN_OR_RETURN (fieldPtr, m_state->appendField(std::move(fieldSymbol)));
    TU_RAISE_IF_NOT_OK (priv->enumBlock->putBinding(fieldPtr));

    DataReference ref;
    ref.symbolUrl = memberUrl;
    ref.typeDef = memberType;
    ref.referenceType = isVariable? ReferenceType::Variable : ReferenceType::Value;
    priv->members[name] = ref;

    return fieldPtr;
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::EnumSymbol::resolveMember(
    const std::string &name,
    AbstractMemberReifier &reifier,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    if (!priv->members.contains(name)) {
        if (priv->superEnum == nullptr)
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
                "missing member {}", name);
        return priv->superEnum->resolveMember(name, reifier, receiverType, thisReceiver);
    }
    const auto &member = priv->members.at(name);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(member.symbolUrl));
    if (symbol->getSymbolType() != SymbolType::FIELD)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid field symbol {}", member.symbolUrl.toString());
    auto *fieldSymbol = cast_symbol_to_field(symbol);
    auto access = fieldSymbol->getAccessType();

    bool thisSymbol = receiverType.getConcreteUrl() == m_enumUrl;

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
lyric_assembler::EnumSymbol::isMemberInitialized(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->initializedMembers.contains(name);
}

tempo_utils::Status
lyric_assembler::EnumSymbol::setMemberInitialized(const std::string &name)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't set member initialized on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();
    if (isMemberInitialized(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidBinding,
            "member {} is already initialized", name);
    priv->initializedMembers.insert(name);
    return {};
}

bool
lyric_assembler::EnumSymbol::isCompletelyInitialized() const
{
    auto *priv = getPriv();
    for (const auto &member : priv->members) {
        if (!priv->initializedMembers.contains(member.first))
            return false;
    }
    return true;
}

lyric_common::SymbolUrl
lyric_assembler::EnumSymbol::getCtor() const
{
    auto location = m_enumUrl.getModuleLocation();
    auto path = m_enumUrl.getSymbolPath();
    return lyric_common::SymbolUrl(location, lyric_common::SymbolPath(path.getPath(), "$ctor"));
}

std::string
lyric_assembler::EnumSymbol::getAllocatorTrap() const
{
    auto *priv = getPriv();
    return priv->allocatorTrap;
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::EnumSymbol::declareCtor(
    lyric_object::AccessType access,
    std::string allocatorTrap)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare ctor on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();

    auto path = m_enumUrl.getSymbolPath().getPath();
    path.emplace_back("$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(path));

    if (m_state->symbolCache()->hasSymbol(ctorUrl))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "ctor already defined for enum {}", m_enumUrl.toString());

    // construct call symbol
    auto ctorSymbol = std::make_unique<CallSymbol>(ctorUrl, m_enumUrl, access,
        lyric_object::CallMode::Constructor, priv->isDeclOnly, priv->enumBlock.get(), m_state);

    CallSymbol *ctorPtr;
    TU_ASSIGN_OR_RETURN (ctorPtr, m_state->appendCall(std::move(ctorSymbol)));
    TU_RAISE_IF_NOT_OK (priv->enumBlock->putBinding(ctorPtr));

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
lyric_assembler::EnumSymbol::prepareCtor(ConstructableInvoker &invoker)
{
    lyric_common::SymbolPath ctorPath = lyric_common::SymbolPath(m_enumUrl.getSymbolPath().getPath(), "$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(m_enumUrl.getModuleLocation(), ctorPath);

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
lyric_assembler::EnumSymbol::hasMethod(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

Option<lyric_assembler::BoundMethod>
lyric_assembler::EnumSymbol::getMethod(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->methods.contains(name))
        return Option<BoundMethod>(priv->methods.at(name));
    return {};
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
lyric_assembler::EnumSymbol::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
lyric_assembler::EnumSymbol::methodsEnd() const
{
    auto *priv = getPriv();
    return priv->methods.cend();
}

tu_uint32
lyric_assembler::EnumSymbol::numMethods() const
{
    auto *priv = getPriv();
    return priv->methods.size();
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::EnumSymbol::declareMethod(
    const std::string &name,
    lyric_object::AccessType access)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare method on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();

    if (priv->methods.contains(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "method {} already defined for enum {}", name, m_enumUrl.toString());

    // build reference path to function
    auto methodPath = m_enumUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));

    // construct call symbol
    auto callSymbol = std::make_unique<CallSymbol>(methodUrl, m_enumUrl, access,
        lyric_object::CallMode::Normal, priv->isDeclOnly, priv->enumBlock.get(), m_state);

    CallSymbol *callPtr;
    TU_ASSIGN_OR_RETURN (callPtr, m_state->appendCall(std::move(callSymbol)));
    TU_RAISE_IF_NOT_OK (priv->enumBlock->putBinding(callPtr));

    // add bound method
    priv->methods[name] = { methodUrl, access, true /* final */ };

    return callPtr;
}

tempo_utils::Status
lyric_assembler::EnumSymbol::prepareMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    CallableInvoker &invoker,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    if (!priv->methods.contains(name)) {
        if (priv->superEnum == nullptr)
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMethod,
                "missing method {}", name);
        return priv->superEnum->prepareMethod(name, receiverType, invoker);
    }

    const auto &method = priv->methods.at(name);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(method.methodCall));
    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", method.methodCall.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    auto access = callSymbol->getAccessType();

    bool thisSymbol = receiverType.getConcreteUrl() == m_enumUrl;

    if (thisReceiver) {
        if (access == lyric_object::AccessType::Private && !thisSymbol)
            return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                "cannot access private method {} on {}", name, m_enumUrl.toString());
    } else {
        if (access != lyric_object::AccessType::Public)
            return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                "cannot access protected method {} on {}", name, m_enumUrl.toString());
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
lyric_assembler::EnumSymbol::hasImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    return priv->impls.contains(implUrl);
}

bool
lyric_assembler::EnumSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    return hasImpl(implType.getConcreteUrl());
}

lyric_assembler::ImplHandle *
lyric_assembler::EnumSymbol::getImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    auto iterator = priv->impls.find(implUrl);
    if (iterator != priv->impls.cend())
        return iterator->second;
    return nullptr;
}

lyric_assembler::ImplHandle *
lyric_assembler::EnumSymbol::getImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return nullptr;
    return getImpl(implType.getConcreteUrl());
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::EnumSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::EnumSymbol::implsEnd() const
{
    auto *priv = getPriv();
    return priv->impls.cend();
}

tu_uint32
lyric_assembler::EnumSymbol::numImpls() const
{
    auto *priv = getPriv();
    return priv->impls.size();
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::EnumSymbol::declareImpl(const lyric_common::TypeDef &implType)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare impl on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid impl type {}", implType.toString());
    auto implUrl = implType.getConcreteUrl();

    if (priv->impls.contains(implUrl))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "impl {} already defined for enum {}", implType.toString(), m_enumUrl.toString());

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
        name, implTypeHandle, conceptSymbol, m_enumUrl, priv->isDeclOnly, priv->enumBlock.get()));

    priv->impls[implUrl] = implHandle;

    return implHandle;
}

bool
lyric_assembler::EnumSymbol::hasSealedType(const lyric_common::TypeDef &sealedType) const
{
    auto *priv = getPriv();
    return priv->sealedTypes.contains(sealedType);
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::EnumSymbol::sealedTypesBegin() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::EnumSymbol::sealedTypesEnd() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cend();
}

tempo_utils::Status
lyric_assembler::EnumSymbol::putSealedType(const lyric_common::TypeDef &sealedType)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't put sealed type on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "enum {} is not sealed", m_enumUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "invalid derived type {} for sealed enum {}", sealedType.toString(), m_enumUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(sealedUrl));
    if (symbol->getSymbolType() != SymbolType::ENUM)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid enum symbol {}", sealedUrl.toString());

    if (cast_symbol_to_enum(symbol)->superEnum() != this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "{} does not derive from sealed enum {}", sealedType.toString(), m_enumUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return {};
}