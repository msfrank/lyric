
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/node_walker.h>
#include <lyric_rewriter/abstract_rewrite_driver.h>
#include <lyric_rewriter/ast_sequence_visitor.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/rewriter_result.h>
#include <lyric_rewriter/rewrite_processor.h>
#include <tempo_utils/log_stream.h>

lyric_rewriter::LyricRewriter::LyricRewriter(const RewriterOptions &options)
    : m_options(options)
{
}

lyric_rewriter::LyricRewriter::LyricRewriter(const LyricRewriter &other)
    : m_options(other.m_options)
{
}

class RewriteProcessorState : public lyric_rewriter::AbstractProcessorState {
public:
    RewriteProcessorState(
        const lyric_rewriter::RewriterOptions *options,
        std::shared_ptr<lyric_rewriter::AbstractRewriteDriver> driver,
        lyric_parser::ArchetypeState *state)
        : m_options(options),
          m_driver(driver),
          m_state(state)
    {
    };
    tempo_utils::Status
    enterNode(lyric_parser::ArchetypeNode *node, lyric_rewriter::VisitorContext &ctx) override
    {
        return m_driver->enter(m_state, node, ctx);
    };
    tempo_utils::Status
    exitNode(lyric_parser::ArchetypeNode *node, const lyric_rewriter::VisitorContext &ctx) override
    {
        return m_driver->exit(m_state, node, ctx);
    };
    tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>>
    makeUnknownVisitor(const lyric_parser::ArchetypeNode *node) override
    {
        if (m_options->makeUnknownVisitor != nullptr)
            return m_options->makeUnknownVisitor(node);
        return lyric_rewriter::RewriterStatus::forCondition(
            lyric_rewriter::RewriterCondition::kRewriterInvariant, "unknown node");
    };

private:
    const lyric_rewriter::RewriterOptions *m_options;
    std::shared_ptr<lyric_rewriter::AbstractRewriteDriver> m_driver;
    lyric_parser::ArchetypeState *m_state;
};

tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_rewriter::LyricRewriter::rewriteArchetype(
    const lyric_parser::LyricArchetype &archetype,
    const tempo_utils::Url &sourceUrl,
    std::shared_ptr<AbstractRewriteDriver> rewriteDriver,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    if (!sourceUrl.isValid())
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "invalid source url");

    if (!archetype.isValid())
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "invalid archetype");

    try {

        // create a new span
        tempo_tracing::ScopeManager scopeManager(recorder);
        auto span = scopeManager.makeSpan();
        span->setOperationName("rewriteArchetype");

        lyric_parser::ArchetypeState archetypeState(sourceUrl, &scopeManager);

        lyric_parser::ArchetypeNode *root;
        TU_ASSIGN_OR_RETURN (root, archetypeState.load(archetype));
        archetypeState.setRoot(root);

        // define the driver state
        RewriteProcessorState processorState(&m_options, rewriteDriver, &archetypeState);

        // construct the root visitor
        auto visitor = std::make_shared<AstSequenceVisitor>(
            lyric_schema::LyricAstId::Block, &processorState);

        // rewrite archetype
        RewriteProcessor processor;
        TU_RETURN_IF_NOT_OK (processor.process(&archetypeState, visitor));
        TU_RETURN_IF_NOT_OK (rewriteDriver->finish());

        // serialize state
        lyric_parser::LyricArchetype rewritten;
        TU_ASSIGN_OR_RETURN (rewritten, archetypeState.toArchetype());

        TU_ASSERT (rewritten.isValid());
        return rewritten;

    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    }
}

class ScanProcessorState : public lyric_rewriter::AbstractProcessorState {
public:
    ScanProcessorState(
        const lyric_rewriter::RewriterOptions *options,
        std::shared_ptr<lyric_rewriter::AbstractScanDriver> driver,
        lyric_parser::ArchetypeState *state)
        : m_options(options),
          m_driver(driver),
          m_state(state)
    {
    };
    tempo_utils::Status
    enterNode(lyric_parser::ArchetypeNode *node, lyric_rewriter::VisitorContext &ctx) override
    {
        return m_driver->enter(m_state, node, ctx);
    };
    tempo_utils::Status
    exitNode(lyric_parser::ArchetypeNode *node, const lyric_rewriter::VisitorContext &ctx) override
    {
        return m_driver->exit(m_state, node, ctx);
    };
    tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>>
    makeUnknownVisitor(const lyric_parser::ArchetypeNode *node) override
    {
        if (m_options->makeUnknownVisitor != nullptr)
            return m_options->makeUnknownVisitor(node);
        return lyric_rewriter::RewriterStatus::forCondition(
            lyric_rewriter::RewriterCondition::kRewriterInvariant, "unknown node");
    };
private:
    const lyric_rewriter::RewriterOptions *m_options;
    std::shared_ptr<lyric_rewriter::AbstractScanDriver> m_driver;
    lyric_parser::ArchetypeState *m_state;
};

tempo_utils::Status
lyric_rewriter::LyricRewriter::scanArchetype(
    const lyric_parser::LyricArchetype &archetype,
    const tempo_utils::Url &sourceUrl,
    std::shared_ptr<AbstractScanDriver> scanDriver,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    if (!sourceUrl.isValid())
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "invalid source url");

    if (!archetype.isValid())
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "invalid archetype");

    try {

        // create a new span
        tempo_tracing::ScopeManager scopeManager(recorder);
        auto span = scopeManager.makeSpan();
        span->setOperationName("rewriteArchetype");

        lyric_parser::ArchetypeState archetypeState(sourceUrl, &scopeManager);

        lyric_parser::ArchetypeNode *root;
        TU_ASSIGN_OR_RETURN (root, archetypeState.load(archetype));
        archetypeState.setRoot(root);

        // define the driver state
        ScanProcessorState processorState(&m_options, scanDriver, &archetypeState);

        // construct the root visitor
        auto visitor = std::make_shared<AstSequenceVisitor>(
            lyric_schema::LyricAstId::Block, &processorState);

        // scan archetype
        RewriteProcessor processor;
        TU_RETURN_IF_NOT_OK (processor.process(&archetypeState, visitor));
        return scanDriver->finish();

    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    }
}
