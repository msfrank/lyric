#ifndef LYRIC_COMPILER_IMPL_HANDLER_H
#define LYRIC_COMPILER_IMPL_HANDLER_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/impl_handle.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "compiler_scan_driver.h"
#include "member_handler.h"
#include "method_handler.h"

namespace lyric_compiler {

    struct Impl {
        lyric_assembler::ImplHandle *implHandle = nullptr;
    };

    class ImplHandler : public BaseGrouping {
    public:
        ImplHandler(
            Impl impl,
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
        Impl m_impl;
    };

    class ImplDef : public BaseGrouping {
    public:
        ImplDef(
            Impl *impl,
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
        Impl *m_impl;
        lyric_assembler::ProcHandle *m_procHandle;
    };

    class ExtensionPack : public BaseGrouping {
    public:
        ExtensionPack(
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status before(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            BeforeContext &ctx) override;
    };

    class ExtensionParam : public BaseChoice {
    public:
        ExtensionParam(
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;
    };
}

#endif // LYRIC_COMPILER_IMPL_HANDLER_H
