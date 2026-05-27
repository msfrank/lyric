
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/ctor_constructable.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/method_callable.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/stub_callable.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_importer/struct_import.h>

lyric_assembler::StructSymbol::StructSymbol(
    const lyric_common::SymbolUrl &structUrl,
    bool isHidden,
    bool isAbstract,
    lyric_object::DeriveType derive,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(new StructSymbolPriv()),
      m_structUrl(structUrl),
      m_state(state)
{
    TU_ASSERT (m_structUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->isHidden = isHidden;
    priv->isAbstract = isAbstract;
    priv->derive = derive;
    priv->isDeclOnly = isDeclOnly;
    priv->structBlock = std::make_unique<BlockHandle>(structUrl, parentBlock);
}

lyric_assembler::StructSymbol::StructSymbol(
    const lyric_common::SymbolUrl &structUrl,
    std::shared_ptr<lyric_importer::StructImport> structImport,
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_structUrl(structUrl),
      m_structImport(std::move(structImport)),
      m_state(state)
{
    TU_ASSERT (m_structUrl.isValid());
    TU_ASSERT (m_structImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::StructSymbolPriv *
lyric_assembler::StructSymbol::load()
{
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<StructSymbolPriv>();

    priv->structBlock = std::make_unique<BlockHandle>(
        m_structUrl, absl::flat_hash_map<std::string, SymbolBinding>(), m_state);

    priv->isHidden = m_structImport->isHidden();
    priv->isAbstract = m_structImport->isAbstract();
    priv->derive = m_structImport->getDerive();
    priv->isDeclOnly = m_structImport->isDeclOnly();

    auto typeImport = m_structImport->getStructType().lock();
    if (typeImport == nullptr)
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kImportError,
            "cannot import struct {}; missing type",
            m_structUrl.toString()));
    TU_ASSIGN_OR_RAISE (priv->structType, typeCache->importType(typeImport));

    auto superStructUrl = m_structImport->getSuperStruct();
    if (superStructUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superStruct, importCache->importStruct(superStructUrl));

        auto superTypeImport = m_structImport->getSuperType().lock();
        if (superTypeImport == nullptr)
            throw tempo_utils::StatusException(
                AssemblerStatus::forCondition(AssemblerCondition::kImportError,
                "cannot import struct {}; missing supertype",
                m_structUrl.toString()));
        TU_ASSIGN_OR_RAISE (priv->superType, typeCache->importType(superTypeImport));
    }

    for (auto it = m_structImport->membersBegin(); it != m_structImport->membersEnd(); it++) {
        FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RAISE (fieldSymbol, importCache->importField(it->second));
        TU_RAISE_IF_NOT_OK (priv->structBlock->putBinding(fieldSymbol));
        priv->members[it->first] = fieldSymbol;
    }

    for (auto it = m_structImport->methodsBegin(); it != m_structImport->methodsEnd(); it++) {
        CallSymbol *callSymbol;
        TU_ASSIGN_OR_RAISE (callSymbol, importCache->importCall(it->second));
        TU_RAISE_IF_NOT_OK (priv->structBlock->putBinding(callSymbol));
        priv->methods[it->first] = callSymbol;
    }

    for (auto it = m_structImport->stubsBegin(); it != m_structImport->stubsEnd(); it++) {
        ActionSymbol *actionSymbol;
        TU_ASSIGN_OR_RAISE (actionSymbol, importCache->importAction(it->second));
        TU_RAISE_IF_NOT_OK (priv->structBlock->putBinding(actionSymbol));
        priv->stubs[it->first] = actionSymbol;
    }

    if (!priv->stubs.empty() && !priv->isAbstract)
        throw tempo_utils::StatusException(AssemblerStatus::forCondition(
            AssemblerCondition::kImportError,
            "cannot import struct {}; struct has stubs but is not declared abstract",
            m_structUrl.toString()));

    auto *implCache = m_state->implCache();
    for (auto it = m_structImport->implsBegin(); it != m_structImport->implsEnd(); it++) {
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
                    consumerType.toString(), m_structUrl.toString()));
        priv->impls[consumerType] = implHandle;

        auto implementationType = contract.getImplementationType();
        if (implementationType == consumerType)
            continue;
        if (priv->impls.contains(implementationType))
            throw tempo_utils::StatusException(AssemblerStatus::forCondition(
                AssemblerCondition::kImportError, "impl {} is already imported for {}",
                    implementationType.toString(), m_structUrl.toString()));
        priv->impls[implementationType] = implHandle;
    }

    for (auto iterator = m_structImport->sealedTypesBegin(); iterator != m_structImport->sealedTypesEnd(); iterator++) {
        priv->sealedTypes.insert(*iterator);
    }

    priv->allocatorTrap = m_structImport->getAllocator();

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::StructSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Struct;
}

lyric_assembler::SymbolType
lyric_assembler::StructSymbol::getSymbolType() const
{
    return SymbolType::STRUCT;
}

lyric_common::SymbolUrl
lyric_assembler::StructSymbol::getSymbolUrl() const
{
    return m_structUrl;
}

lyric_common::TypeDef
lyric_assembler::StructSymbol::getTypeDef() const
{
    auto *priv = getPriv();
    return priv->structType->getTypeDef();
}

lyric_assembler::BlockHandle *
lyric_assembler::StructSymbol::derefBlock()
{
    auto *priv = getPriv();
    return priv->structBlock.get();
}

bool
lyric_assembler::StructSymbol::isHidden() const
{
    auto *priv = getPriv();
    return priv->isHidden;
}

bool
lyric_assembler::StructSymbol::isAbstract() const
{
    auto *priv = getPriv();
    return priv->isAbstract;
}

lyric_object::DeriveType
lyric_assembler::StructSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
}

bool
lyric_assembler::StructSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

lyric_assembler::StructSymbol *
lyric_assembler::StructSymbol::superStruct() const
{
    auto *priv = getPriv();
    return priv->superStruct;
}

lyric_assembler::TypeHandle *
lyric_assembler::StructSymbol::superType() const
{
    auto *priv = getPriv();
    return priv->superStruct->structType();
}

lyric_assembler::TypeHandle *
lyric_assembler::StructSymbol::structType() const
{
    auto *priv = getPriv();
    return priv->structType;
}

lyric_assembler::BlockHandle *
lyric_assembler::StructSymbol::structBlock() const
{
    auto *priv = getPriv();
    return priv->structBlock.get();
}

lyric_assembler::AbstractResolver *
lyric_assembler::StructSymbol::structResolver() const
{
    auto *priv = getPriv();
    return priv->structBlock.get();
}

tempo_utils::Status
lyric_assembler::StructSymbol::finalizeStruct(const lyric_common::TypeDef &superStructType)
{
    auto *typeCache = m_state->typeCache();

    auto *priv = getPriv();

    if (priv->superType != nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "{} is already finalized", m_structUrl.toString());

    StructSymbol *superStruct;
    TU_ASSIGN_OR_RETURN (superStruct, priv->structBlock->resolveStruct(superStructType));

    auto superDerive = superStruct->getDeriveType();
    if (superDerive == lyric_object::DeriveType::Final)
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
            "cannot derive struct {} from {}; base struct is marked final",
            m_structUrl.getSymbolPath().toString(), superStruct->getSymbolUrl().toString());
    if (superDerive == lyric_object::DeriveType::Sealed && superStruct->isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
            "cannot derive struct {} from {}; sealed base struct must be located in the same module",
            m_structUrl.getSymbolPath().toString(), superStruct->getSymbolUrl().toString());

    // create the supertype if necessary
    auto *supertypeHandle = superStruct->structType();

    // create the type
    TypeHandle *typeHandle;
    TU_ASSIGN_OR_RETURN (typeHandle, typeCache->declareSubType(m_structUrl, {}, superStructType));

    priv->superStruct = superStruct;
    priv->superType = supertypeHandle;
    priv->structType = typeHandle;
    return {};
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::StructSymbol::resolveGlobalMember(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *symbolCache = m_state->symbolCache();
    auto *priv = getPriv();

    auto globalSymbolUrl = priv->structBlock->makeSymbolUrl(name);

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(globalSymbolUrl, /* allowMissing= */ true));
    if (symbol == nullptr) {
        if (priv->superStruct == nullptr)
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
                "missing global member {}", name);
        return priv->superStruct->resolveGlobalMember(name, receiverType, thisReceiver);
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

    bool thisSymbol = receiverType.getConcreteUrl() == m_structUrl;
    if (isHidden && !(thisReceiver && thisSymbol))
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
            "access to hidden member {} is not allowed", name);
    ref.symbolUrl = globalSymbolUrl;
    ref.typeDef = symbol->getTypeDef();
    return ref;
}

tempo_utils::Status
lyric_assembler::StructSymbol::prepareGlobalMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    std::unique_ptr<AbstractCallable> &callable,
    bool thisOrInheritedReceiver) const
{
    auto *symbolCache = m_state->symbolCache();
    auto *priv = getPriv();

    auto globalSymbolUrl = priv->structBlock->makeSymbolUrl(name);

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(globalSymbolUrl, /* allowMissing= */ true));
    if (symbol == nullptr) {
        if (priv->superStruct == nullptr)
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMethod,
                "missing global method {}", name);
        return priv->superStruct->prepareGlobalMethod(name, receiverType, callable, thisOrInheritedReceiver);
    }

    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", symbol->getSymbolUrl().toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    if (callSymbol->isHidden()) {
        if (!thisOrInheritedReceiver)
            return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                "cannot access hidden method {} on {}", name, m_structUrl.toString());
    }

    if (callSymbol->isBound())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", callSymbol->getSymbolUrl().toString());

    callable = std::make_unique<FunctionCallable>(callSymbol, callSymbol->isInline());
    return {};
}

bool
lyric_assembler::StructSymbol::hasMember(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->members.contains(name);
}

lyric_assembler::FieldSymbol *
lyric_assembler::StructSymbol::getMember(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->members.find(name);
    if (entry != priv->members.cend())
        return entry->second;
    return nullptr;
}

absl::flat_hash_map<std::string,lyric_assembler::FieldSymbol *>::const_iterator
lyric_assembler::StructSymbol::membersBegin() const
{
    auto *priv = getPriv();
    return priv->members.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::FieldSymbol *>::const_iterator
lyric_assembler::StructSymbol::membersEnd() const
{
    auto *priv = getPriv();
    return priv->members.cend();
}

tu_uint32
lyric_assembler::StructSymbol::numMembers() const
{
    auto *priv = getPriv();
    return static_cast<tu_uint32>(priv->members.size());
}

static lyric_assembler::AbstractSymbol *
find_existing_or_overridden_struct_binding(
    lyric_assembler::StructSymbol *structSymbol,
    const std::string &name)
{
    // if struct contains the named binding then return the StructSymbol pointer
    auto *block = structSymbol->structBlock();
    if (block->hasBinding(name))
        return structSymbol;

    // otherwise if a superstruct contains the named binding then return pointer to the binding symbol
    for (auto *currStruct = structSymbol->superStruct(); currStruct != nullptr; currStruct = currStruct->superStruct()) {
        block = currStruct->structBlock();
        if (!block->hasBinding(name))
            continue;

        auto binding = block->getBinding(name);
        if (binding.bindingType == lyric_assembler::BindingType::Descriptor) {
            auto *state = block->blockState();
            auto *symbolCache = state->symbolCache();
            return symbolCache->getSymbolOrNull(binding.symbolUrl);
        }
    }

    // otherwise no binding exists in any superstruct
    return nullptr;
}

tempo_utils::Result<lyric_assembler::FieldSymbol *>
lyric_assembler::StructSymbol::declareMember(
    const std::string &name,
    const lyric_common::TypeDef &memberType,
    bool isHidden)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare member on imported struct {}", m_structUrl.toString());

    auto *priv = getPriv();

    auto *existingOrOverridden = find_existing_or_overridden_struct_binding(this, name);
    if (existingOrOverridden == this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "'{}' is already defined for struct {}", name, m_structUrl.toString());
    if (existingOrOverridden != nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "{} cannot be overridden by struct {}",
            existingOrOverridden->getSymbolUrl().toString(), m_structUrl.toString());

    TypeHandle *fieldType;
    TU_ASSIGN_OR_RETURN (fieldType, m_state->typeCache()->getOrMakeType(memberType));

    auto memberPath = m_structUrl.getSymbolPath().getPath();
    memberPath.push_back(name);
    auto memberUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(memberPath));

    // construct the field symbol
    auto fieldSymbol = std::make_unique<FieldSymbol>(memberUrl, isHidden, /* isVariable= */ false,
        fieldType, priv->isDeclOnly, priv->structBlock.get(), m_state);

    FieldSymbol *fieldPtr;
    TU_ASSIGN_OR_RETURN (fieldPtr, m_state->appendField(std::move(fieldSymbol)));
    TU_RAISE_IF_NOT_OK (priv->structBlock->putBinding(fieldPtr));
    priv->members[name] = fieldPtr;

    return fieldPtr;
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::StructSymbol::resolveMember(
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
            bool thisSymbol = receiverType.getConcreteUrl() == m_structUrl;
            if (!(thisReceiver && thisSymbol))
                return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                    "access to hidden member {} is not allowed", name);
        }

        return reifier.reifyMember(name, fieldSymbol);
    }

    if (priv->superStruct == nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
            "missing member {}", name);
    return priv->superStruct->resolveMember(name, reifier, receiverType, thisReceiver);
}

bool
lyric_assembler::StructSymbol::isMemberInitialized(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->initializedMembers.contains(name);
}

tempo_utils::Status
lyric_assembler::StructSymbol::setMemberInitialized(const std::string &name)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't set member initialized on imported struct {}", m_structUrl.toString());

    auto *priv = getPriv();
    if (isMemberInitialized(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidBinding,
            "member {} is already initialized", name);
    priv->initializedMembers.insert(name);
    return {};
}

bool
lyric_assembler::StructSymbol::isCompletelyInitialized() const
{
    auto *priv = getPriv();
    for (const auto &member : priv->members) {
        if (!priv->initializedMembers.contains(member.first))
            return false;
    }
    return true;
}

std::string
lyric_assembler::StructSymbol::getAllocatorTrap() const
{
    auto *priv = getPriv();
    return priv->allocatorTrap;
}

bool
lyric_assembler::StructSymbol::hasCtor(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->methods.find(name);
    if (entry == priv->methods.cend())
        return false;
    auto *callSymbol = entry->second;
    return callSymbol->isCtor();
}

lyric_assembler::CallSymbol *
lyric_assembler::StructSymbol::getCtor(const std::string &name) const
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
lyric_assembler::StructSymbol::declareCtor(
    const std::string &name,
    bool isHidden,
    std::string allocatorTrap)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare constructor on imported struct {}", m_structUrl.toString());

    auto *priv = getPriv();

    auto path = m_structUrl.getSymbolPath().getPath();
    path.emplace_back(name);
    auto ctorUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(path));

    auto *existingSymbol = m_state->symbolCache()->getSymbolOrNull(ctorUrl);
    if (existingSymbol != nullptr) {
        if (existingSymbol->getSymbolType() == SymbolType::CALL) {
            auto existingCall = cast_symbol_to_call(existingSymbol);
            if (existingCall->isCtor()) {
                auto friendlyName = name == lyric_object::kCtorSpecialSymbol?
                    "default" : absl::StrCat("'", name, "'");
                return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
                    "{} constructor already defined for struct {}",
                    friendlyName, m_structUrl.toString());
            }
        }
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "'{}' already defined for struct {}", name, m_structUrl.toString());
    }

    // construct call symbol
    auto ctorSymbol = std::make_unique<CallSymbol>(ctorUrl, m_structUrl, isHidden,
        lyric_object::CallMode::Constructor, /* isFinal= */ false, priv->isDeclOnly,
        priv->structBlock.get(), m_state);

    CallSymbol *ctorPtr;
    TU_ASSIGN_OR_RETURN (ctorPtr, m_state->appendCall(std::move(ctorSymbol)));
    TU_RAISE_IF_NOT_OK (priv->structBlock->putBinding(ctorPtr));
    priv->methods[name] = ctorPtr;

    // set allocator trap
    priv->allocatorTrap = std::move(allocatorTrap);

    return ctorPtr;
}

tempo_utils::Status
lyric_assembler::StructSymbol::prepareCtor(const std::string &name, std::unique_ptr<AbstractCallable> &callable)
{
    lyric_common::SymbolPath ctorPath = lyric_common::SymbolPath(
        m_structUrl.getSymbolPath().getPath(), name);
    auto ctorUrl = lyric_common::SymbolUrl(m_structUrl.getModuleLocation(), ctorPath);

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(ctorUrl));
    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", ctorUrl.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    callable = std::make_unique<CtorConstructable>(callSymbol, this);
    return {};
}

bool
lyric_assembler::StructSymbol::hasMethod(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

lyric_assembler::CallSymbol *
lyric_assembler::StructSymbol::getMethod(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->methods.find(name);
    if (entry != priv->methods.cend())
        return entry->second;
    return nullptr;
}

absl::flat_hash_map<std::string,lyric_assembler::CallSymbol *>::const_iterator
lyric_assembler::StructSymbol::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::CallSymbol *>::const_iterator
lyric_assembler::StructSymbol::methodsEnd() const
{
    auto *priv = getPriv();
    return priv->methods.cend();
}

tu_uint32
lyric_assembler::StructSymbol::numMethods() const
{
    auto *priv = getPriv();
    return static_cast<tu_uint32>(priv->methods.size());
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::StructSymbol::declareMethod(
    const std::string &name,
    bool isHidden,
    bool isFinal)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare method on imported struct {}", m_structUrl.toString());

    auto *priv = getPriv();

    auto *existingOrOverridden = find_existing_or_overridden_struct_binding(this, name);
    if (existingOrOverridden == this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "{} already defined for struct {}", name, m_structUrl.toString());

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
                        "final method {} cannot be overridden by struct {}",
                        existingOrOverridden->getSymbolUrl().toString(), m_structUrl.toString());
                baseUrl = existingOrOverridden->getSymbolUrl();
                break;
            }
            default:
                return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
                    "{} cannot be overridden by struct {}",
                    existingOrOverridden->getSymbolUrl().toString(), m_structUrl.toString());
        }
    }

    // build reference path to function
    auto methodPath = m_structUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));

    // construct call symbol
    std::unique_ptr<CallSymbol> callSymbol;
    if (baseUrl.isValid()) {
        callSymbol = std::make_unique<CallSymbol>(methodUrl, m_structUrl, isHidden, baseUrl, isFinal,
            priv->isDeclOnly, priv->structBlock.get(), m_state);
    } else {
        callSymbol = std::make_unique<CallSymbol>(methodUrl, m_structUrl, isHidden,
            lyric_object::CallMode::Normal, isFinal, priv->isDeclOnly, priv->structBlock.get(),
            m_state);
    }

    CallSymbol *callPtr;
    TU_ASSIGN_OR_RETURN (callPtr, m_state->appendCall(std::move(callSymbol)));
    TU_RAISE_IF_NOT_OK (priv->structBlock->putBinding(callPtr));
    priv->methods[name] = callPtr;

    return callPtr;
}

tempo_utils::Status
lyric_assembler::StructSymbol::prepareMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    std::unique_ptr<AbstractCallable> &callable,
    bool thisOrInheritedReceiver) const
{
    auto *priv = getPriv();

    auto method = priv->methods.find(name);
    if (method != priv->methods.cend()) {
        auto *callSymbol = method->second;

        if (callSymbol->isHidden()) {
            if (!thisOrInheritedReceiver)
                return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                    "cannot access hidden method {} on {}", name, m_structUrl.toString());
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
            if (!thisOrInheritedReceiver)
                return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                    "cannot access hidden method {} on {}", name, m_structUrl.toString());
        }

        callable = std::make_unique<StubCallable>(actionSymbol);
        return {};
    }

    if (priv->superStruct == nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingMethod,
            "missing method {}", name);
    return priv->superStruct->prepareMethod(name, receiverType, callable, thisOrInheritedReceiver);
}

bool
lyric_assembler::StructSymbol::hasStub(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

lyric_assembler::ActionSymbol *
lyric_assembler::StructSymbol::getStub(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->stubs.find(name);
    if (entry != priv->stubs.cend())
        return entry->second;
    return nullptr;
}

absl::flat_hash_map<std::string,lyric_assembler::ActionSymbol *>::const_iterator
lyric_assembler::StructSymbol::stubsBegin() const
{
    auto *priv = getPriv();
    return priv->stubs.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::ActionSymbol *>::const_iterator
lyric_assembler::StructSymbol::stubsEnd() const
{
    auto *priv = getPriv();
    return priv->stubs.cend();
}

tu_uint32
lyric_assembler::StructSymbol::numStubs() const
{
    auto *priv = getPriv();
    return static_cast<tu_uint32>(priv->stubs.size());
}

tempo_utils::Result<lyric_assembler::ActionSymbol *>
lyric_assembler::StructSymbol::declareStub(const std::string &name, bool isHidden)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare stub on imported struct {}", m_structUrl.toString());

    auto *priv = getPriv();

    auto *existingOrOverridden = find_existing_or_overridden_struct_binding(this, name);
    if (existingOrOverridden == this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "{} already defined for struct {}", name, m_structUrl.toString());
    if (existingOrOverridden != nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "{} cannot be overridden", existingOrOverridden->getSymbolUrl().toString());

    // build reference path to stub
    auto stubPath = m_structUrl.getSymbolPath().getPath();
    stubPath.push_back(name);
    auto stubUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(stubPath));

    // construct call symbol
    auto actionSymbol = std::make_unique<ActionSymbol>(stubUrl, m_structUrl, isHidden,
        priv->isDeclOnly, priv->structBlock.get(), m_state);

    ActionSymbol *actionPtr;
    TU_ASSIGN_OR_RETURN (actionPtr, m_state->appendAction(std::move(actionSymbol)));
    TU_RETURN_IF_NOT_OK (priv->structBlock->putBinding(actionPtr));
    priv->stubs[name] = actionPtr;

    priv->isAbstract =  true;

    return actionPtr;
}

bool
lyric_assembler::StructSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    auto *priv = getPriv();
    return priv->impls.contains(implType);
}

lyric_assembler::ImplHandle *
lyric_assembler::StructSymbol::getImpl(const lyric_common::TypeDef &implType) const
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
lyric_assembler::StructSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::StructSymbol::implsEnd() const
{
    auto *priv = getPriv();
    return priv->impls.cend();
}

tu_uint32
lyric_assembler::StructSymbol::numImpls() const
{
    auto *priv = getPriv();
    return priv->impls.size();
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::StructSymbol::declareImpl(const lyric_common::TypeDef &implType)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare impl on imported struct {}", m_structUrl.toString());

    auto *priv = getPriv();

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid impl type {}", implType.toString());

    if (priv->impls.contains(implType))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "impl {} already defined for struct {}", implType.toString(), m_structUrl.toString());

    auto *implCache = m_state->implCache();

    ImplHandle *implHandle;
    TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(implType, m_structUrl,
        priv->isDeclOnly, priv->structBlock.get()));

    priv->impls[implType] = implHandle;

    return implHandle;
}

bool
lyric_assembler::StructSymbol::hasSealedType(const lyric_common::TypeDef &sealedType) const
{
    auto *priv = getPriv();
    return priv->sealedTypes.contains(sealedType);
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::StructSymbol::sealedTypesBegin() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::StructSymbol::sealedTypesEnd() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cend();
}

tempo_utils::Status
lyric_assembler::StructSymbol::putSealedType(const lyric_common::TypeDef &sealedType)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't put sealed type on imported struct {}", m_structUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "struct {} is not sealed", m_structUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "invalid derived type {} for sealed struct {}", sealedType.toString(), m_structUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(sealedUrl));
    if (symbol->getSymbolType() != SymbolType::STRUCT)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid struct symbol {}", sealedUrl.toString());

    if (cast_symbol_to_struct(symbol)->superStruct() != this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "{} does not derive from sealed struct {}", sealedType.toString(), m_structUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return {};
}
