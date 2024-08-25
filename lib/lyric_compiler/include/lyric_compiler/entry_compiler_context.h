#ifndef LYRIC_COMPILER_ENTRY_COMPILER_CONTEXT_H
#define LYRIC_COMPILER_ENTRY_COMPILER_CONTEXT_H

#include <lyric_assembler/call_symbol.h>

#include "abstract_compiler_context.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class EntryCompilerContext : public AbstractCompilerContext {
    public:
        EntryCompilerContext(
            CompilerScanDriver *driver,
            lyric_assembler::CallSymbol *entry,
            lyric_assembler::NamespaceSymbol *root);

        lyric_assembler::BlockHandle *getBlock() const override;

        tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

    private:
        CompilerScanDriver *m_driver;
        lyric_assembler::CallSymbol *m_entry;
        lyric_assembler::NamespaceSymbol *m_root;
    };
}

#endif // LYRIC_COMPILER_ENTRY_COMPILER_CONTEXT_H
