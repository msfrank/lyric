
#include <absl/strings/match.h>

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_callable.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <tempo_utils/log_message.h>

lyric_assembler::ExistentialSymbol::ExistentialSymbol(
    const lyric_common::SymbolUrl &existentialUrl,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    ExistentialAddress address,
    TypeHandle *existentialType,
    ExistentialSymbol *superExistential,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(address, new ExistentialSymbolPriv()),
      m_existentialUrl(existentialUrl),
      m_state(state)
{
    TU_ASSERT (m_existentialUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = access;
    priv->derive = derive;
    priv->isDeclOnly = isDeclOnly;
    priv->existentialType = existentialType;
    priv->existentialTemplate = nullptr;
    priv->superExistential = superExistential;
    priv->existentialBlock = std::make_unique<BlockHandle>(existentialUrl, parentBlock, false);

    TU_ASSERT (priv->existentialType != nullptr);
    TU_ASSERT (priv->superExistential != nullptr);
}

lyric_assembler::ExistentialSymbol::ExistentialSymbol(
    const lyric_common::SymbolUrl &existentialUrl,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    ExistentialAddress address,
    TypeHandle *existentialType,
    TemplateHandle *existentialTemplate,
    ExistentialSymbol *superExistential,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : ExistentialSymbol(
        existentialUrl,
        access,
        derive,
        address,
        existentialType,
        superExistential,
        isDeclOnly,
        parentBlock,
        state)
{
    auto *priv = getPriv();
    priv->existentialTemplate = existentialTemplate;
    TU_ASSERT(priv->existentialTemplate != nullptr);
    for (auto it = existentialTemplate->templateParametersBegin(); it != existentialTemplate->templateParametersEnd(); it++) {
        const auto &tp = *it;
        TU_RAISE_IF_STATUS (priv->existentialBlock->declareAlias(tp.name, existentialTemplate->getTemplateUrl(), tp.index));
    }
}

lyric_assembler::ExistentialSymbol::ExistentialSymbol(
    const lyric_common::SymbolUrl &existentialUrl,
    lyric_importer::ExistentialImport *existentialImport,
    ObjectState *state)
    : m_existentialUrl(existentialUrl),
      m_existentialImport(existentialImport),
      m_state(state)
{
    TU_ASSERT (m_existentialUrl.isValid());
    TU_ASSERT (m_existentialImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::ExistentialSymbolPriv *
lyric_assembler::ExistentialSymbol::load()
{
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<ExistentialSymbolPriv>();
    priv->access = lyric_object::AccessType::Public;
    priv->derive = m_existentialImport->getDerive();
    priv->isDeclOnly = m_existentialImport->isDeclOnly();

    auto *existentialType = m_existentialImport->getExistentialType();
    TU_ASSIGN_OR_RAISE (priv->existentialType, typeCache->importType(existentialType));

    auto *existentialTemplate = m_existentialImport->getExistentialTemplate();
    if (existentialTemplate != nullptr) {
        TU_ASSIGN_OR_RAISE (priv->existentialTemplate, typeCache->importTemplate(existentialTemplate));
    }

    auto superExistentialUrl = m_existentialImport->getSuperExistential();
    if (superExistentialUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superExistential, importCache->importExistential(superExistentialUrl));
    }

    for (auto iterator = m_existentialImport->methodsBegin(); iterator != m_existentialImport->methodsEnd(); iterator++) {
        CallSymbol *callSymbol;
        TU_ASSIGN_OR_RAISE (callSymbol, importCache->importCall(iterator->second));

        BoundMethod methodBinding;
        methodBinding.methodCall = iterator->second;
        methodBinding.access = callSymbol->getAccessType();
        methodBinding.final = false;    // FIXME: this should come from the call symbol
        priv->methods[iterator->first] = methodBinding;
    }

    auto *implCache = m_state->implCache();
    for (auto iterator = m_existentialImport->implsBegin(); iterator != m_existentialImport->implsEnd(); iterator++) {
        ImplHandle *implHandle;
        TU_ASSIGN_OR_RAISE (implHandle, implCache->importImpl(iterator->second));
        auto implUrl = iterator->first.getConcreteUrl();
        priv->impls[implUrl] = implHandle;
    }

    for (auto iterator = m_existentialImport->sealedTypesBegin(); iterator != m_existentialImport->sealedTypesEnd(); iterator++) {
        priv->sealedTypes.insert(*iterator);
    }

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::ExistentialSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Existential;
}

lyric_assembler::SymbolType
lyric_assembler::ExistentialSymbol::getSymbolType() const
{
    return SymbolType::EXISTENTIAL;
}

lyric_common::SymbolUrl
lyric_assembler::ExistentialSymbol::getSymbolUrl() const
{
    return m_existentialUrl;
}

lyric_common::TypeDef
lyric_assembler::ExistentialSymbol::getAssignableType() const
{
    auto *priv = getPriv();
    return priv->existentialType->getTypeDef();
}

lyric_assembler::TypeSignature
lyric_assembler::ExistentialSymbol::getTypeSignature() const
{
    auto *priv = getPriv();
    priv->existentialType->touch();
    return priv->existentialType->getTypeSignature();
}

void
lyric_assembler::ExistentialSymbol::touch()
{
    if (getAddress().isValid())
        return;
    m_state->touchExistential(this);
}

bool
lyric_assembler::ExistentialSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

lyric_object::DeriveType
lyric_assembler::ExistentialSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
}

lyric_assembler::ExistentialSymbol *
lyric_assembler::ExistentialSymbol::superExistential() const
{
    auto *priv = getPriv();
    return priv->superExistential;
}

lyric_assembler::TypeHandle *
lyric_assembler::ExistentialSymbol::existentialType() const
{
    auto *priv = getPriv();
    return priv->existentialType;
}

lyric_assembler::TemplateHandle *
lyric_assembler::ExistentialSymbol::existentialTemplate() const
{
    auto *priv = getPriv();
    return priv->existentialTemplate;
}

bool
lyric_assembler::ExistentialSymbol::hasMethod(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

Option<lyric_assembler::BoundMethod>
lyric_assembler::ExistentialSymbol::getMethod(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->methods.contains(name))
        return Option<BoundMethod>(priv->methods.at(name));
    return Option<BoundMethod>();
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
lyric_assembler::ExistentialSymbol::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
lyric_assembler::ExistentialSymbol::methodsEnd() const
{
    auto *priv = getPriv();
    return priv->methods.cend();
}

tu_uint32
lyric_assembler::ExistentialSymbol::numMethods() const
{
    auto *priv = getPriv();
    return static_cast<tu_uint32>(priv->methods.size());
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::ExistentialSymbol::declareMethod(
    const std::string &name,
    lyric_object::AccessType access)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare method on imported existential {}", m_existentialUrl.toString());

    auto *priv = getPriv();

    if (priv->methods.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "method {} already defined for existential {}", name, m_existentialUrl.toString());

    // build reference path to function
    auto methodPath = m_existentialUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));
    auto callIndex = m_state->numCalls();
    auto address = CallAddress::near(callIndex);

    // construct call symbol
    auto *callSymbol = new CallSymbol(methodUrl, m_existentialUrl, access, address,
        lyric_object::CallMode::Normal, priv->isDeclOnly, priv->existentialBlock.get(), m_state);

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
lyric_assembler::ExistentialSymbol::prepareMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    CallableInvoker &invoker,
    bool thisReceiver)
{
    auto *priv = getPriv();

    if (!priv->methods.contains(name)) {
        if (priv->superExistential == nullptr)
            return m_state->logAndContinue(AssemblerCondition::kMissingMethod,
                tempo_tracing::LogSeverity::kError,
                "missing method {}", name);
        return priv->superExistential->prepareMethod(name, receiverType, invoker, thisReceiver);
    }

    const auto &method = priv->methods.at(name);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(method.methodCall));
    if (symbol->getSymbolType() != SymbolType::CALL)
        m_state->throwAssemblerInvariant("invalid call symbol {}", method.methodCall.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);
    callSymbol->touch();

    if (callSymbol->isInline()) {
        auto callable = std::make_unique<ExistentialCallable>(callSymbol, callSymbol->callProc());
        return invoker.initialize(std::move(callable));
    }

    if (!callSymbol->isBound())
        m_state->throwAssemblerInvariant("invalid call symbol {}", callSymbol->getSymbolUrl().toString());

    auto callable = std::make_unique<ExistentialCallable>(this, callSymbol);
    return invoker.initialize(std::move(callable));
}

bool
lyric_assembler::ExistentialSymbol::hasImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    return priv->impls.contains(implUrl);
}

bool
lyric_assembler::ExistentialSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    return hasImpl(implType.getConcreteUrl());
}

lyric_assembler::ImplHandle *
lyric_assembler::ExistentialSymbol::getImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    auto iterator = priv->impls.find(implUrl);
    if (iterator != priv->impls.cend())
        return iterator->second;
    return nullptr;
}

lyric_assembler::ImplHandle *
lyric_assembler::ExistentialSymbol::getImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return nullptr;
    return getImpl(implType.getConcreteUrl());
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ExistentialSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ExistentialSymbol::implsEnd() const
{
    auto *priv = getPriv();
    return priv->impls.cend();
}

tu_uint32
lyric_assembler::ExistentialSymbol::numImpls() const
{
    auto *priv = getPriv();
    return priv->impls.size();
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::ExistentialSymbol::declareImpl(const lyric_common::TypeDef &implType)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare impl on imported existential {}", m_existentialUrl.toString());

    auto *priv = getPriv();

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        m_state->throwAssemblerInvariant("invalid impl type {}", implType.toString());
    auto implUrl = implType.getConcreteUrl();

    if (priv->impls.contains(implUrl))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "impl {} already defined for existential {}", implType.toString(), m_existentialUrl.toString());

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
    if (priv->existentialTemplate != nullptr) {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
            name, implTypeHandle, conceptSymbol, m_existentialUrl, priv->existentialTemplate, priv->isDeclOnly,
            priv->existentialBlock.get()));
    } else {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
            name, implTypeHandle, conceptSymbol, m_existentialUrl, priv->isDeclOnly, priv->existentialBlock.get()));
    }

    priv->impls[implUrl] = implHandle;

    return implHandle;
}

bool
lyric_assembler::ExistentialSymbol::hasSealedType(const lyric_common::TypeDef &sealedType) const
{
    auto *priv = getPriv();
    return priv->sealedTypes.contains(sealedType);
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ExistentialSymbol::sealedTypesBegin() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ExistentialSymbol::sealedTypesEnd() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cend();
}

tempo_utils::Status
lyric_assembler::ExistentialSymbol::putSealedType(const lyric_common::TypeDef &sealedType)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't put sealed type on imported existential {}", m_existentialUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "existential {} is not sealed", m_existentialUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "invalid derived type {} for sealed existential {}",
            sealedType.toString(), m_existentialUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(sealedUrl));
    if (symbol->getSymbolType() != SymbolType::EXISTENTIAL)
        m_state->throwAssemblerInvariant("invalid existential symbol {}", sealedUrl.toString());

    TypeHandle *derivedType = nullptr;
    switch (symbol->getSymbolType()) {
        case SymbolType::CLASS:
            derivedType = cast_symbol_to_class(symbol)->classType();
            break;
        case SymbolType::ENUM:
            derivedType = cast_symbol_to_enum(symbol)->enumType();
            break;
        case SymbolType::EXISTENTIAL:
            derivedType = cast_symbol_to_existential(symbol)->existentialType();
            break;
        case SymbolType::INSTANCE:
            derivedType = cast_symbol_to_instance(symbol)->instanceType();
            break;
        case SymbolType::STRUCT:
            derivedType = cast_symbol_to_struct(symbol)->structType();
            break;
        default:
            break;
    }

    if (derivedType == nullptr || derivedType->getSuperType() != priv->existentialType)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "{} does not derive from sealed existential {}",
            sealedType.toString(), m_existentialUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return {};
}
