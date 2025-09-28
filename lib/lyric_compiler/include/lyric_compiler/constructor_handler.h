#ifndef LYRIC_COMPILER_CONSTRUCTOR_HANDLER_H
#define LYRIC_COMPILER_CONSTRUCTOR_HANDLER_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_grouping.h"
#include "base_invokable_handler.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct Constructor {
        lyric_assembler::AbstractSymbol *thisSymbol = nullptr;
        lyric_assembler::AbstractSymbol *superSymbol = nullptr;
        lyric_assembler::CallSymbol *callSymbol = nullptr;
        lyric_assembler::ProcHandle *procHandle = nullptr;
    };

    class ConstructorHandler : public BaseGrouping {
    public:
        ConstructorHandler(
            Constructor constructor,
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
        Constructor m_constructor;
    };

    class InitBase : public BaseInvokableHandler {
    public:
        InitBase(
            Constructor *constructor,
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
        Constructor *m_constructor;
        std::unique_ptr<lyric_assembler::ConstructableInvoker> m_invoker;
    };
}

#endif // LYRIC_COMPILER_CONSTRUCTOR_HANDLER_H
