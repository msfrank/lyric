
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

lyric_assembler::StaticSymbol::StaticSymbol(
    const lyric_common::SymbolUrl &staticUrl,
    bool isVariable,
    StaticAddress address,
    TypeHandle *staticType,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : BaseSymbol(address, new StaticSymbolPriv()),
      m_staticUrl(staticUrl),
      m_state(state)
{
    TU_ASSERT (m_staticUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->isVariable = isVariable;
    priv->staticType = staticType;
    priv->initCall = nullptr;
    priv->isDeclOnly = isDeclOnly;
    priv->staticBlock = std::make_unique<BlockHandle>(staticUrl, parentBlock, false);

    TU_ASSERT (priv->staticType != nullptr);
}

lyric_assembler::StaticSymbol::StaticSymbol(
    const lyric_common::SymbolUrl &staticUrl,
    lyric_importer::StaticImport *staticImport,
    AssemblyState *state)
    : m_staticUrl(staticUrl),
      m_staticImport(staticImport),
      m_state(state)
{
    TU_ASSERT (m_staticUrl.isValid());
    TU_ASSERT (m_staticImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::StaticSymbolPriv *
lyric_assembler::StaticSymbol::load()
{
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<StaticSymbolPriv>();
    priv->isVariable = m_staticImport->isVariable();

    auto *staticType = m_staticImport->getStaticType();
    TU_ASSIGN_OR_RAISE (priv->staticType, typeCache->importType(staticType));

    auto initializerUrl = m_staticImport->getInitializer();
    TU_ASSIGN_OR_RAISE (priv->initCall, importCache->importCall(initializerUrl));

    priv->isDeclOnly = m_staticImport->isDeclOnly();

    priv->staticBlock = std::make_unique<BlockHandle>(
        m_staticUrl, absl::flat_hash_map<std::string, SymbolBinding>(), m_state);

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::StaticSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Static;
}

lyric_assembler::SymbolType
lyric_assembler::StaticSymbol::getSymbolType() const
{
    return SymbolType::STATIC;
}

lyric_common::SymbolUrl
lyric_assembler::StaticSymbol::getSymbolUrl() const
{
    return m_staticUrl;
}

lyric_common::TypeDef
lyric_assembler::StaticSymbol::getAssignableType() const
{
    auto *priv = getPriv();
    return priv->staticType->getTypeDef();
}

lyric_assembler::TypeSignature
lyric_assembler::StaticSymbol::getTypeSignature() const
{
    auto *priv = getPriv();
    return priv->staticType->getTypeSignature();
}

void
lyric_assembler::StaticSymbol::touch()
{
    if (getAddress().isValid())
        return;
    m_state->touchStatic(this);
}

std::string
lyric_assembler::StaticSymbol::getName() const
{
    return m_staticUrl.getSymbolPath().getName();
}

bool
lyric_assembler::StaticSymbol::isVariable() const
{
    auto *priv = getPriv();
    return priv->isVariable;
}

bool
lyric_assembler::StaticSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

lyric_common::SymbolUrl
lyric_assembler::StaticSymbol::getInitializer() const
{
    auto *priv = getPriv();
    if (priv->initCall == nullptr)
        return {};
    return priv->initCall->getSymbolUrl();
}

tempo_utils::Result<lyric_assembler::ProcHandle *>
lyric_assembler::StaticSymbol::defineInitializer()
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare init on imported static {}", m_staticUrl.toString());

    auto *priv = getPriv();

    lyric_common::SymbolPath path(m_staticUrl.getSymbolPath().getPath(), "$init");
    lyric_common::SymbolUrl initializerUrl(path);

    if (priv->initCall != nullptr || m_state->symbolCache()->hasSymbol(initializerUrl))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "initializer already defined for static {}", m_staticUrl.toString());

    //
    auto returnType = getAssignableType();
    m_state->typeCache()->touchType(returnType);

    std::vector<lyric_object::Parameter> parameters;
    auto callIndex = m_state->numCalls();
    auto address = CallAddress::near(callIndex);

    // construct call symbol
    auto *initSymbol = new CallSymbol(initializerUrl, lyric_object::AccessType::Public, address,
        lyric_object::CallMode::Normal, priv->isDeclOnly, priv->staticBlock.get(), m_state);

    auto status = m_state->appendCall(initSymbol);
    if (status.notOk()) {
        delete initSymbol;
        return status;
    }

    priv->initCall = initSymbol;

    return priv->initCall->defineCall({}, returnType);
}

tempo_utils::Status
lyric_assembler::StaticSymbol::prepareInitializer(CallableInvoker &invoker)
{
    auto *priv = getPriv();

    if (priv->initCall == nullptr)
        return m_state->logAndContinue(AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing initializer for static {}", m_staticUrl.toString());

    auto initializerUrl = priv->initCall->getSymbolUrl();

    if (!m_state->symbolCache()->hasSymbol(initializerUrl))
        return m_state->logAndContinue(AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing init for static {}", m_staticUrl.toString());
    AbstractSymbol *initSym;
    TU_ASSIGN_OR_RETURN (initSym, m_state->symbolCache()->getOrImportSymbol(initializerUrl));
    if (initSym->getSymbolType() != SymbolType::CALL)
        m_state->throwAssemblerInvariant("invalid call symbol {}", initializerUrl.toString());
    auto *init = cast_symbol_to_call(initSym);

    auto callable = std::make_unique<FunctionCallable>(init);
    return invoker.initialize(std::move(callable));
}

lyric_assembler::DataReference
lyric_assembler::StaticSymbol::getReference() const
{
    auto *priv = getPriv();
    DataReference ref;
    ref.symbolUrl = m_staticUrl;
    ref.typeDef = priv->staticType->getTypeDef();
    ref.referenceType = priv->isVariable? ReferenceType::Variable : ReferenceType::Value;
    return ref;
}
