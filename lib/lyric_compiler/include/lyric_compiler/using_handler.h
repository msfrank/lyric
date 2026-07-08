#ifndef LYRIC_COMPILER_USING_HANDLER_H
#define LYRIC_COMPILER_USING_HANDLER_H

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct Capture {
        lyric_common::SymbolPath usingPath;
        lyric_assembler::DataReference captureRef;
        absl::flat_hash_set<lyric_common::TypeDef> implTypes;
    };

    class RootUsingHandler : public BaseGrouping {
    public:
        explicit RootUsingHandler(CompilerScanDriver *driver);

        tempo_utils::Status before(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            BeforeContext &ctx) override;

        tempo_utils::Status after(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            AfterContext &ctx) override;

    private:
        Capture m_capture;
    };

    class BlockUsingHandler : public BaseGrouping {
    public:
        BlockUsingHandler(
            bool isSideEffect,
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
        Capture m_capture;
    };

    class UsingImpl : public BaseChoice {
    public:
        UsingImpl(
            Capture *capture,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        Capture *m_capture;
    };
}

#endif // LYRIC_COMPILER_USING_HANDLER_H
