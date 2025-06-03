
#include <lyric_assembler/binding_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/namespace_import.h>

lyric_assembler::NamespaceSymbol::NamespaceSymbol(
    const lyric_common::SymbolUrl &namespaceUrl,
    TypeHandle *namespaceType,
    BlockHandle *rootBlock,
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
    priv->namespaceBlock = std::make_unique<BlockHandle>(rootBlock, state);

    TU_ASSERT (priv->namespaceType != nullptr);
}

lyric_assembler::NamespaceSymbol::NamespaceSymbol(
    const lyric_common::SymbolUrl &namespaceUrl,
    lyric_object::AccessType access,
    TypeHandle *namespaceType,
    NamespaceSymbol *superNamespace,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
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
    priv->namespaceBlock = std::make_unique<BlockHandle>(namespaceUrl, parentBlock);

    TU_ASSERT (priv->namespaceType != nullptr);
    TU_ASSERT (priv->superNamespace != nullptr);
}

lyric_assembler::NamespaceSymbol::NamespaceSymbol(
    const lyric_common::SymbolUrl &namespaceUrl,
    lyric_importer::NamespaceImport *namespaceImport,
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_namespaceUrl(namespaceUrl),
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

    for (auto it = m_namespaceImport->symbolsBegin(); it != m_namespaceImport->symbolsEnd(); it++) {
        const auto &symbolUrl = *it;
        auto identifier = symbolUrl.getSymbolName();
        TU_RAISE_IF_STATUS (importCache->importSymbol(symbolUrl));
        priv->targets[identifier] = symbolUrl;
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

bool
lyric_assembler::NamespaceSymbol::hasTarget(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->targets.contains(name);
}

lyric_common::SymbolUrl
lyric_assembler::NamespaceSymbol::getTarget(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->targets.find(name);
    if (entry != priv->targets.cend())
        return entry->second;
    return {};
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_assembler::NamespaceSymbol::targetsBegin() const
{
    auto *priv = getPriv();
    return priv->targets.cbegin();
}

absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator
lyric_assembler::NamespaceSymbol::targetsEnd() const
{
    auto *priv = getPriv();
    return priv->targets.cend();
}

tu_uint32
lyric_assembler::NamespaceSymbol::numTargets() const
{
    auto *priv = getPriv();
    return priv->targets.size();
}

tempo_utils::Status
lyric_assembler::NamespaceSymbol::putTarget(const lyric_common::SymbolUrl &targetUrl)
{
    auto name = targetUrl.getSymbolName();
    auto *priv = getPriv();

    auto entry = priv->targets.find(name);
    if (entry != priv->targets.cend() && entry->second != targetUrl)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "cannot put namespace target {}; target is already defined", targetUrl.toString());

    priv->targets[name] = targetUrl;
    return {};
}

tempo_utils::Result<lyric_assembler::BindingSymbol *>
lyric_assembler::NamespaceSymbol::declareBinding(
    const std::string &name,
    lyric_object::AccessType access,
    const std::vector<lyric_object::TemplateParameter> &templateParameters)
{
    auto *priv = getPriv();
    if (priv->targets.contains(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "cannot put namespace binding {}; symbol is already defined", name);

    BindingSymbol *bindingSymbol;
    TU_ASSIGN_OR_RETURN (bindingSymbol, priv->namespaceBlock->declareBinding(name, access, templateParameters));

    priv->targets[name] = bindingSymbol->getSymbolUrl();

    return bindingSymbol;
}

tempo_utils::Result<lyric_assembler::NamespaceSymbol *>
lyric_assembler::NamespaceSymbol::declareSubspace(
    const std::string &name,
    lyric_object::AccessType access)
{
    auto *priv = getPriv();
    if (priv->targets.contains(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "cannot declare namespace {}; symbol is already defined", name);

    auto definition = priv->namespaceBlock->getDefinition();
    auto parentPath = definition.getSymbolPath();
    lyric_common::SymbolPath namespacePath(parentPath.getPath(), name);
    lyric_common::SymbolUrl namespaceUrl(m_namespaceUrl.getModuleLocation(), namespacePath);

    auto namespaceSymbol = std::make_unique<NamespaceSymbol>(
        namespaceUrl, access, priv->namespaceType, this, priv->isDeclOnly,
        priv->namespaceBlock.get(), m_state);

    NamespaceSymbol *nsPtr;
    TU_ASSIGN_OR_RETURN (nsPtr, m_state->appendNamespace(std::move(namespaceSymbol)));

    TU_RETURN_IF_STATUS (priv->namespaceBlock->declareAlias(name, namespaceUrl));

    priv->targets[name] = namespaceUrl;

    return nsPtr;
}