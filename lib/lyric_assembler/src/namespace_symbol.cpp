
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/namespace_import.h>

lyric_assembler::NamespaceSymbol::NamespaceSymbol(
    const lyric_common::SymbolUrl &namespaceUrl,
    lyric_object::AccessType access,
    TypeHandle *namespaceType,
    NamespaceSymbol *superNamespace,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state,
    bool isRoot)
    : BaseSymbol(new NamespaceSymbolPriv()),
      m_namespaceUrl(namespaceUrl),
      m_state(state)
{
    TU_ASSERT (m_namespaceUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = access;
    priv->isDeclOnly = isDeclOnly;
    priv->namespaceType = namespaceType;
    priv->superNamespace = superNamespace;
    priv->namespaceBlock = std::make_unique<BlockHandle>(namespaceUrl, parentBlock, isRoot);

    TU_ASSERT (priv->namespaceType != nullptr);
    TU_ASSERT (priv->superNamespace != nullptr);
}

lyric_assembler::NamespaceSymbol::NamespaceSymbol(
    const lyric_common::SymbolUrl &namespaceUrl,
    TypeHandle *namespaceType,
    ProcHandle *entryProc,
    ObjectState *state)
    : BaseSymbol(new NamespaceSymbolPriv()),
      m_namespaceUrl(namespaceUrl),
      m_state(state)
{
    TU_ASSERT (m_namespaceUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = lyric_object::AccessType::Public;
    priv->isDeclOnly = false;
    priv->namespaceType = namespaceType;
    priv->superNamespace = nullptr;
    priv->namespaceBlock = std::make_unique<BlockHandle>(this, entryProc, entryProc->procBlock(), state, true);

    TU_ASSERT (priv->namespaceType != nullptr);
}

lyric_assembler::NamespaceSymbol::NamespaceSymbol(
    const lyric_common::SymbolUrl &namespaceUrl,
    lyric_importer::NamespaceImport *namespaceImport,
    ObjectState *state)
    : m_namespaceUrl(namespaceUrl),
      m_namespaceImport(namespaceImport),
      m_state(state)
{
    TU_ASSERT (m_namespaceUrl.isValid());
    TU_ASSERT (m_namespaceImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::NamespaceSymbolPriv *
lyric_assembler::NamespaceSymbol::load()
{
    auto *fundamentalCache = m_state->fundamentalCache();
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<NamespaceSymbolPriv>();

    priv->access = lyric_object::AccessType::Public;
    priv->isDeclOnly = m_namespaceImport->isDeclOnly();

    TU_ASSIGN_OR_RAISE (priv->namespaceType, typeCache->getOrMakeType(
        fundamentalCache->getFundamentalType(FundamentalSymbol::Namespace)));

    auto superNamespaceUrl = m_namespaceImport->getSuperNamespace();
    if (superNamespaceUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superNamespace, importCache->importNamespace(superNamespaceUrl));
    }

    priv->namespaceBlock = std::make_unique<BlockHandle>(
        m_namespaceUrl, absl::flat_hash_map<std::string, SymbolBinding>(), m_state);

    for (auto iterator = m_namespaceImport->bindingsBegin(); iterator != m_namespaceImport->bindingsEnd(); iterator++) {
        auto &bindingUrl = *iterator;

        TU_RAISE_IF_STATUS(importCache->importSymbol(bindingUrl));
        auto bindingPath = bindingUrl.getSymbolPath();
        auto identifier = bindingPath.getName();
        priv->namespaceBlock->declareAlias(identifier, bindingUrl);
    }

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::NamespaceSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Namespace;
}

lyric_assembler::SymbolType
lyric_assembler::NamespaceSymbol::getSymbolType() const
{
    return SymbolType::NAMESPACE;
}

lyric_common::SymbolUrl
lyric_assembler::NamespaceSymbol::getSymbolUrl() const
{
    return m_namespaceUrl;
}

lyric_common::TypeDef
lyric_assembler::NamespaceSymbol::getTypeDef() const
{
    auto *priv = getPriv();
    return priv->namespaceType->getTypeDef();
}

lyric_object::AccessType
lyric_assembler::NamespaceSymbol::getAccessType() const
{
    auto *priv = getPriv();
    return priv->access;
}

bool
lyric_assembler::NamespaceSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

lyric_assembler::NamespaceSymbol *
lyric_assembler::NamespaceSymbol::superNamespace() const
{
    auto *priv = getPriv();
    return priv->superNamespace;
}

lyric_assembler::TypeHandle *
lyric_assembler::NamespaceSymbol::namespaceType() const
{
    auto *priv = getPriv();
    return priv->namespaceType;
}

lyric_assembler::BlockHandle *
lyric_assembler::NamespaceSymbol::namespaceBlock() const
{
    auto *priv = getPriv();
    return priv->namespaceBlock.get();
}