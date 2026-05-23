
#include <lyric_assembler/action_symbol.h>
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
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/stub_callable.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/enum_import.h>

lyric_assembler::EnumSymbol::EnumSymbol(
    const lyric_common::SymbolUrl &enumUrl,
    bool isHidden,
    bool isAbstract,
    lyric_object::DeriveType derive,
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
    priv->isHidden = isHidden;
    priv->isAbstract = isAbstract;
    priv->derive = derive;
    priv->isDeclOnly = isDeclOnly;
    priv->enumBlock = std::make_unique<BlockHandle>(enumUrl, parentBlock);
}

lyric_assembler::EnumSymbol::EnumSymbol(
    const lyric_common::SymbolUrl &enumUrl,
    std::shared_ptr<lyric_importer::EnumImport> enumImport,
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_enumUrl(enumUrl),
      m_enumImport(std::move(enumImport)),
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

    priv->isHidden = m_enumImport->isHidden();
    priv->isAbstract = m_enumImport->isAbstract();
    priv->derive = m_enumImport->getDerive();
    priv->isDeclOnly = m_enumImport->isDeclOnly();

    auto typeImport = m_enumImport->getEnumType().lock();
    if (typeImport == nullptr)
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kImportError,
            "cannot import enum {}; missing type",
            m_enumUrl.toString()));
    TU_ASSIGN_OR_RAISE (priv->enumType, typeCache->importType(typeImport));

    auto superEnumUrl = m_enumImport->getSuperEnum();
    if (superEnumUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superEnum, importCache->importEnum(superEnumUrl));

        auto superTypeImport = m_enumImport->getSuperType().lock();
        if (superTypeImport == nullptr)
            throw tempo_utils::StatusException(
                AssemblerStatus::forCondition(AssemblerCondition::kImportError,
                "cannot import enum {}; missing supertype",
                m_enumUrl.toString()));
        TU_ASSIGN_OR_RAISE (priv->superType, typeCache->importType(superTypeImport));
    }

    for (auto it = m_enumImport->membersBegin(); it != m_enumImport->membersEnd(); it++) {
        FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RAISE (fieldSymbol, importCache->importField(it->second));
        TU_RAISE_IF_NOT_OK (priv->enumBlock->putBinding(fieldSymbol));
        priv->members[it->first] = fieldSymbol;
    }

    for (auto it = m_enumImport->methodsBegin(); it != m_enumImport->methodsEnd(); it++) {
        CallSymbol *callSymbol;
        TU_ASSIGN_OR_RAISE (callSymbol, importCache->importCall(it->second));
        TU_RAISE_IF_NOT_OK (priv->enumBlock->putBinding(callSymbol));
        priv->methods[it->first] = callSymbol;
    }

    for (auto it = m_enumImport->stubsBegin(); it != m_enumImport->stubsEnd(); it++) {
        ActionSymbol *actionSymbol;
        TU_ASSIGN_OR_RAISE (actionSymbol, importCache->importAction(it->second));
        TU_RAISE_IF_NOT_OK (priv->enumBlock->putBinding(actionSymbol));
        priv->stubs[it->first] = actionSymbol;
    }

    if (!priv->stubs.empty() && !priv->isAbstract)
        throw tempo_utils::StatusException(AssemblerStatus::forCondition(
            AssemblerCondition::kImportError,
            "cannot import enum {}; enum has stubs but is not declared abstract",
            m_enumUrl.toString()));

    auto *implCache = m_state->implCache();
    for (auto it = m_enumImport->implsBegin(); it != m_enumImport->implsEnd(); it++) {
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
                    consumerType.toString(), m_enumUrl.toString()));
        priv->impls[consumerType] = implHandle;

        auto implementationType = contract.getImplementationType();
        if (implementationType == consumerType)
            continue;
        if (priv->impls.contains(implementationType))
            throw tempo_utils::StatusException(AssemblerStatus::forCondition(
                AssemblerCondition::kImportError, "impl {} is already imported for {}",
                    implementationType.toString(), m_enumUrl.toString()));
        priv->impls[implementationType] = implHandle;
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

bool
lyric_assembler::EnumSymbol::isHidden() const
{
    auto *priv = getPriv();
    return priv->isHidden;
}

bool
lyric_assembler::EnumSymbol::isAbstract() const
{
    auto *priv = getPriv();
    return priv->isAbstract;
}

lyric_assembler::BlockHandle *
lyric_assembler::EnumSymbol::derefBlock()
{
    auto *priv = getPriv();
    return priv->enumBlock.get();
}

lyric_object::DeriveType
lyric_assembler::EnumSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
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

lyric_assembler::TypeHandle *
lyric_assembler::EnumSymbol::superType() const
{
    auto *priv = getPriv();
    return priv->superEnum->enumType();
}

lyric_assembler::BlockHandle *
lyric_assembler::EnumSymbol::enumBlock() const
{
    auto *priv = getPriv();
    return priv->enumBlock.get();
}

lyric_assembler::AbstractResolver *
lyric_assembler::EnumSymbol::enumResolver() const
{
    auto *priv = getPriv();
    return priv->enumBlock.get();
}

tempo_utils::Status
lyric_assembler::EnumSymbol::finalizeEnum(const lyric_common::TypeDef &superEnumType)
{
    auto *typeCache = m_state->typeCache();

    auto *priv = getPriv();

    if (priv->superType != nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "{} is already finalized", m_enumUrl.toString());

    EnumSymbol *superEnum;
    TU_ASSIGN_OR_RETURN (superEnum, priv->enumBlock->resolveEnum(superEnumType));

    auto superDerive = superEnum->getDeriveType();
    if (superDerive == lyric_object::DeriveType::Final)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
            "cannot derive enum {} from {}; base enum is marked final",
            m_enumUrl.getSymbolPath().toString(), superEnum->getSymbolUrl().toString());
    if (superDerive == lyric_object::DeriveType::Sealed && superEnum->isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
            "cannot derive enum {} from {}; sealed base enum must be located in the same module",
            m_enumUrl.getSymbolPath().toString(), superEnum->getSymbolUrl().toString());

    // create the supertype if necessary
    auto *supertypeHandle = superEnum->enumType();

    // create the type
    TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, typeCache->declareSubType(m_enumUrl, {}, superEnumType));

    priv->superEnum = superEnum;
    priv->superType = supertypeHandle;
    priv->enumType = typeHandle;
    return {};
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::EnumSymbol::resolveGlobalMember(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *symbolCache = m_state->symbolCache();
    auto *priv = getPriv();

    auto globalSymbolUrl = priv->enumBlock->makeSymbolUrl(name);

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(globalSymbolUrl, /* allowMissing= */ true));
    if (symbol == nullptr) {
        if (priv->superEnum == nullptr)
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
                "missing global member {}", name);
        return priv->superEnum->resolveGlobalMember(name, receiverType, thisReceiver);
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

    bool thisSymbol = receiverType.getConcreteUrl() == m_enumUrl;
    if (isHidden && !(thisReceiver && thisSymbol))
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
            "access to hidden member {} is not allowed", name);
    ref.symbolUrl = globalSymbolUrl;
    ref.typeDef = symbol->getTypeDef();
    return ref;
}

tempo_utils::Status
lyric_assembler::EnumSymbol::prepareGlobalMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    std::unique_ptr<AbstractCallable> &callable,
    bool thisReceiver) const
{
    auto *symbolCache = m_state->symbolCache();
    auto *priv = getPriv();

    auto globalSymbolUrl = priv->enumBlock->makeSymbolUrl(name);

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(globalSymbolUrl, /* allowMissing= */ true));
    if (symbol == nullptr) {
        if (priv->superEnum == nullptr)
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMethod,
                "missing global method {}", name);
        return priv->superEnum->prepareGlobalMethod(name, receiverType, callable, thisReceiver);
    }

    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", symbol->getSymbolUrl().toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    if (callSymbol->isHidden()) {
        if (!thisReceiver)
            return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                "cannot access hidden method {} on {}", name, m_enumUrl.toString());
    }

    if (callSymbol->isBound())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", callSymbol->getSymbolUrl().toString());

    callable = std::make_unique<FunctionCallable>(callSymbol, callSymbol->isInline());
    return {};
}

bool
lyric_assembler::EnumSymbol::hasMember(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->members.contains(name);
}

lyric_assembler::FieldSymbol *
lyric_assembler::EnumSymbol::getMember(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->members.find(name);
    if (entry != priv->members.cend())
        return entry->second;
    return nullptr;
}

absl::flat_hash_map<std::string,lyric_assembler::FieldSymbol *>::const_iterator
lyric_assembler::EnumSymbol::membersBegin() const
{
    auto *priv = getPriv();
    return priv->members.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::FieldSymbol *>::const_iterator
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

static lyric_assembler::AbstractSymbol *
find_existing_or_overridden_enum_binding(
    lyric_assembler::EnumSymbol *enumSymbol,
    const std::string &name)
{
    // if enum contains the named binding then return the EnumSymbol pointer
    auto *block = enumSymbol->enumBlock();
    if (block->hasBinding(name))
        return enumSymbol;

    // otherwise if a superenum contains the named binding then return pointer to the binding symbol
    for (auto *currEnum = enumSymbol->superEnum(); currEnum != nullptr; currEnum = currEnum->superEnum()) {
        block = currEnum->enumBlock();
        if (!block->hasBinding(name))
            continue;

        auto binding = block->getBinding(name);
        if (binding.bindingType == lyric_assembler::BindingType::Descriptor) {
            auto *state = block->blockState();
            auto *symbolCache = state->symbolCache();
            return symbolCache->getSymbolOrNull(binding.symbolUrl);
        }
    }

    // otherwise no binding exists in any superenum
    return nullptr;
}

tempo_utils::Result<lyric_assembler::FieldSymbol *>
lyric_assembler::EnumSymbol::declareMember(
    const std::string &name,
    const lyric_common::TypeDef &memberType,
    bool isVariable,
    bool isHidden)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare member on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();

    auto *existingOrOverridden = find_existing_or_overridden_enum_binding(this, name);
    if (existingOrOverridden == this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "'{}' is already defined for enum {}", name, m_enumUrl.toString());
    if (existingOrOverridden != nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "{} cannot be overridden by enum {}",
            existingOrOverridden->getSymbolUrl().toString(), m_enumUrl.toString());

    TypeHandle *fieldType;
    TU_ASSIGN_OR_RETURN (fieldType, m_state->typeCache()->getOrMakeType(memberType));

    auto memberPath = m_enumUrl.getSymbolPath().getPath();
    memberPath.push_back(name);
    auto memberUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(memberPath));

    // construct the field symbol
    auto fieldSymbol = std::make_unique<FieldSymbol>(memberUrl, isHidden, isVariable,
        fieldType, priv->isDeclOnly, priv->enumBlock.get(), m_state);

    FieldSymbol *fieldPtr;
    TU_ASSIGN_OR_RETURN (fieldPtr, m_state->appendField(std::move(fieldSymbol)));
    TU_RAISE_IF_NOT_OK (priv->enumBlock->putBinding(fieldPtr));
    priv->members[name] = fieldPtr;

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

    auto entry = priv->members.find(name);
    if (entry != priv->members.cend()) {
        auto *fieldSymbol = entry->second;

        if (fieldSymbol->isHidden()) {
            bool thisSymbol = receiverType.getConcreteUrl() == m_enumUrl;
            if (!(thisReceiver && thisSymbol))
                return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                    "access to hidden member {} is not allowed", name);
        }

        return reifier.reifyMember(name, fieldSymbol);
    }

    if (priv->superEnum == nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
            "missing member {}", name);
    return priv->superEnum->resolveMember(name, reifier, receiverType, thisReceiver);
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

std::string
lyric_assembler::EnumSymbol::getAllocatorTrap() const
{
    auto *priv = getPriv();
    return priv->allocatorTrap;
}

bool
lyric_assembler::EnumSymbol::hasCtor(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->methods.find(name);
    if (entry == priv->methods.cend())
        return false;
    auto *callSymbol = entry->second;
    return callSymbol->isCtor();
}

lyric_assembler::CallSymbol *
lyric_assembler::EnumSymbol::getCtor(const std::string &name) const
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
lyric_assembler::EnumSymbol::declareCtor(
    bool isHidden,
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
    auto ctorSymbol = std::make_unique<CallSymbol>(ctorUrl, m_enumUrl, isHidden,
        lyric_object::CallMode::Constructor, /* isFinal= */ false, priv->isDeclOnly,
        priv->enumBlock.get(), m_state);

    CallSymbol *ctorPtr;
    TU_ASSIGN_OR_RETURN (ctorPtr, m_state->appendCall(std::move(ctorSymbol)));
    TU_RAISE_IF_NOT_OK (priv->enumBlock->putBinding(ctorPtr));
    priv->methods["$ctor"] = ctorPtr;

    // set allocator trap
    priv->allocatorTrap = std::move(allocatorTrap);

    return ctorPtr;
}

tempo_utils::Status
lyric_assembler::EnumSymbol::prepareCtor(std::unique_ptr<AbstractCallable> &callable)
{
    lyric_common::SymbolPath ctorPath = lyric_common::SymbolPath(m_enumUrl.getSymbolPath().getPath(), "$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(m_enumUrl.getModuleLocation(), ctorPath);

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
lyric_assembler::EnumSymbol::hasMethod(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

lyric_assembler::CallSymbol *
lyric_assembler::EnumSymbol::getMethod(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->methods.find(name);
    if (entry != priv->methods.cend())
        return entry->second;
    return nullptr;
}

absl::flat_hash_map<std::string,lyric_assembler::CallSymbol *>::const_iterator
lyric_assembler::EnumSymbol::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::CallSymbol *>::const_iterator
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
    bool isHidden,
    bool isFinal)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare method on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();

    auto *existingOrOverridden = find_existing_or_overridden_enum_binding(this, name);
    if (existingOrOverridden == this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "{} already defined for enum {}", name, m_enumUrl.toString());

    // determine the base symbol if it exists
    lyric_common::SymbolUrl baseUrl;
    if (existingOrOverridden != nullptr) {
        switch (existingOrOverridden->getSymbolType()) {
            case SymbolType::ACTION:
                baseUrl = existingOrOverridden->getSymbolUrl();
                break;
            case SymbolType::CALL: {
                auto *baseCall = cast_symbol_to_call(existingOrOverridden);
                if (baseCall->isFinal())
                    return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
                        "final method {} cannot be overridden by enum {}",
                        existingOrOverridden->getSymbolUrl().toString(), m_enumUrl.toString());
                baseUrl = existingOrOverridden->getSymbolUrl();
                break;
            }
            default:
                return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
                    "{} cannot be overridden by enum {}",
                    existingOrOverridden->getSymbolUrl().toString(), m_enumUrl.toString());
        }
    }

    // build reference path to function
    auto methodPath = m_enumUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));

    // construct call symbol
    std::unique_ptr<CallSymbol> callSymbol;
    if (baseUrl.isValid()) {
        callSymbol = std::make_unique<CallSymbol>(methodUrl, m_enumUrl, isHidden, baseUrl, isFinal,
            priv->isDeclOnly, priv->enumBlock.get(), m_state);
    } else {
        callSymbol = std::make_unique<CallSymbol>(methodUrl, m_enumUrl, isHidden,
            lyric_object::CallMode::Normal, isFinal, priv->isDeclOnly, priv->enumBlock.get(),
            m_state);
    }

    CallSymbol *callPtr;
    TU_ASSIGN_OR_RETURN (callPtr, m_state->appendCall(std::move(callSymbol)));
    TU_RAISE_IF_NOT_OK (priv->enumBlock->putBinding(callPtr));
    priv->methods[name] = callPtr;

    return callPtr;
}

tempo_utils::Status
lyric_assembler::EnumSymbol::prepareMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    std::unique_ptr<AbstractCallable> &callable,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    auto method = priv->methods.find(name);
    if (method != priv->methods.cend()) {
        auto *callSymbol = method->second;

        if (callSymbol->isHidden()) {
            if (!thisReceiver)
                return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                    "cannot access hidden method {} on {}", name, m_enumUrl.toString());
        }

        if (!callSymbol->isBound())
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid call symbol {}", callSymbol->getSymbolUrl().toString());

        if (!callSymbol->hasBaseUrl()) {
            callable = std::make_unique<MethodCallable>(callSymbol, callSymbol->isInline());
            return {};
        }

        auto *symbolCache = m_state->symbolCache();

        AbstractSymbol *baseSymbol;
        TU_ASSIGN_OR_RETURN (baseSymbol, symbolCache->getOrImportSymbol(callSymbol->getBaseUrl()));
        switch (baseSymbol->getSymbolType()) {
            case SymbolType::ACTION:
                callable = std::make_unique<StubCallable>(cast_symbol_to_action(baseSymbol));
                return {};
            case SymbolType::CALL:
                callable = std::make_unique<MethodCallable>(cast_symbol_to_call(baseSymbol));
                return {};
            default:
                return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                    "invalid base symbol {}", baseSymbol->getSymbolUrl().toString());
        }
    }

    auto stub = priv->stubs.find(name);
    if (stub != priv->stubs.cend()) {
        auto *actionSymbol = stub->second;

        if (actionSymbol->isHidden()) {
            if (!thisReceiver)
                return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                    "cannot access hidden method {} on {}", name, m_enumUrl.toString());
        }

        callable = std::make_unique<StubCallable>(actionSymbol);
        return {};
    }

    if (priv->superEnum == nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingMethod,
            "missing method {}", name);
    return priv->superEnum->prepareMethod(name, receiverType, callable);
}

bool
lyric_assembler::EnumSymbol::hasStub(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

lyric_assembler::ActionSymbol *
lyric_assembler::EnumSymbol::getStub(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->stubs.find(name);
    if (entry != priv->stubs.cend())
        return entry->second;
    return nullptr;
}

absl::flat_hash_map<std::string,lyric_assembler::ActionSymbol *>::const_iterator
lyric_assembler::EnumSymbol::stubsBegin() const
{
    auto *priv = getPriv();
    return priv->stubs.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::ActionSymbol *>::const_iterator
lyric_assembler::EnumSymbol::stubsEnd() const
{
    auto *priv = getPriv();
    return priv->stubs.cend();
}

tu_uint32
lyric_assembler::EnumSymbol::numStubs() const
{
    auto *priv = getPriv();
    return static_cast<tu_uint32>(priv->stubs.size());
}

tempo_utils::Result<lyric_assembler::ActionSymbol *>
lyric_assembler::EnumSymbol::declareStub(const std::string &name, bool isHidden)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare stub on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();

    auto *existingOrOverridden = find_existing_or_overridden_enum_binding(this, name);
    if (existingOrOverridden == this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "{} already defined for enum {}", name, m_enumUrl.toString());
    if (existingOrOverridden != nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "{} cannot be overridden", existingOrOverridden->getSymbolUrl().toString());

    // build reference path to stub
    auto stubPath = m_enumUrl.getSymbolPath().getPath();
    stubPath.push_back(name);
    auto stubUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(stubPath));

    // construct call symbol
    auto actionSymbol = std::make_unique<ActionSymbol>(stubUrl, m_enumUrl, isHidden,
            priv->isDeclOnly, priv->enumBlock.get(), m_state);

    ActionSymbol *actionPtr;
    TU_ASSIGN_OR_RETURN (actionPtr, m_state->appendAction(std::move(actionSymbol)));
    TU_RETURN_IF_NOT_OK (priv->enumBlock->putBinding(actionPtr));
    priv->stubs[name] = actionPtr;

    priv->isAbstract =  true;

    return actionPtr;
}

bool
lyric_assembler::EnumSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    auto *priv = getPriv();
    return priv->impls.contains(implType);
}

lyric_assembler::ImplHandle *
lyric_assembler::EnumSymbol::getImpl(const lyric_common::TypeDef &implType) const
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
lyric_assembler::EnumSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::ImplHandle *>::const_iterator
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

    if (priv->impls.contains(implType))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "impl {} already defined for enum {}", implType.toString(), m_enumUrl.toString());

    auto *implCache = m_state->implCache();

    ImplHandle *implHandle;
    TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(implType, m_enumUrl,
        priv->isDeclOnly, priv->enumBlock.get()));

    priv->impls[implType] = implHandle;

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