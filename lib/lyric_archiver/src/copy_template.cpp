
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/copy_template.h>
#include <lyric_assembler/type_cache.h>

#include "lyric_assembler/fundamental_cache.h"

tempo_utils::Result<lyric_assembler::TemplateHandle *>
lyric_archiver::copy_template(
    std::weak_ptr<lyric_importer::TemplateImport> templateImport,
    const lyric_common::SymbolUrl &templateUrl,
    lyric_assembler::ObjectState *objectState)
{
    TU_ASSERT (templateUrl.isValid());
    TU_ASSERT (objectState != nullptr);
    auto *typeCache = objectState->typeCache();

    auto sharedTemplate = templateImport.lock();
    if (sharedTemplate == nullptr)
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "missing template import");

    if (sharedTemplate->hasSuperTemplate())
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "template import {} has unexpected super template", templateUrl.toString());

    std::vector<lyric_object::TemplateParameter> templateParameters;

    for (auto it = sharedTemplate->templateParametersBegin(); it != sharedTemplate->templateParametersEnd(); it++) {
        lyric_object::TemplateParameter tp;
        tp.index = it->index;
        tp.name = it->name;
        tp.variance = it->variance;

        if (it->constraint.has_value()) {
            const auto &constraint = it->constraint.value();
            tp.bound = constraint.bound;
            auto constraintImport = constraint.type.lock();
            if (constraintImport == nullptr)
                return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
                    "template import {} has invalid template parameter at index {}",
                    templateUrl.toString(), it->index);
            tp.typeDef = constraintImport->getTypeDef();
        } else {
            auto *fundamentalCache = objectState->fundamentalCache();
            tp.bound = lyric_object::BoundType::Extends;
            tp.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
        }

        templateParameters.push_back(std::move(tp));
    }

    auto templateHandle = std::make_unique<lyric_assembler::TemplateHandle>(
        templateUrl, /* superTemplate= */ nullptr, templateParameters, objectState);
    TU_RETURN_IF_NOT_OK (typeCache->appendTemplate(templateHandle.get()));
    auto *templateHandlePtr = templateHandle.release();

    return templateHandlePtr;
}

tempo_utils::Result<lyric_assembler::TemplateHandle *>
lyric_archiver::copy_template(
    std::weak_ptr<lyric_importer::TemplateImport> templateImport,
    const lyric_common::SymbolUrl &templateUrl,
    lyric_assembler::TemplateHandle *superTemplate,
    lyric_assembler::ObjectState *objectState)
{
    TU_ASSERT (templateUrl.isValid());
    TU_ASSERT (superTemplate != nullptr);
    TU_ASSERT (objectState != nullptr);
    auto *typeCache = objectState->typeCache();

    auto sharedTemplate = templateImport.lock();
    if (sharedTemplate == nullptr)
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "missing template import");

    if (!sharedTemplate->hasSuperTemplate())
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "template import {} does not have a super template", templateUrl.toString());

    std::vector<lyric_object::TemplateParameter> templateParameters;

    for (auto it = sharedTemplate->templateParametersBegin(); it != sharedTemplate->templateParametersEnd(); it++) {
        lyric_object::TemplateParameter tp;
        tp.index = it->index;
        tp.name = it->name;
        tp.variance = it->variance;

        if (it->constraint.has_value()) {
            const auto &constraint = it->constraint.value();
            tp.bound = constraint.bound;
            auto constraintImport = constraint.type.lock();
            if (constraintImport == nullptr)
                return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
                    "template import {} has invalid template parameter at index {}",
                    templateUrl.toString(), it->index);
            tp.typeDef = constraintImport->getTypeDef();
        } else {
            auto *fundamentalCache = objectState->fundamentalCache();
            tp.bound = lyric_object::BoundType::Extends;
            tp.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
        }

        templateParameters.push_back(std::move(tp));
    }


    auto templateHandle = std::make_unique<lyric_assembler::TemplateHandle>(
        templateUrl, superTemplate, templateParameters, objectState);
    TU_RETURN_IF_NOT_OK (typeCache->appendTemplate(templateHandle.get()));
    auto *templateHandlePtr = templateHandle.release();

    return templateHandlePtr;
}