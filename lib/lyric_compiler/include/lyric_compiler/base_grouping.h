#ifndef LYRIC_COMPILER_BASE_GROUPING_H
#define LYRIC_COMPILER_BASE_GROUPING_H

#include <lyric_schema/ast_schema.h>

#include "abstract_behavior.h"
#include "visitor_context.h"

namespace lyric_compiler {

    class BaseGrouping {
    public:
        explicit BaseGrouping(CompilerScanDriver *driver);
        BaseGrouping(lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);
        virtual ~BaseGrouping() = default;

        virtual tempo_utils::Status before(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            BeforeContext &ctx);

        virtual tempo_utils::Status after(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            AfterContext &ctx);

        lyric_assembler::BlockHandle *getBlock();
        CompilerScanDriver *getDriver();


    private:
        lyric_assembler::BlockHandle *m_block;
        CompilerScanDriver *m_driver;
        std::vector<std::unique_ptr<Handler>> m_handlers;
        int m_curr;

//        tempo_utils::Status enter(
//            const lyric_parser::ArchetypeState *state,
//            const lyric_parser::ArchetypeNode *node,
//            EnterContext &ctx);
//        tempo_utils::Status exit(
//            const lyric_parser::ArchetypeState *state,
//            const lyric_parser::ArchetypeNode *node,
//            ExitContext &ctx);

        Handler *currentHandler() const;
        void appendHandlers(std::vector<std::unique_ptr<Handler>> &&handlers);
        bool advanceHandler();
        bool isFinished() const;

        friend class CompilerScanDriver;
    };
}

#endif // LYRIC_COMPILER_BASE_GROUPING_H
