#ifndef LYRIC_COMPILER_METHOD_HANDLER_H
#define LYRIC_COMPILER_METHOD_HANDLER_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_grouping.h"
#include "base_invokable_handler.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct Method {
        lyric_assembler::DispatchType dispatch = lyric_assembler::DispatchType::Virtual;
        lyric_assembler::CallSymbol *callSymbol = nullptr;
        lyric_assembler::ProcHandle *procHandle = nullptr;
    };

    class MethodHandler : public BaseGrouping {
    public:
        MethodHandler(
            Method method,
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
        Method m_method;
    };

    class AbstractMethodHandler : public BaseGrouping {
    public:
        AbstractMethodHandler(
            Method method,
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
        Method m_method;
    };
}

#endif // LYRIC_COMPILER_METHOD_HANDLER_H
