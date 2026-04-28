#ifndef LYRIC_COMPILER_DATA_DEREF_HANDLER_H
#define LYRIC_COMPILER_DATA_DEREF_HANDLER_H

#include "base_choice.h"
#include "base_grouping.h"
#include "base_invokable_handler.h"
#include "block_handler.h"
#include "compiler_scan_driver.h"
#include "deref_utils.h"
#include "new_handler.h"

namespace lyric_compiler {

    struct DataDeref {
        lyric_assembler::BlockHandle *block = nullptr;
        lyric_assembler::CodeFragment *fragment = nullptr;
        std::vector<DerefEffect> effects;
    };

    /**
     * DerefThisBehavior handles a single This node which is not wrapped in a DataDeref node. After
     * processing the node the result will be pushed onto the result stack, unless `isSideEffect` is true,
     * in which case the result will be dropped.
     */
    class DerefThisBehavior : public AbstractBehavior {
    public:
        DerefThisBehavior(DataDeref *deref, CompilerScanDriver *driver);
        DerefThisBehavior(
            bool isSideEffect,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);
        ~DerefThisBehavior();

        tempo_utils::Status enter(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            EnterContext &ctx) override;
        tempo_utils::Status exit(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            ExitContext &ctx) override;

    private:
        bool m_isSideEffect;
        bool m_privateDeref;
        DataDeref *m_deref;
        CompilerScanDriver *m_driver;
    };

    /**
     * DerefNameBehavior handles a single Name node which is not wrapped in a DataDeref node. After
     * processing the node the result will be pushed onto the result stack, unless `isSideEffect` is true,
     * in which case the result will be dropped.
     */
    class DerefNameBehavior : public AbstractBehavior {
    public:
        DerefNameBehavior(DataDeref *deref, CompilerScanDriver *driver);
        DerefNameBehavior(
            bool isSideEffect,
            lyric_assembler::CodeFragment *fragment,
            lyric_assembler::BlockHandle *block,
            CompilerScanDriver *driver);
        ~DerefNameBehavior();

        tempo_utils::Status enter(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            EnterContext &ctx) override;
        tempo_utils::Status exit(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            ExitContext &ctx) override;

    private:
        bool m_isSideEffect;
        bool m_privateDeref;
        DataDeref *m_deref;
        CompilerScanDriver *m_driver;
    };

    /**
     * DataDerefHandle handles a DataDeref node.
     */
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

    /**
     *
     */
    class DataDerefBlock : public BaseGrouping {
    public:
        DataDerefBlock(
            DataDeref *deref,
            lyric_assembler::BlockHandle *block,
            lyric_assembler::CodeFragment *fragment,
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
        DataDeref *m_deref;
        std::unique_ptr<BlockHandler> m_blockHandler;
    };

    /**
     *
     */
    class DataDerefNew : public BaseGrouping {
    public:
        DataDerefNew(
            DataDeref *deref,
            lyric_assembler::BlockHandle *block,
            lyric_assembler::CodeFragment *fragment,
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
        DataDeref *m_deref;
        std::unique_ptr<NewHandler> m_newHandler;
    };

    /**
     *
     */
    class DataDerefCall : public BaseInvokableHandler {
    public:
        DataDerefCall(
            DataDeref *deref,
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
        DataDeref *m_deref;
        std::unique_ptr<lyric_assembler::CallableInvoker> m_invoker;
        std::unique_ptr<lyric_typing::CallsiteReifier> m_reifier;
    };

    /**
     *
     */
    class DataDerefMethod : public BaseInvokableHandler {
    public:
        DataDerefMethod(
            DataDeref *deref,
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
        DataDeref *m_deref;
        std::unique_ptr<lyric_assembler::CallableInvoker> m_invoker;
        std::unique_ptr<lyric_typing::CallsiteReifier> m_reifier;
    };

}

#endif // LYRIC_COMPILER_DATA_DEREF_HANDLER_H
