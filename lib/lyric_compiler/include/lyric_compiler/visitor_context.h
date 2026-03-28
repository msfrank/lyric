#ifndef LYRIC_COMPILER_VISITOR_CONTEXT_H
#define LYRIC_COMPILER_VISITOR_CONTEXT_H

#include <lyric_rewriter/rewrite_processor.h>

namespace lyric_compiler {

    // forward declarations
    class AbstractBehavior;
    class AfterContext;
    class BeforeContext;
    class CompilerScanDriver;
    class DecideContext;
    class EnterContext;
    class ExitContext;
    struct Handler;

    class AbstractBehavior {
    public:
        virtual ~AbstractBehavior() = default;

        virtual tempo_utils::Status enter(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            EnterContext &ctx) = 0;

        virtual tempo_utils::Status exit(
            CompilerScanDriver *driver,
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            lyric_assembler::BlockHandle *block,
            ExitContext &ctx) = 0;
    };

    class AbstractChoice {
    public:
        virtual ~AbstractChoice() = default;

        virtual tempo_utils::Status decide(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            DecideContext &ctx) = 0;

        virtual lyric_assembler::BlockHandle *getBlock() = 0;

        virtual CompilerScanDriver *getDriver() = 0;
    };

    class AbstractGrouping {
    public:
        virtual ~AbstractGrouping() = default;

        virtual tempo_utils::Status before(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            BeforeContext &ctx) = 0;

        virtual tempo_utils::Status after(
            const lyric_parser::ArchetypeState *state,
            const lyric_parser::ArchetypeNode *node,
            AfterContext &ctx) = 0;

        virtual lyric_assembler::BlockHandle *getBlock() = 0;

        virtual CompilerScanDriver *getDriver() = 0;

    private:
        virtual Handler *currentHandler() const = 0;
        virtual void appendHandlers(std::vector<std::unique_ptr<Handler>> &&handlers) = 0;
        virtual bool advanceHandler() = 0;
        virtual bool isFinished() const = 0;

        friend class CompilerScanDriver;
    };

    enum class HandlerType {
        Invalid,
        Behavior,
        Choice,
        Grouping,
    };

    struct Handler {
        HandlerType type;
        std::unique_ptr<AbstractBehavior> behavior;
        std::unique_ptr<AbstractChoice> choice;
        std::unique_ptr<AbstractGrouping> grouping;
    };

    class BeforeContext {
    public:
        BeforeContext(lyric_rewriter::VisitorContext &visitorContext, const CompilerScanDriver *driver);

        const CompilerScanDriver *getDriver() const;
        const lyric_parser::ArchetypeNode *parentNode() const;
        int childIndex() const;

        void setSkipChildren(bool skip);

        void appendBehavior(std::unique_ptr<AbstractBehavior> &&behavior);
        void appendChoice(std::unique_ptr<AbstractChoice> &&choice);
        void appendGrouping(std::unique_ptr<AbstractGrouping> &&grouping);

        std::vector<std::unique_ptr<Handler>>&& takeHandlers();

    private:
        lyric_rewriter::VisitorContext &m_visitorContext;
        const CompilerScanDriver *m_driver;
        std::vector<std::unique_ptr<Handler>> m_handlers;
    };

    class AfterContext {
    public:
        AfterContext(const lyric_rewriter::VisitorContext &visitorContext, const CompilerScanDriver *driver);

        const CompilerScanDriver *getDriver() const;
        const lyric_parser::ArchetypeNode *parentNode() const;
        int childIndex() const;

    private:
        const lyric_rewriter::VisitorContext &m_visitorContext;
        const CompilerScanDriver *m_driver;
    };

    class EnterContext {
    public:
        EnterContext(lyric_rewriter::VisitorContext &visitorContext, const CompilerScanDriver *driver);

        const CompilerScanDriver *getDriver() const;
        const lyric_parser::ArchetypeNode *parentNode() const;
        int childIndex() const;

        void appendBehavior(std::unique_ptr<AbstractBehavior> &&behavior);
        void appendChoice(std::unique_ptr<AbstractChoice> &&choice);
        void appendGrouping(std::unique_ptr<AbstractGrouping> &&grouping);

        std::vector<std::unique_ptr<Handler>>&& takeHandlers();

    private:
        lyric_rewriter::VisitorContext &m_visitorContext;
        const CompilerScanDriver *m_driver;

        std::vector<std::unique_ptr<Handler>> m_handlers;
    };

    class ExitContext {
    public:
        ExitContext(const lyric_rewriter::VisitorContext &visitorContext, const CompilerScanDriver *driver);

        const CompilerScanDriver *getDriver() const;
        const lyric_parser::ArchetypeNode *parentNode() const;
        int childIndex() const;

        void setPopHandler(bool popHandler);
        bool popHandler() const;

    private:
        const lyric_rewriter::VisitorContext &m_visitorContext;
        const CompilerScanDriver *m_driver;
        bool m_popHandler;
    };

    class DecideContext {
    public:
        DecideContext(const lyric_rewriter::VisitorContext &visitorContext, const CompilerScanDriver *driver);

        const CompilerScanDriver *getDriver() const;
        const lyric_parser::ArchetypeNode *parentNode() const;
        int childIndex() const;

        void setBehavior(std::unique_ptr<AbstractBehavior> &&behavior);
        void setChoice(std::unique_ptr<AbstractChoice> &&choice);
        void setGrouping(std::unique_ptr<AbstractGrouping> &&grouping);

        std::unique_ptr<Handler>&& takeHandler();

    private:
        const lyric_rewriter::VisitorContext &m_visitorContext;
        const CompilerScanDriver *m_driver;
        std::unique_ptr<Handler> m_handler;
    };
}

#endif // LYRIC_COMPILER_VISITOR_CONTEXT_H
