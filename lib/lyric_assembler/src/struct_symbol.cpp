
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/strings/match.h>

#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/ctor_constructable.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/method_callable.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/struct_import.h>

lyric_assembler::StructSymbol::StructSymbol(
    const lyric_common::SymbolUrl &structUrl,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    bool isAbstract,
    StructAddress address,
    TypeHandle *structType,
    StructSymbol *superStruct,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : BaseSymbol(address, new StructSymbolPriv()),
      m_structUrl(structUrl),
      m_state(state)
{
    TU_ASSERT (m_structUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = access;
    priv->derive = derive;
    priv->isAbstract = isAbstract;
    priv->structType = structType;
    priv->superStruct = superStruct;
    priv->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    priv->structBlock = std::make_unique<BlockHandle>(structUrl, parentBlock, false);

    TU_ASSERT (priv->structType != nullptr);
    TU_ASSERT (priv->superStruct != nullptr);
}

lyric_assembler::StructSymbol::StructSymbol(
    const lyric_common::SymbolUrl &structUrl,
    lyric_importer::StructImport *structImport,
    AssemblyState *state)
    : m_structUrl(structUrl),
      m_structImport(structImport),
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
    priv->access = lyric_object::AccessType::Public;
    priv->derive = m_structImport->getDerive();
    priv->isAbstract = m_structImport->isAbstract();

    auto *structType = m_structImport->getStructType();
    TU_ASSIGN_OR_RAISE (priv->structType, typeCache->importType(structType));

    auto superStructUrl = m_structImport->getSuperStruct();
    if (superStructUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superStruct, importCache->importStruct(superStructUrl));
    }

    for (auto iterator = m_structImport->membersBegin(); iterator != m_structImport->membersEnd(); iterator++) {
        FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RAISE (fieldSymbol, importCache->importField(iterator->second));

        DataReference memberRef;
        memberRef.symbolUrl = iterator->second;
        memberRef.typeDef = fieldSymbol->getAssignableType();
        memberRef.referenceType = fieldSymbol->isVariable()? ReferenceType::Variable : ReferenceType::Value;
        priv->members[iterator->first] = memberRef;
    }

    for (auto iterator = m_structImport->methodsBegin(); iterator != m_structImport->methodsEnd(); iterator++) {
        CallSymbol *callSymbol;
        TU_ASSIGN_OR_RAISE (callSymbol, importCache->importCall(iterator->second));

        BoundMethod methodBinding;
        methodBinding.methodCall = iterator->second;
        methodBinding.access = callSymbol->getAccessType();
        methodBinding.final = false;    // FIXME: this should come from the call symbol
        priv->methods[iterator->first] = methodBinding;
    }

    auto *implCache = m_state->implCache();
    for (auto iterator = m_structImport->implsBegin(); iterator != m_structImport->implsEnd(); iterator++) {
        ImplHandle *implHandle;
        TU_ASSIGN_OR_RAISE (implHandle, implCache->importImpl(iterator->second));
        auto implUrl = iterator->first.getConcreteUrl();
        priv->impls[implUrl] = implHandle;
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
lyric_assembler::StructSymbol::getAssignableType() const
{
    auto *priv = getPriv();
    return priv->structType->getTypeDef();
}

lyric_assembler::TypeSignature
lyric_assembler::StructSymbol::getTypeSignature() const
{
    auto *priv = getPriv();
    return priv->structType->getTypeSignature();
}

void
lyric_assembler::StructSymbol::touch()
{
    if (getAddress().isValid())
        return;
    m_state->touchStruct(this);
}

lyric_object::AccessType
lyric_assembler::StructSymbol::getAccessType() const
{
    auto *priv = getPriv();
    return priv->access;
}

lyric_object::DeriveType
lyric_assembler::StructSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
}

bool lyric_assembler::StructSymbol::isAbstract() const
{
    auto *priv = getPriv();
    return priv->isAbstract;
}

lyric_assembler::StructSymbol *
lyric_assembler::StructSymbol::superStruct() const
{
    auto *priv = getPriv();
    return priv->superStruct;
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

bool
lyric_assembler::StructSymbol::hasMember(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->members.contains(name);
}

Option<lyric_assembler::DataReference>
lyric_assembler::StructSymbol::getMember(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->members.contains(name))
        return Option<DataReference>(priv->members.at(name));
    return Option<DataReference>();
}

absl::flat_hash_map<std::string,lyric_assembler::DataReference>::const_iterator
lyric_assembler::StructSymbol::membersBegin() const
{
    auto *priv = getPriv();
    return priv->members.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::DataReference>::const_iterator
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

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::StructSymbol::declareMember(
    const std::string &name,
    const lyric_common::TypeDef &memberType,
    const lyric_common::SymbolUrl &init)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare member on imported struct {}", m_structUrl.toString());

    auto *priv = getPriv();

    if (absl::StartsWith(name, "__"))
        return m_state->logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "declaration of private member {} is not allowed", name);
    if (absl::StartsWith(name, "_"))
        return m_state->logAndContinue(AssemblerCondition::kInvalidAccess,
            tempo_tracing::LogSeverity::kError,
            "declaration of protected member {} is not allowed", name);

    if (priv->members.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "member {} already defined for struct {}", name, m_structUrl.toString());

    lyric_assembler::TypeHandle *fieldType;
    TU_ASSIGN_OR_RETURN (fieldType, m_state->typeCache()->getOrMakeType(memberType));

    auto memberPath = m_structUrl.getSymbolPath().getPath();
    memberPath.push_back(name);
    auto memberUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(memberPath));
    auto fieldIndex = m_state->numFields();
    auto address = FieldAddress::near(fieldIndex);

    // construct the field symbol
    FieldSymbol *fieldSymbol;
    if (init.isValid()) {
        fieldSymbol = new FieldSymbol(memberUrl, lyric_object::AccessType::Public,
            false, init, address, fieldType, m_state);
    } else {
        fieldSymbol = new FieldSymbol(memberUrl, lyric_object::AccessType::Public,
            false, address, fieldType, m_state);
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
    ref.referenceType = ReferenceType::Value;
    priv->members[name] = ref;

    return ref;
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::StructSymbol::resolveMember(
    const std::string &name,
    AbstractMemberReifier &reifier,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    if (!priv->members.contains(name)) {
        if (priv->superStruct == nullptr)
            return m_state->logAndContinue(AssemblerCondition::kMissingMember,
                tempo_tracing::LogSeverity::kError,
                "missing member {}", name);
        return priv->superStruct->resolveMember(name, reifier, receiverType, thisReceiver);
    }

    const auto &member = priv->members.at(name);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(member.symbolUrl));
    if (symbol->getSymbolType() != SymbolType::FIELD)
        m_state->throwAssemblerInvariant("invalid field symbol {}", member.symbolUrl.toString());
    auto *fieldSymbol = cast_symbol_to_field(symbol);
    auto access = fieldSymbol->getAccessType();

    bool thisSymbol = receiverType.getConcreteUrl() == m_structUrl;

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
lyric_assembler::StructSymbol::isMemberInitialized(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->initializedMembers.contains(name);
}

tempo_utils::Status
lyric_assembler::StructSymbol::setMemberInitialized(const std::string &name)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't set member initialized on imported struct {}", m_structUrl.toString());

    auto *priv = getPriv();
    if (isMemberInitialized(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidBinding,
            "member {} is already initialized", name);
    priv->initializedMembers.insert(name);
    return AssemblerStatus::ok();
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

lyric_common::SymbolUrl
lyric_assembler::StructSymbol::getCtor() const
{
    auto location = m_structUrl.getAssemblyLocation();
    auto path = m_structUrl.getSymbolPath();
    return lyric_common::SymbolUrl(location, lyric_common::SymbolPath(path.getPath(), "$ctor"));
}

tu_uint32
lyric_assembler::StructSymbol::getAllocatorTrap() const
{
    auto *priv = getPriv();
    return priv->allocatorTrap;
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::StructSymbol::declareCtor(
    lyric_object::AccessType access,
    tu_uint32 allocatorTrap)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare ctor on imported struct {}", m_structUrl.toString());

    auto *priv = getPriv();

    auto path = m_structUrl.getSymbolPath().getPath();
    path.push_back("$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(path));

    if (m_state->symbolCache()->hasSymbol(ctorUrl))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "ctor already defined for struct {}", m_structUrl.toString());

    auto fundamentalStruct = m_state->fundamentalCache()->getFundamentalUrl(FundamentalSymbol::Struct);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(fundamentalStruct));
    symbol->touch();

    auto callIndex = m_state->numCalls();
    auto address = CallAddress::near(callIndex);

    // construct call symbol
    auto *ctorSymbol = new CallSymbol(ctorUrl, m_structUrl, access, address,
        lyric_object::CallMode::Constructor, priv->structBlock.get(), m_state);

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

    return ctorSymbol;
}

tempo_utils::Status
lyric_assembler::StructSymbol::prepareCtor(ConstructableInvoker &invoker)
{
    lyric_common::SymbolPath ctorPath = lyric_common::SymbolPath(m_structUrl.getSymbolPath().getPath(), "$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(m_structUrl.getAssemblyLocation(), ctorPath);

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(ctorUrl));
    if (symbol->getSymbolType() != SymbolType::CALL)
        m_state->throwAssemblerInvariant("invalid call symbol {}", ctorUrl.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);
    callSymbol->touch();

    auto constructable = std::make_unique<CtorConstructable>(callSymbol, this);
    return invoker.initialize(std::move(constructable));
}

bool
lyric_assembler::StructSymbol::hasMethod(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

Option<lyric_assembler::BoundMethod>
lyric_assembler::StructSymbol::getMethod(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->methods.contains(name))
        return Option<BoundMethod>(priv->methods.at(name));
    return Option<BoundMethod>();
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
lyric_assembler::StructSymbol::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
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
    lyric_object::AccessType acccess)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare method on imported struct {}", m_structUrl.toString());

    auto *priv = getPriv();

    if (priv->methods.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "method {} already defined for struct {}", name, m_structUrl.toString());

    // build reference path to function
    auto methodPath = m_structUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));
    auto callIndex = m_state->numCalls();
    auto address = CallAddress::near(callIndex);

    // construct call symbol
    auto *callSymbol = new CallSymbol(methodUrl, m_structUrl, acccess, address,
        lyric_object::CallMode::Normal, priv->structBlock.get(), m_state);

    auto status = m_state->appendCall(callSymbol);
    if (status.notOk()) {
        delete callSymbol;
        return status;
    }

    // add bound method
    priv->methods[name] = { methodUrl, lyric_object::AccessType::Public, false /* final */ };

    return callSymbol;
}

tempo_utils::Status
lyric_assembler::StructSymbol::prepareMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    CallableInvoker &invoker,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    if (!priv->methods.contains(name)) {
        if (priv->superStruct == nullptr)
            return m_state->logAndContinue(AssemblerCondition::kMissingMethod,
                tempo_tracing::LogSeverity::kError,
                "missing method {}", name);
        return priv->superStruct->prepareMethod(name, receiverType, invoker, thisReceiver);
    }

    const auto &method = priv->methods.at(name);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(method.methodCall));
    if (symbol->getSymbolType() != SymbolType::CALL)
        m_state->throwAssemblerInvariant("invalid call symbol {}", method.methodCall.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);
    callSymbol->touch();

    if (callSymbol->isInline()) {
        auto callable = std::make_unique<MethodCallable>(callSymbol, callSymbol->callProc());
        return invoker.initialize(std::move(callable));
    }

    if (!callSymbol->isBound())
        m_state->throwAssemblerInvariant("invalid call symbol {}", callSymbol->getSymbolUrl().toString());

    auto callable = std::make_unique<MethodCallable>(callSymbol);
    return invoker.initialize(std::move(callable));
}

bool
lyric_assembler::StructSymbol::hasImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    return priv->impls.contains(implUrl);
}

bool
lyric_assembler::StructSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    return hasImpl(implType.getConcreteUrl());
}

lyric_assembler::ImplHandle *
lyric_assembler::StructSymbol::getImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    auto iterator = priv->impls.find(implUrl);
    if (iterator != priv->impls.cend())
        return iterator->second;
    return nullptr;
}

lyric_assembler::ImplHandle *
lyric_assembler::StructSymbol::getImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return nullptr;
    return getImpl(implType.getConcreteUrl());
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::StructSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
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
        m_state->throwAssemblerInvariant(
            "can't declare impl on imported struct {}", m_structUrl.toString());

    auto *priv = getPriv();

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        m_state->throwAssemblerInvariant("invalid impl type {}", implType.toString());
    auto implUrl = implType.getConcreteUrl();

    if (priv->impls.contains(implUrl))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "impl {} already defined for struct {}", implType.toString(), m_structUrl.toString());

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
        name, implTypeHandle, conceptSymbol, m_structUrl, priv->structBlock.get()));

    priv->impls[implUrl] = implHandle;

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
        m_state->throwAssemblerInvariant(
            "can't put sealed type on imported struct {}", m_structUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "struct {} is not sealed", m_structUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "invalid derived type {} for sealed struct {}", sealedType.toString(), m_structUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(sealedUrl));
    if (symbol->getSymbolType() != SymbolType::STRUCT)
        m_state->throwAssemblerInvariant("invalid struct symbol {}", sealedUrl.toString());

    if (cast_symbol_to_struct(symbol)->superStruct() != this)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "{} does not derive from sealed struct {}", sealedType.toString(), m_structUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return {};
}
