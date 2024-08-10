
#include <lyric_importer/importer_result.h>
#include <lyric_importer/namespace_import.h>

namespace lyric_importer {
    struct NamespaceImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        bool isDeclOnly;
        lyric_common::SymbolUrl superNamespace;
        absl::flat_hash_set<lyric_common::SymbolUrl> bindings;
    };
}

lyric_importer::NamespaceImport::NamespaceImport(
    std::shared_ptr<ModuleImport> moduleImport,
    tu_uint32 namespaceOffset)
    : m_moduleImport(moduleImport),
      m_namespaceOffset(namespaceOffset)
{
    TU_ASSERT (m_moduleImport != nullptr);
    TU_ASSERT (m_namespaceOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::NamespaceImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::NamespaceImport::isDeclOnly()
{
    load();
    return m_priv->isDeclOnly;
}

lyric_common::SymbolUrl
lyric_importer::NamespaceImport::getSuperNamespace()
{
    load();
    return m_priv->superNamespace;
}

absl::flat_hash_set<lyric_common::SymbolUrl>::const_iterator
lyric_importer::NamespaceImport::bindingsBegin()
{
    load();
    return m_priv->bindings.cbegin();
}

absl::flat_hash_set<lyric_common::SymbolUrl>::const_iterator
lyric_importer::NamespaceImport::bindingsEnd()
{
    load();
    return m_priv->bindings.cend();
}

int
lyric_importer::NamespaceImport::numBindings()
{
    load();
    return m_priv->bindings.size();
}

void
lyric_importer::NamespaceImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto location = m_moduleImport->getLocation();
    auto namespaceWalker = m_moduleImport->getObject().getObject().getNamespace(m_namespaceOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(location, namespaceWalker.getSymbolPath());

    priv->isDeclOnly = namespaceWalker.isDeclOnly();

    if (namespaceWalker.hasSuperNamespace()) {
        switch (namespaceWalker.superNamespaceAddressType()) {
            case lyric_object::AddressType::Near:
                priv->superNamespace = lyric_common::SymbolUrl(
                    location, namespaceWalker.getNearSuperNamespace().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                priv->superNamespace = namespaceWalker.getFarSuperNamespace().getLinkUrl();
                break;
            default:
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import namespace at index {} in assembly {}; invalid super namespace",
                        m_namespaceOffset, location.toString()));
        }
    }

    // preallocate space for all the bindings
    priv->bindings.reserve(namespaceWalker.numBindings());

    for (tu_uint32 i = 0; i < namespaceWalker.numBindings(); i++) {
        auto bindingSymbol = namespaceWalker.getBinding(i);
        lyric_common::SymbolUrl bindingUrl(location, bindingSymbol.getSymbolPath());
        priv->bindings.insert(std::move(bindingUrl));
    }

    m_priv = std::move(priv);
}
