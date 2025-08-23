
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

lyric_assembler::StaticSymbol::StaticSymbol(
    const lyric_common::SymbolUrl &staticUrl,
    bool isHidden,
    bool isVariable,
    TypeHandle *staticType,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(new StaticSymbolPriv()),
      m_staticUrl(staticUrl),
      m_state(state)
{
    TU_ASSERT (m_staticUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->isHidden = isHidden;
    priv->isVariable = isVariable;
    priv->staticType = staticType;
    priv->parentBlock = parentBlock;
    priv->isDeclOnly = isDeclOnly;
    priv->staticBlock = std::make_unique<BlockHandle>(staticUrl, parentBlock);

    TU_ASSERT (priv->staticType != nullptr);
    TU_ASSERT (priv->parentBlock != nullptr);
}

lyric_assembler::StaticSymbol::StaticSymbol(
    const lyric_common::SymbolUrl &staticUrl,
    lyric_importer::StaticImport *staticImport,
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_staticUrl(staticUrl),
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
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<StaticSymbolPriv>();

    priv->parentBlock = nullptr;
    priv->isHidden = m_staticImport->isHidden();
    priv->isVariable = m_staticImport->isVariable();
    priv->isDeclOnly = m_staticImport->isDeclOnly();

    auto *staticType = m_staticImport->getStaticType();
    TU_ASSIGN_OR_RAISE (priv->staticType, typeCache->importType(staticType));

    auto *symbolCache = m_state->symbolCache();

    auto initializerUrl = m_staticImport->getInitializer();
    if (initializerUrl.isValid()) {
        CallSymbol *initializerCall;
        TU_ASSIGN_OR_RAISE (initializerCall, symbolCache->getOrImportCall(initializerUrl));
        priv->initializerHandle = std::make_unique<InitializerHandle>(getName(), initializerCall);
    }

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
lyric_assembler::StaticSymbol::getTypeDef() const
{
    auto *priv = getPriv();
    return priv->staticType->getTypeDef();
}

std::string
lyric_assembler::StaticSymbol::getName() const
{
    return m_staticUrl.getSymbolPath().getName();
}

bool
lyric_assembler::StaticSymbol::isHidden() const
{
    auto *priv = getPriv();
    return priv->isHidden;
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
    if (priv->initializerHandle != nullptr)
        return priv->initializerHandle->getSymbolUrl();
    return {};
}

tempo_utils::Result<lyric_assembler::InitializerHandle *>
lyric_assembler::StaticSymbol::defineInitializer()
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare init on imported static {}", m_staticUrl.toString());

    auto *priv = getPriv();

    lyric_common::SymbolPath path(m_staticUrl.getSymbolPath().getPath(), "$init");
    lyric_common::SymbolUrl initializerUrl(path);

    if (priv->initializerHandle != nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "initializer already defined for static {}", m_staticUrl.toString());

    auto identifier = absl::StrCat("$inits$", m_staticUrl.getSymbolName());

    // declare the initializer call
    CallSymbol *callSymbol;
    TU_ASSIGN_OR_RETURN (callSymbol, priv->parentBlock->declareFunction(
        identifier, /* isHidden= */ false, {}, priv->isDeclOnly));

    // define the initializer with no parameters
    TU_RETURN_IF_STATUS (callSymbol->defineCall({}));

    // create an initializer handle
    priv->initializerHandle = std::make_unique<InitializerHandle>(getName(), callSymbol);

    return priv->initializerHandle.get();
}

tempo_utils::Status
lyric_assembler::StaticSymbol::prepareInitializer(CallableInvoker &invoker)
{
    auto *priv = getPriv();

    if (priv->initializerHandle == nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingSymbol,
            "missing initializer for static {}", m_staticUrl.toString());

    auto initializerUrl = priv->initializerHandle->getSymbolUrl();

    auto *symbolCache = m_state->symbolCache();

    CallSymbol *initSymbol;
    TU_ASSIGN_OR_RETURN (initSymbol, symbolCache->getOrImportCall(initializerUrl));

    auto callable = std::make_unique<FunctionCallable>(initSymbol, /* isInlined= */ false);
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
