
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/copy_template.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Result<lyric_assembler::TemplateHandle *>
lyric_archiver::copy_template(
    lyric_importer::TemplateImport *templateImport,
    const lyric_common::SymbolUrl &templateUrl,
    lyric_assembler::ObjectState *objectState)
{
    TU_ASSERT (templateImport != nullptr);
    TU_ASSERT (templateUrl.isValid());
    TU_ASSERT (objectState != nullptr);
    auto *typeCache = objectState->typeCache();

    if (templateImport->getSuperTemplate() != nullptr)
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "template import {} has unexpected super template",
            templateImport->getTemplateUrl().toString());

    std::vector<lyric_object::TemplateParameter> templateParameters;

    for (auto it = templateImport->templateParametersBegin(); it != templateImport->templateParametersEnd(); it++) {
        lyric_object::TemplateParameter tp;
        tp.index = it->index;
        tp.name = it->name;
        tp.typeDef = it->type->getTypeDef();
        tp.bound = it->bound;
        tp.variance = it->variance;
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
    lyric_importer::TemplateImport *templateImport,
    const lyric_common::SymbolUrl &templateUrl,
    lyric_assembler::TemplateHandle *superTemplate,
    lyric_assembler::ObjectState *objectState)
{
    TU_ASSERT (templateImport != nullptr);
    TU_ASSERT (templateUrl.isValid());
    TU_ASSERT (superTemplate != nullptr);
    TU_ASSERT (objectState != nullptr);
    auto *typeCache = objectState->typeCache();

    if (templateImport->getSuperTemplate() == nullptr)
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "template import {} does not have a super template",
            templateImport->getTemplateUrl().toString());

    std::vector<lyric_object::TemplateParameter> templateParameters;

    for (auto it = templateImport->templateParametersBegin(); it != templateImport->templateParametersEnd(); it++) {
        lyric_object::TemplateParameter tp;
        tp.index = it->index;
        tp.name = it->name;
        tp.typeDef = it->type->getTypeDef();
        tp.bound = it->bound;
        tp.variance = it->variance;
        templateParameters.push_back(std::move(tp));
    }


    auto templateHandle = std::make_unique<lyric_assembler::TemplateHandle>(
        templateUrl, superTemplate, templateParameters, objectState);
    TU_RETURN_IF_NOT_OK (typeCache->appendTemplate(templateHandle.get()));
    auto *templateHandlePtr = templateHandle.release();

    return templateHandlePtr;
}