#ifndef LYRIC_COMPILER_IMPORT_HANDLER_H
#define LYRIC_COMPILER_IMPORT_HANDLER_H

#include <lyric_assembler/call_symbol.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct Import {
        lyric_common::ModuleLocation importLocation;
        lyric_assembler::BlockHandle *importBlock;
        absl::flat_hash_set<lyric_assembler::ImportRef> importRefs;
    };

    class ImportHandler : public BaseGrouping {
    public:
        ImportHandler(
            bool isSideEffect,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);
        ImportHandler(
            lyric_assembler::NamespaceSymbol *parentNamespace,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status before(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            BeforeContext &ctx) override;

        tempo_utils::Status after(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            AfterContext &ctx) override;

    private:
        bool m_isSideEffect;
        lyric_assembler::NamespaceSymbol *m_parentNamespace;
        Import m_import;
    };

    class ImportSymbol : public BaseChoice {
    public:
        ImportSymbol(Import *import, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Import *m_import;
    };
}

#endif // LYRIC_COMPILER_IMPORT_HANDLER_H
