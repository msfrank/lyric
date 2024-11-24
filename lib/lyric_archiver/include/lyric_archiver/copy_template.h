#ifndef LYRIC_ARCHIVER_COPY_TEMPLATE_H
#define LYRIC_ARCHIVER_COPY_TEMPLATE_H

#include <lyric_assembler/object_root.h>
#include <lyric_common/symbol_url.h>
#include <lyric_importer/template_import.h>

namespace lyric_archiver {

    tempo_utils::Result<lyric_assembler::TemplateHandle *> copy_template(
        lyric_importer::TemplateImport *templateImport,
        const lyric_common::SymbolUrl &templateUrl,
        lyric_assembler::ObjectState *objectState);

    tempo_utils::Result<lyric_assembler::TemplateHandle *> copy_template(
        lyric_importer::TemplateImport *templateImport,
        const lyric_common::SymbolUrl &templateUrl,
        lyric_assembler::TemplateHandle *superTemplate,
        lyric_assembler::ObjectState *objectState);
}

#endif // LYRIC_ARCHIVER_COPY_TEMPLATE_H
