
#include <lyric_importer/binding_import.h>
#include <lyric_importer/importer_result.h>
#include <lyric_object/binding_walker.h>

namespace lyric_importer {
    struct BindingImport::Priv {
        lyric_common::SymbolUrl symbolUrl;
        lyric_object::AccessType access;
        TypeImport *bindingType;
        TemplateImport *bindingTemplate;
        TypeImport *targetType;
    };
}

lyric_importer::BindingImport::BindingImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 bindingOffset)
    : BaseImport(moduleImport),
      m_bindingOffset(bindingOffset)
{
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
    load();
    return m_priv->access;
}

lyric_importer::TypeImport *
lyric_importer::BindingImport::getBindingType()
{
    load();
    return m_priv->bindingType;
}

lyric_importer::TemplateImport *
lyric_importer::BindingImport::getBindingTemplate()
{
    load();
    return m_priv->bindingTemplate;
}

lyric_importer::TypeImport *
lyric_importer::BindingImport::getTargetType()
{
    load();
    return m_priv->targetType;
}

void
lyric_importer::BindingImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto moduleImport = getModuleImport();
    auto location = moduleImport->getLocation();
    auto bindingWalker = moduleImport->getObject().getObject().getBinding(m_bindingOffset);
    priv->symbolUrl = lyric_common::SymbolUrl(location, bindingWalker.getSymbolPath());

    priv->access = bindingWalker.getAccess();
    if (priv->access == lyric_object::AccessType::Invalid)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(
                ImporterCondition::kImportError,
                "cannot import binding at index {} in module {}; invalid access type",
                m_bindingOffset, location.toString()));

    priv->bindingType = moduleImport->getType(
        bindingWalker.getBindingType().getDescriptorOffset());

    if (bindingWalker.hasTemplate()) {
        priv->bindingTemplate = moduleImport->getTemplate(
            bindingWalker.getTemplate().getDescriptorOffset());
    } else {
        priv->bindingTemplate = nullptr;
    }

    priv->targetType = moduleImport->getType(
        bindingWalker.getTargetType().getDescriptorOffset());

    m_priv = std::move(priv);
}
