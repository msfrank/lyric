
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/node_walker.h>
#include <lyric_rewriter/abstract_rewrite_driver.h>
#include <lyric_rewriter/ast_sequence_visitor.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/rewriter_result.h>
#include <lyric_rewriter/rewrite_processor.h>
#include <tempo_utils/log_stream.h>

#include "lyric_rewriter/pragma_rewriter.h"

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
        std::shared_ptr<lyric_rewriter::VisitorRegistry> registry,
        std::shared_ptr<lyric_rewriter::AbstractRewriteDriver> driver,
        lyric_parser::ArchetypeState *state)
        : m_registry(std::move(registry)),
          m_driver(driver),
          m_state(state)
    {
        TU_ASSERT (m_registry != nullptr);
        TU_ASSERT (m_driver != nullptr);
        TU_ASSERT (m_state != nullptr);
    }
    tempo_utils::Status
    enterNode(lyric_parser::ArchetypeNode *node, lyric_rewriter::VisitorContext &ctx) override
    {
        return m_driver->enter(m_state, node, ctx);
    }
    tempo_utils::Status
    exitNode(lyric_parser::ArchetypeNode *node, const lyric_rewriter::VisitorContext &ctx) override
    {
        return m_driver->exit(m_state, node, ctx);
    }
    tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>>
    makeVisitor(const lyric_parser::ArchetypeNode *node) override
    {
        return m_registry->makeVisitor(node, this);
    };

private:
    std::shared_ptr<lyric_rewriter::VisitorRegistry> m_registry;
    std::shared_ptr<lyric_rewriter::AbstractRewriteDriver> m_driver;
    lyric_parser::ArchetypeState *m_state;
};

tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_rewriter::LyricRewriter::rewriteArchetype(
    const lyric_parser::LyricArchetype &archetype,
    const tempo_utils::Url &sourceUrl,
    std::shared_ptr<AbstractRewriteDriverBuilder> rewriteDriverBuilder,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    if (!sourceUrl.isValid())
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "invalid source url");

    if (!archetype.isValid())
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "invalid archetype");

    try {
        RewriteProcessor processor;

        // create a new span
        tempo_tracing::ScopeManager scopeManager(recorder);
        auto span = scopeManager.makeSpan();
        span->setOperationName("rewriteArchetype");

        lyric_parser::ArchetypeState archetypeState(sourceUrl, &scopeManager);

        // load the archetype state
        lyric_parser::ArchetypeNode *root;
        TU_ASSIGN_OR_RETURN (root, archetypeState.load(archetype));
        archetypeState.setRoot(root);

        // construct the visitor registry if one was not specified
        std::shared_ptr<VisitorRegistry> visitorRegistry;
        if (m_options.visitorRegistry != nullptr) {
            visitorRegistry = m_options.visitorRegistry;
        } else {
            visitorRegistry = std::make_shared<VisitorRegistry>();
        }
        visitorRegistry->sealRegistry();

        // preprocess the archetype
        PragmaRewriter pragmaRewriter(rewriteDriverBuilder, &archetypeState);
        TU_RETURN_IF_NOT_OK (pragmaRewriter.rewritePragmas());

        // construct the driver
        std::shared_ptr<AbstractRewriteDriver> rewriteDriver;
        TU_ASSIGN_OR_RETURN (rewriteDriver, rewriteDriverBuilder->makeRewriteDriver());

        // define the driver state
        RewriteProcessorState processorState(visitorRegistry, rewriteDriver, &archetypeState);

        // construct the root visitor
        auto rootVisitor = std::make_shared<AstSequenceVisitor>(
            lyric_schema::LyricAstId::Block, &processorState);

        // rewrite archetype
        TU_RETURN_IF_NOT_OK (processor.process(&archetypeState, rootVisitor));
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
        std::shared_ptr<lyric_rewriter::VisitorRegistry> registry,
        std::shared_ptr<lyric_rewriter::AbstractScanDriver> driver,
        lyric_parser::ArchetypeState *state)
        : m_registry(registry),
          m_driver(driver),
          m_state(state)
    {
        TU_ASSERT (m_registry != nullptr);
        TU_ASSERT (m_driver != nullptr);
        TU_ASSERT (m_state != nullptr);
    }
    tempo_utils::Status
    enterNode(lyric_parser::ArchetypeNode *node, lyric_rewriter::VisitorContext &ctx) override
    {
        return m_driver->enter(m_state, node, ctx);
    }
    tempo_utils::Status
    exitNode(lyric_parser::ArchetypeNode *node, const lyric_rewriter::VisitorContext &ctx) override
    {
        return m_driver->exit(m_state, node, ctx);
    }
    tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>>
    makeVisitor(const lyric_parser::ArchetypeNode *node) override
    {
        return m_registry->makeVisitor(node, this);
    }
private:
    std::shared_ptr<lyric_rewriter::VisitorRegistry> m_registry;
    std::shared_ptr<lyric_rewriter::AbstractScanDriver> m_driver;
    lyric_parser::ArchetypeState *m_state;
};

tempo_utils::Status
lyric_rewriter::LyricRewriter::scanArchetype(
    const lyric_parser::LyricArchetype &archetype,
    const tempo_utils::Url &sourceUrl,
    std::shared_ptr<AbstractScanDriverBuilder> scanDriverBuilder,
    std::shared_ptr<tempo_tracing::TraceRecorder> recorder)
{
    if (!sourceUrl.isValid())
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "invalid source url");

    if (!archetype.isValid())
        return RewriterStatus::forCondition(
            RewriterCondition::kRewriterInvariant, "invalid archetype");

    try {
        RewriteProcessor processor;

        // create a new span
        tempo_tracing::ScopeManager scopeManager(recorder);
        auto span = scopeManager.makeSpan();
        span->setOperationName("rewriteArchetype");

        lyric_parser::ArchetypeState archetypeState(sourceUrl, &scopeManager);

        // load the archetype state
        lyric_parser::ArchetypeNode *root;
        TU_ASSIGN_OR_RETURN (root, archetypeState.load(archetype));
        archetypeState.setRoot(root);

        // construct the visitor registry if one was not specified
        std::shared_ptr<VisitorRegistry> visitorRegistry;
        if (m_options.visitorRegistry != nullptr) {
            visitorRegistry = m_options.visitorRegistry;
        } else {
            visitorRegistry = std::make_shared<VisitorRegistry>();
        }
        visitorRegistry->sealRegistry();

        // preprocess the archetype
        for (auto it = archetypeState.pragmasBegin(); it != archetypeState.pragmasEnd(); it++) {
            TU_RETURN_IF_NOT_OK (scanDriverBuilder->applyPragma(&archetypeState, *it));
        }

        // construct the driver
        std::shared_ptr<AbstractScanDriver> scanDriver;
        TU_ASSIGN_OR_RETURN (scanDriver, scanDriverBuilder->makeScanDriver());

        // define the driver state
        ScanProcessorState processorState(visitorRegistry, scanDriver, &archetypeState);

        // construct the root visitor
        auto rootVisitor = std::make_shared<AstSequenceVisitor>(
            lyric_schema::LyricAstId::Block, &processorState);

        // scan archetype
        TU_RETURN_IF_NOT_OK (processor.process(&archetypeState, rootVisitor));
        return scanDriver->finish();

    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    }
}
