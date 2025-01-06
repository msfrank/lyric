
#include <lyric_archiver/archive_utils.h>

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
lyric_archiver::parse_template_parameters(lyric_importer::TemplateImport *templateImport)
{
    TU_ASSERT (templateImport != nullptr);

    std::vector<lyric_object::TemplateParameter> templateParameters;
    for (auto it = templateImport->templateParametersBegin(); it != templateImport->templateParametersEnd(); it++) {
        lyric_object::TemplateParameter tp;
        tp.index = it->index;
        tp.name = it->name;
        tp.typeDef = it->type->getTypeDef();
        tp.variance = it->variance;
        tp.bound = it->bound;
        templateParameters.push_back(std::move(tp));
    }

    return templateParameters;
}