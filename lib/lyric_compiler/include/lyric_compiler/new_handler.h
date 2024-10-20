#ifndef LYRIC_COMPILER_NEW_HANDLER_H
#define LYRIC_COMPILER_NEW_HANDLER_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_invokable_handler.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class NewHandler : public BaseInvokableHandler {
    public:
        NewHandler(
            const lyric_common::TypeDef &typeHint,
            bool isSideEffect,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);
        NewHandler(
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
        lyric_common::TypeDef m_typeHint;
        bool m_isSideEffect;
        lyric_assembler::CodeFragment *m_fragment;
        lyric_assembler::ConstructableInvoker m_invoker;
        std::unique_ptr<lyric_typing::CallsiteReifier> m_reifier;
    };
}

#endif // LYRIC_COMPILER_NEW_HANDLER_H
