
#include <lyric_importer/binding_import.h>
#include <lyric_importer/importer_result.h>
#include <lyric_object/binding_walker.h>

namespace lyric_importer {
    struct BindingImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        lyric_object::AccessType access;
        lyric_common::SymbolUrl targetUrl;
    };
}

lyric_importer::BindingImport::BindingImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 bindingOffset)
    : m_moduleImport(moduleImport),
      m_bindingOffset(bindingOffset)
{
    TU_ASSERT (m_moduleImport != nullptr);
    TU_ASSERT (m_bindingOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::BindingImport::getSymbolUrl()
{
    load();
    return m_priv->symbolUrl;
}

lyric_object::AccessType
lyric_importer::BindingImport::getAccess()
{
    return m_priv->access;
}

lyric_common::SymbolUrl
lyric_importer::BindingImport::getTargetUrl()
{
    load();
    return m_priv->targetUrl;
}

void
lyric_importer::BindingImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto location = m_moduleImport->getLocation();
    auto bindingWalker = m_moduleImport->getObject().getObject().getBinding(m_bindingOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(location, bindingWalker.getSymbolPath());

    priv->access = bindingWalker.getAccess();
    if (priv->access == lyric_object::AccessType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import binding at index {} in module {}; invalid access type",
                m_bindingOffset, location.toString()));

    switch (bindingWalker.targetAddressType()) {
        case lyric_object::AddressType::Near: {
            priv->targetUrl = lyric_common::SymbolUrl(
                location, bindingWalker.getNearTarget().getSymbolPath());
            break;
        }
        case lyric_object::AddressType::Far: {
            priv->targetUrl = bindingWalker.getFarTarget().getLinkUrl();
            break;
        }
        default:
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(lyric_importer::ImporterCondition::kImportError,
                    "cannot import binding at index {} in module {}; invalid target",
                    bindingWalker.getDescriptorOffset(), location.toString()));
    }

    m_priv = std::move(priv);
}
