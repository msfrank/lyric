#ifndef LYRIC_COMPILER_VISITOR_CONTEXT_H
#define LYRIC_COMPILER_VISITOR_CONTEXT_H

#include <lyric_rewriter/rewrite_processor.h>

namespace lyric_compiler {

    // forward declarations
    class AbstractBehavior;
    class BaseChoice;
    class BaseGrouping;
    class CompilerScanDriver;

    enum class HandlerType {
        Invalid,
        Behavior,
        Choice,
        Grouping,
    };

    struct Handler {
        HandlerType type;
        std::unique_ptr<AbstractBehavior> behavior;
        std::unique_ptr<BaseChoice> choice;
        std::unique_ptr<BaseGrouping> grouping;
    };

    class BeforeContext {
    public:
        BeforeContext(lyric_rewriter::VisitorContext &visitorContext, const CompilerScanDriver *driver);

        const CompilerScanDriver *getDriver() const;
        const lyric_parser::ArchetypeNode *parentNode() const;
        int childIndex() const;

        void setSkipChildren(bool skip);

        void appendBehavior(std::unique_ptr<AbstractBehavior> &&behavior);
        void appendChoice(std::unique_ptr<BaseChoice> &&choice);
        void appendGrouping(std::unique_ptr<BaseGrouping> &&grouping);

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
        void appendChoice(std::unique_ptr<BaseChoice> &&choice);
        void appendGrouping(std::unique_ptr<BaseGrouping> &&grouping);

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
        void setChoice(std::unique_ptr<BaseChoice> &&choice);
        void setGrouping(std::unique_ptr<BaseGrouping> &&grouping);

        std::unique_ptr<Handler>&& takeHandler();

    private:
        const lyric_rewriter::VisitorContext &m_visitorContext;
        const CompilerScanDriver *m_driver;
        std::unique_ptr<Handler> m_handler;
    };
}

#endif // LYRIC_COMPILER_VISITOR_CONTEXT_H
