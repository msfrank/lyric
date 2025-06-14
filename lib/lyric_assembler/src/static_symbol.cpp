
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

lyric_assembler::StaticSymbol::StaticSymbol(
    const lyric_common::SymbolUrl &staticUrl,
    lyric_object::AccessType access,
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
    priv->access = access;
    priv->isVariable = isVariable;
    priv->staticType = staticType;
    priv->initCall = nullptr;
    priv->isDeclOnly = isDeclOnly;
    priv->staticBlock = std::make_unique<BlockHandle>(staticUrl, parentBlock);

    TU_ASSERT (priv->staticType != nullptr);
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
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<StaticSymbolPriv>();
    priv->access = m_staticImport->getAccess();
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

lyric_object::AccessType
lyric_assembler::StaticSymbol::getAccessType() const
{
    auto *priv = getPriv();
    return priv->access;
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
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare init on imported static {}", m_staticUrl.toString());

    auto *priv = getPriv();

    lyric_common::SymbolPath path(m_staticUrl.getSymbolPath().getPath(), "$init");
    lyric_common::SymbolUrl initializerUrl(path);

    if (priv->initCall != nullptr || m_state->symbolCache()->hasSymbol(initializerUrl))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "initializer already defined for static {}", m_staticUrl.toString());

    //
    auto returnType = getTypeDef();

    std::vector<lyric_object::Parameter> parameters;

    // construct call symbol
    auto initSymbol = std::make_unique<CallSymbol>(initializerUrl, lyric_object::AccessType::Public,
        lyric_object::CallMode::Normal, priv->isDeclOnly, priv->staticBlock.get(), m_state);

    TU_ASSIGN_OR_RETURN (priv->initCall, m_state->appendCall(std::move(initSymbol)));

    return priv->initCall->defineCall({}, returnType);
}

tempo_utils::Status
lyric_assembler::StaticSymbol::prepareInitializer(CallableInvoker &invoker)
{
    auto *priv = getPriv();

    if (priv->initCall == nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingSymbol,
            "missing initializer for static {}", m_staticUrl.toString());

    auto initializerUrl = priv->initCall->getSymbolUrl();

    if (!m_state->symbolCache()->hasSymbol(initializerUrl))
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingSymbol,
            "missing init for static {}", m_staticUrl.toString());
    AbstractSymbol *initSym;
    TU_ASSIGN_OR_RETURN (initSym, m_state->symbolCache()->getOrImportSymbol(initializerUrl));
    if (initSym->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", initializerUrl.toString());
    auto *init = cast_symbol_to_call(initSym);

    auto callable = std::make_unique<FunctionCallable>(init, /* isInlined= */ false);
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
