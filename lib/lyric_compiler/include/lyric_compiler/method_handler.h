#ifndef LYRIC_COMPILER_METHOD_HANDLER_H
#define LYRIC_COMPILER_METHOD_HANDLER_H

#include <lyric_assembler/call_symbol.h>
#include <lyric_typing/callsite_reifier.h>

#include "base_invokable_handler.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    class MethodHandler : public BaseInvokableHandler {
    public:
        MethodHandler(
            const lyric_common::TypeDef &receiverType,
            lyric_assembler::BlockHandle *bindingBlock,
            lyric_assembler::BlockHandle *invokeBlock,
            std::unique_ptr<lyric_assembler::CallableInvoker> &&invoker,
            std::unique_ptr<lyric_typing::CallsiteReifier> &&reifier,
            lyric_assembler::CodeFragment *fragment,
            CompilerScanDriver *driver);

        tempo_utils::Status after(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            AfterContext &ctx) override;

    private:
        lyric_common::TypeDef m_receiverType;
        std::unique_ptr<lyric_assembler::CallableInvoker> m_invoker;
        std::unique_ptr<lyric_typing::CallsiteReifier> m_reifier;
    };
}

#endif // LYRIC_COMPILER_METHOD_HANDLER_H
