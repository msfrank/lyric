
#include <lyric_importer/importer_result.h>
#include <lyric_importer/template_import.h>
#include <lyric_importer/type_import.h>
#include <lyric_object/template_walker.h>

namespace lyric_importer {
    struct TemplateImport::Priv {
        lyric_common::SymbolUrl templateUrl;
        TemplateImport *superTemplate;
        std::vector<TemplateParameter> templateParameters;
    };
}

lyric_importer::TemplateImport::TemplateImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 templateOffset)
    : m_moduleImport(moduleImport),
      m_templateOffset(templateOffset)
{
    TU_ASSERT (m_moduleImport != nullptr);
    TU_ASSERT (m_templateOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::TemplateImport::getTemplateUrl()
{
    load();
    return m_priv->templateUrl;
}

lyric_importer::TemplateImport *
lyric_importer::TemplateImport::getSuperTemplate()
{
    load();
    return m_priv->superTemplate;
}

lyric_importer::TemplateParameter
lyric_importer::TemplateImport::getTemplateParameter(int index)
{
    load();
    if (0 <= index && std::cmp_less(index, m_priv->templateParameters.size()))
        return m_priv->templateParameters.at(index);
    return {};
}

std::vector<lyric_importer::TemplateParameter>::const_iterator
lyric_importer::TemplateImport::templateParametersBegin()
{
    load();
    return m_priv->templateParameters.cbegin();
}

std::vector<lyric_importer::TemplateParameter>::const_iterator
lyric_importer::TemplateImport::templateParametersEnd()
{
    load();
    return m_priv->templateParameters.cend();
}

int
lyric_importer::TemplateImport::numTemplateParameters()
{
    load();
    return m_priv->templateParameters.size();
}

void
lyric_importer::TemplateImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto location = m_moduleImport->getLocation();
    auto templateWalker = m_moduleImport->getObject().getObject().getTemplate(m_templateOffset);
    priv->templateUrl = lyric_common::SymbolUrl(location, templateWalker.getSymbolPath());

    if (templateWalker.hasSuperTemplate()) {
        priv->superTemplate = m_moduleImport->getTemplate(
            templateWalker.getSuperTemplate().getDescriptorOffset());
    } else {
        priv->superTemplate = nullptr;
    }

    for (tu_uint8 i = 0; i < templateWalker.numTemplateParameters(); i++) {
        auto templateParameter = templateWalker.getTemplateParameter(i);

        TemplateParameter tp;
        tp.index = i;
        tp.name = templateParameter.getPlaceholderName();
        tp.variance = templateParameter.getPlaceholderVariance();
        tp.bound = templateParameter.getConstraintBound();

        auto constraintType = templateParameter.getConstraintType();
        if (constraintType.isValid()) {
            tp.type = m_moduleImport->getType(constraintType.getDescriptorOffset());
        } else {
            tp.type = nullptr;
        }

        priv->templateParameters.push_back(tp);
    }

    m_priv = std::move(priv);
}
