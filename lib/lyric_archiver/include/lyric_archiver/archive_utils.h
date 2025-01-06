#ifndef LYRIC_ARCHIVER_ARCHIVE_UTILS_H
#define LYRIC_ARCHIVER_ARCHIVE_UTILS_H

#include <lyric_assembler/abstract_symbol.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/importer_types.h>
#include <lyric_importer/template_import.h>

namespace lyric_archiver {

    lyric_common::SymbolUrl build_relative_url(
        const std::string &importHash,
        const lyric_common::SymbolUrl &symbolUrl);

    lyric_common::SymbolUrl build_absolute_url(
        const lyric_common::ModuleLocation &location,
        const std::string &importHash,
        const lyric_common::SymbolUrl &symbolUrl);

    lyric_common::SymbolUrl build_absolute_url(
        const lyric_common::ModuleLocation &location,
        const std::string &importHash,
        const lyric_assembler::AbstractSymbol *symbol);

    tempo_utils::Result<std::vector<lyric_object::TemplateParameter>> parse_template_parameters(
        lyric_importer::TemplateImport *templateImport);

}

#endif // LYRIC_ARCHIVER_ARCHIVE_UTILS_H
