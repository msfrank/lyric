
#include <lyric_importer/importer_result.h>
#include <lyric_importer/namespace_import.h>

namespace lyric_importer {
    struct NamespaceImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        bool isDeclOnly;
        lyric_object::AccessType access;
        lyric_common::SymbolUrl superNamespace;
        absl::flat_hash_set<lyric_common::SymbolUrl> symbols;
    };
}

lyric_importer::NamespaceImport::NamespaceImport(
    std::shared_ptr<ModuleImport> moduleImport,
    tu_uint32 namespaceOffset)
    : BaseImport(moduleImport),
      m_namespaceOffset(namespaceOffset)
{
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

lyric_object::AccessType
lyric_importer::NamespaceImport::getAccess()
{
    load();
    return m_priv->access;
}

lyric_common::SymbolUrl
lyric_importer::NamespaceImport::getSuperNamespace()
{
    load();
    return m_priv->superNamespace;
}

absl::flat_hash_set<lyric_common::SymbolUrl>::const_iterator
lyric_importer::NamespaceImport::symbolsBegin()
{
    load();
    return m_priv->symbols.cbegin();
}

absl::flat_hash_set<lyric_common::SymbolUrl>::const_iterator
lyric_importer::NamespaceImport::symbolsEnd()
{
    load();
    return m_priv->symbols.cend();
}

int
lyric_importer::NamespaceImport::numBindings()
{
    load();
    return m_priv->symbols.size();
}

void
lyric_importer::NamespaceImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto moduleImport = getModuleImport();
    auto objectLocation = moduleImport->getObjectLocation();
    auto namespaceWalker = moduleImport->getObject().getObject().getNamespace(m_namespaceOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(objectLocation, namespaceWalker.getSymbolPath());

    priv->isDeclOnly = namespaceWalker.isDeclOnly();

    priv->access = namespaceWalker.getAccess();
    if (priv->access == lyric_object::AccessType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import namespace at index {} in module {}; invalid access type",
                m_namespaceOffset, objectLocation.toString()));

    if (namespaceWalker.hasSuperNamespace()) {
        switch (namespaceWalker.superNamespaceAddressType()) {
            case lyric_object::AddressType::Near:
                priv->superNamespace = lyric_common::SymbolUrl(
                    objectLocation, namespaceWalker.getNearSuperNamespace().getSymbolPath());
                break;
            case lyric_object::AddressType::Far:
                priv->superNamespace = namespaceWalker.getFarSuperNamespace().getLinkUrl(objectLocation);
                break;
            default:
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(
                        ImporterCondition::kImportError,
                        "cannot import namespace at index {} in module {}; invalid super namespace",
                        m_namespaceOffset, objectLocation.toString()));
        }
    }

    // preallocate space for all the bindings
    priv->symbols.reserve(namespaceWalker.numSymbols());

    for (tu_uint32 i = 0; i < namespaceWalker.numSymbols(); i++) {
        auto symbol = namespaceWalker.getSymbol(i);
        lyric_common::SymbolUrl symbolUrl(objectLocation, symbol.getSymbolPath());
        priv->symbols.insert(std::move(symbolUrl));
    }

    m_priv = std::move(priv);
}
