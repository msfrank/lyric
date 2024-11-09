#ifndef LYRIC_COMPILER_DATA_DEREF_HANDLER_H
#define LYRIC_COMPILER_DATA_DEREF_HANDLER_H

#include <lyric_assembler/call_symbol.h>

#include "base_choice.h"
#include "base_grouping.h"
#include "base_invokable_handler.h"
#include "compiler_scan_driver.h"

namespace lyric_compiler {

    struct DataDeref {
        lyric_assembler::CodeFragment *fragment = nullptr;
        lyric_assembler::BlockHandle *bindingBlock = nullptr;
        lyric_assembler::BlockHandle *invokeBlock = nullptr;
        lyric_assembler::DataReference currentRef;
        bool thisReceiver = false;
    };

    class DataDerefHandler : public BaseGrouping {
    public:
        DataDerefHandler(
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
        DataDeref m_deref;
    };

    class DataDerefSingle : public BaseChoice {
    public:
        DataDerefSingle(DataDeref *deref, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        DataDeref *m_deref;
    };

    class DataDerefFirst : public BaseChoice {
    public:
        DataDerefFirst(DataDeref *deref, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        DataDeref *m_deref;
    };

    class DataDerefCall : public BaseInvokableHandler {
    public:
        DataDerefCall(
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
        std::unique_ptr<lyric_assembler::CallableInvoker> m_invoker;
        std::unique_ptr<lyric_typing::CallsiteReifier> m_reifier;
    };

    class DataDerefNext : public BaseChoice {
    public:
        DataDerefNext(DataDeref *deref, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        DataDeref *m_deref;
    };

    class DataDerefMethod : public BaseInvokableHandler {
    public:
        DataDerefMethod(
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

    class DataDerefLast : public BaseChoice {
    public:
        DataDerefLast(DataDeref *deref, lyric_assembler::BlockHandle *block, CompilerScanDriver *driver);

        tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) override;

    private:
        DataDeref *m_deref;
    };
}

#endif // LYRIC_COMPILER_DATA_DEREF_HANDLER_H
