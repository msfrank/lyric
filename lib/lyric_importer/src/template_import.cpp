
#include <lyric_importer/importer_result.h>
#include <lyric_importer/template_import.h>
#include <lyric_importer/type_import.h>
#include <lyric_object/template_walker.h>
#include <lyric_object/template_parameter_walker.h>

lyric_importer::TemplateImport::TemplateImport(std::weak_ptr<ModuleImport> moduleImport, tu_uint32 templateOffset)
    : BaseImport(std::move(moduleImport)),
      m_templateOffset(templateOffset)
{
    TU_ASSERT (m_templateOffset != lyric_object::INVALID_ADDRESS_U32);
}

lyric_common::SymbolUrl
lyric_importer::TemplateImport::getTemplateUrl()
{
    load();
    return m_priv->templateUrl;
}

bool
lyric_importer::TemplateImport::hasSuperTemplate()
{
    load();
    return m_priv->hasSuper;
}

std::weak_ptr<lyric_importer::TemplateImport>
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

tu_uint32
lyric_importer::TemplateImport::getTemplateOffset() const
{
    return m_templateOffset;
}

void
lyric_importer::TemplateImport::load()
{
    absl::MutexLock locker(&m_lock);

    if (m_priv != nullptr)
        return;

    auto priv = std::make_unique<Priv>();

    auto moduleImport = acquireModuleImport();
    if (moduleImport == nullptr)
        throw tempo_utils::StatusException(
            ImporterStatus::forCondition(ImporterCondition::kImporterInvariant,
                "invalid module import"));

    auto objectLocation = moduleImport->getObjectLocation();
    auto templateWalker = moduleImport->getObject().getTemplate(m_templateOffset);
    priv->templateUrl = lyric_common::SymbolUrl(objectLocation, templateWalker.getSymbolPath());

    priv->hasSuper = templateWalker.hasSuperTemplate();
    if (priv->hasSuper) {
        priv->superTemplate = moduleImport->getTemplate(
            templateWalker.getSuperTemplate().getDescriptorOffset());
    }

    for (tu_uint8 i = 0; i < templateWalker.numTemplateParameters(); i++) {
        auto templateParameter = templateWalker.getTemplateParameter(i);

        TemplateParameter tp;
        tp.index = i;
        tp.name = templateParameter.getPlaceholderName();
        if (tp.name.empty())
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(ImporterCondition::kImportError,
                "cannot import template at index {} in module {}; invalid template parameter at index {}",
                m_templateOffset, objectLocation.toString(), i));

        tp.variance = templateParameter.getPlaceholderVariance();
        if (tp.variance == lyric_object::VarianceType::Invalid)
            throw tempo_utils::StatusException(
                ImporterStatus::forCondition(ImporterCondition::kImportError,
                "cannot import template at index {} in module {}; invalid template parameter at index {}",
                m_templateOffset, objectLocation.toString(), i));

        if (templateParameter.hasConstraint()) {
            TemplateParameter::Constraint constraint;
            constraint.bound = templateParameter.getConstraintBound();
            if (constraint.bound == lyric_object::BoundType::Invalid)
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(ImporterCondition::kImportError,
                    "cannot import template at index {} in module {}; invalid constraint on template parameter at index {}",
                    m_templateOffset, objectLocation.toString(), i));

            auto constraintType = templateParameter.getConstraintType();
            if (!constraintType.isValid())
                throw tempo_utils::StatusException(
                    ImporterStatus::forCondition(ImporterCondition::kImportError,
                    "cannot import template at index {} in module {}; invalid constraint on template parameter at index {}",
                    m_templateOffset, objectLocation.toString(), i));

            constraint.type = moduleImport->getType(constraintType.getDescriptorOffset());
            tp.constraint = std::move(constraint);
        } else {
            tp.constraint = {};
        }

        priv->templateParameters.push_back(std::move(tp));
    }

    m_priv = std::move(priv);
}
