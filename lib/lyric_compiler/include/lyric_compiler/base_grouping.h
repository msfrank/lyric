#ifndef LYRIC_COMPILER_BASE_GROUPING_H
#define LYRIC_COMPILER_BASE_GROUPING_H

#include <lyric_assembler/block_handle.h>
#include <lyric_parser/archetype_node.h>

#include "visitor_context.h"

namespace lyric_compiler {

    class BaseGrouping : public AbstractGrouping {
    public:
        explicit BaseGrouping(CompilerScanDriver *driver);
        BaseGrouping(lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status before(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            BeforeContext &ctx) override;

        tempo_utils::Status after(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            AfterContext &ctx) override;

        lyric_assembler::BlockHandle *getBlock() override;
        CompilerScanDriver *getDriver() override;

    private:
        lyric_assembler::BlockHandle *m_block;
        CompilerScanDriver *m_driver;
        std::vector<std::unique_ptr<Handler>> m_handlers;
        int m_curr;

        Handler *currentHandler() const override;
        void appendHandlers(std::vector<std::unique_ptr<Handler>> &&handlers) override;
        bool advanceHandler() override;
        bool isFinished() const override;
    };
}

#endif // LYRIC_COMPILER_BASE_GROUPING_H
