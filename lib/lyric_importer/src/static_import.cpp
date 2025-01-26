
#include <lyric_importer/importer_result.h>
#include <lyric_importer/static_import.h>
#include <lyric_object/static_walker.h>

namespace lyric_importer {
    struct StaticImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        bool isVariable;
        bool isDeclOnly;
        lyric_object::AccessType access;
        TypeImport *staticType;
        lyric_common::SymbolUrl initializer;
    };
}

lyric_importer::StaticImport::StaticImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 staticOffset)
    : BaseImport(moduleImport),
      m_staticOffset(staticOffset)
{
    TU_ASSERT (m_staticOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::StaticImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

bool
lyric_importer::StaticImport::isVariable()
{
    load();
    return m_priv->isVariable;
}

lyric_object::AccessType
lyric_importer::StaticImport::getAccess()
{
    return m_priv->access;
}

lyric_importer::TypeImport *
lyric_importer::StaticImport::getStaticType()
{
    load();
    return m_priv->staticType;
}

lyric_common::SymbolUrl
lyric_importer::StaticImport::getInitializer()
{
    load();
    return m_priv->initializer;
}

bool
lyric_importer::StaticImport::isDeclOnly()
{
    load();
    return m_priv->isDeclOnly;
}

void
lyric_importer::StaticImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto moduleImport = getModuleImport();
    auto location = moduleImport->getLocation();
    auto staticWalker = moduleImport->getObject().getObject().getStatic(m_staticOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(location, staticWalker.getSymbolPath());

    priv->isVariable = staticWalker.isVariable();
    priv->isDeclOnly = staticWalker.isDeclOnly();

    priv->access = staticWalker.getAccess();
    if (priv->access == lyric_object::AccessType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import static at index {} in module {}; invalid access type",
                m_staticOffset, location.toString()));

    priv->staticType = moduleImport->getType(staticWalker.getStaticType().getDescriptorOffset());

    switch (staticWalker.initializerAddressType()) {
        case lyric_object::AddressType::Near: {
            priv->initializer = lyric_common::SymbolUrl(
                location, staticWalker.getNearInitializer().getSymbolPath());
            break;
        }
        case lyric_object::AddressType::Far: {
            priv->initializer = staticWalker.getFarInitializer().getLinkUrl();
            break;
        }
        default:
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                    "cannot import static at index {} in module {}; invalid initializer",
                    staticWalker.getDescriptorOffset(), location.toString()));
    }

    m_priv = std::move(priv);
}
