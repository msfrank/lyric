#ifndef LYRIC_COMPILER_USING_HANDLER_H
#define LYRIC_COMPILER_USING_HANDLER_H

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct Using {
        lyric_assembler::CodeFragment *fragment = nullptr;
        lyric_assembler::DataReference usingRef;
        absl::flat_hash_set<lyric_common::TypeDef> implTypes;
    };

    class UsingHandler : public BaseGrouping {
    public:
        UsingHandler(
            bool isSideEffect,
            lyric_assembler::CodeFragment *fragment,
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
        Using m_using;
    };

    class UsingRef : public BaseGrouping {
    public:
        UsingRef(
            Using *using_,
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
        Using *m_using;
    };

    class UsingImpl : public BaseChoice {
    public:
        UsingImpl(
            Using *using_,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Using *m_using;
    };
}

#endif // LYRIC_COMPILER_USING_HANDLER_H
