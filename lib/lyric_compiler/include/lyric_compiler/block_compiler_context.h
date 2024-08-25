#ifndef LYRIC_COMPILER_BLOCK_COMPILER_CONTEXT_H
#define LYRIC_COMPILER_BLOCK_COMPILER_CONTEXT_H

#include <lyric_assembler/call_symbol.h>

#include "abstract_compiler_context.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class BlockCompilerContext : public AbstractCompilerContext {
    public:
        BlockCompilerContext(
            CompilerScanDriver *driver,
            lyric_assembler::BlockHandle *parentBlock);

        lyric_assembler::BlockHandle *getBlock() const override;

        tempo_utils::Status enter(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status exit(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            const lyric_rewriter::VisitorContext &ctx) override;

        tempo_utils::Status declareBlock(const lyric_parser::ArchetypeNode *node);

    private:
        CompilerScanDriver *m_driver;
        lyric_assembler::BlockHandle *m_parentBlock;
        std::unique_ptr<lyric_assembler::BlockHandle> m_block;
    };
}

#endif // LYRIC_COMPILER_BLOCK_COMPILER_CONTEXT_H
