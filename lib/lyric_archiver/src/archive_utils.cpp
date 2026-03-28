
#include <lyric_archiver/archive_utils.h>

#include "lyric_archiver/archiver_result.h"
#include "lyric_assembler/fundamental_cache.h"

lyric_common::SymbolUrl
lyric_archiver::build_relative_url(const std::string &importHash, const lyric_common::SymbolUrl &symbolUrl)
{
    std::vector<std::string> parts{importHash};
    auto symbolPath = symbolUrl.getSymbolPath().getPath();
    parts.insert(parts.end(), symbolPath.cbegin(), symbolPath.cend());
    return lyric_common::SymbolUrl(lyric_common::SymbolPath(parts));
}

lyric_common::SymbolUrl
lyric_archiver::build_absolute_url(
    const lyric_common::ModuleLocation &location,
    const std::string &importHash,
    const lyric_common::SymbolUrl &symbolUrl)
{
    std::vector<std::string> parts{importHash};
    auto symbolPath = symbolUrl.getSymbolPath().getPath();
    parts.insert(parts.end(), symbolPath.cbegin(), symbolPath.cend());
    return lyric_common::SymbolUrl(location, lyric_common::SymbolPath(parts));
}

lyric_common::SymbolUrl
lyric_archiver::build_absolute_url(
    const lyric_common::ModuleLocation &location,
    const std::string &importHash,
    const lyric_assembler::AbstractSymbol *symbol)
{
    return build_absolute_url(location, importHash, symbol->getSymbolUrl());
}

tempo_utils::Result<std::vector<lyric_object::TemplateParameter>>
lyric_archiver::parse_template_parameters(
    std::weak_ptr<lyric_importer::TemplateImport> templateImport,
    const lyric_assembler::ObjectState *objectState)
{
    TU_NOTNULL (objectState);

    auto sharedTemplate = templateImport.lock();
    if (sharedTemplate == nullptr)
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "missing template import");

    std::vector<lyric_object::TemplateParameter> templateParameters;
    for (auto it = sharedTemplate->templateParametersBegin(); it != sharedTemplate->templateParametersEnd(); it++) {
        lyric_object::TemplateParameter tp;
        tp.index = it->index;
        tp.name = it->name;
        tp.variance = it->variance;

        if (it->constraint.has_value()) {
            const auto &constraint = it->constraint.value();
            tp.bound = constraint.bound;

            auto typeImport = constraint.type.lock();
            if (typeImport == nullptr)
                return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
                    "invalid type import");
            tp.typeDef = typeImport->getTypeDef();
        } else {
            auto *fundamentalCache = objectState->fundamentalCache();
            tp.bound = lyric_object::BoundType::Extends;
            tp.typeDef = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Any);
        }

        templateParameters.push_back(std::move(tp));
    }

    return templateParameters;
}