#ifndef LYRIC_COMPILER_CONSTRUCTOR_HANDLER_H
#define LYRIC_COMPILER_CONSTRUCTOR_HANDLER_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_grouping.h"
#include "base_invokable_handler.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct Constructor {
        std::unique_ptr<lyric_assembler::ConstructableInvoker> superInvoker;
        lyric_assembler::CallSymbol *callSymbol = nullptr;
        lyric_assembler::ProcHandle *procHandle = nullptr;
    };

    class ConstructorHandler : public BaseGrouping {
    public:
        ConstructorHandler(
            std::unique_ptr<lyric_assembler::ConstructableInvoker> &&superInvoker,
            lyric_assembler::CallSymbol *initCall,
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

    class InitSuper : public BaseInvokableHandler {
    public:
        InitSuper(
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
    };
}

#endif // LYRIC_COMPILER_CONSTRUCTOR_HANDLER_H
