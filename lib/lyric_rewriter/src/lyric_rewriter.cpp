
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/node_walker.h>
#include <lyric_rewriter/abstract_rewrite_driver.h>
#include <lyric_rewriter/lyric_ast_sequence_visitor.h>
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

struct RewriteDriverState {
    std::shared_ptr<lyric_rewriter::AbstractRewriteDriver> driver;
    lyric_parser::ArchetypeState *state;
};

static tempo_utils::Status
rewrite_arrange(
    lyric_schema::LyricAstId astId,
    lyric_parser::ArchetypeNode *node,
    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> &children,
    void *data)
{
    auto *driverState = static_cast<RewriteDriverState *>(data);
    return driverState->driver->arrange(driverState->state, node, children);
}

static tempo_utils::Status
rewrite_enter_node(
    lyric_schema::LyricAstId astId,
    lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx,
    void *data)
{
    auto *driverState = static_cast<RewriteDriverState *>(data);
    return driverState->driver->enter(driverState->state, node, ctx);
}

static tempo_utils::Status
rewrite_exit_node(
    lyric_schema::LyricAstId astId,
    lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx,
    void *data)
{
    auto *driverState = static_cast<RewriteDriverState *>(data);
    return driverState->driver->exit(driverState->state, node, ctx);
}

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
        RewriteDriverState driverState;
        driverState.driver = rewriteDriver;
        driverState.state = &archetypeState;

        // construct the root visitor
        LyricAstOptions options;
        options.arrange = rewrite_arrange;
        options.enter = rewrite_enter_node;
        options.exit = rewrite_exit_node;
        options.data = &driverState;
        options.unknown = m_options.unknownVisitor;

        auto visitor = std::make_shared<LyricAstSequenceVisitor>(
            lyric_schema::LyricAstId::Block, &options);

        // rewrite archetype
        RewriteProcessor processor;
        TU_RETURN_IF_NOT_OK (processor.process(&archetypeState, visitor));

        // serialize state
        lyric_parser::LyricArchetype rewritten;
        TU_ASSIGN_OR_RETURN (rewritten, archetypeState.toArchetype());

        TU_ASSERT (rewritten.isValid());
        return rewritten;

    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    }
}

struct ScanDriverState {
    std::shared_ptr<lyric_rewriter::AbstractScanDriver> driver;
    lyric_parser::ArchetypeState *state;
};

static tempo_utils::Status
scan_arrange(
    lyric_schema::LyricAstId astId,
    lyric_parser::ArchetypeNode *node,
    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> &children,
    void *data)
{
    auto *driverState = static_cast<ScanDriverState *>(data);
    return driverState->driver->arrange(driverState->state, node, children);
}

static tempo_utils::Status
scan_enter_node(
    lyric_schema::LyricAstId astId,
    lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx,
    void *data)
{
    auto *driverState = static_cast<ScanDriverState *>(data);
    return driverState->driver->enter(driverState->state, node, ctx);
}

static tempo_utils::Status
scan_exit_node(
    lyric_schema::LyricAstId astId,
    lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx,
    void *data)
{
    auto *driverState = static_cast<ScanDriverState *>(data);
    return driverState->driver->exit(driverState->state, node, ctx);
}

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
        ScanDriverState driverState;
        driverState.driver = scanDriver;
        driverState.state = &archetypeState;

        // construct the root visitor
        LyricAstOptions options;
        options.arrange = scan_arrange;
        options.enter = scan_enter_node;
        options.exit = scan_exit_node;
        options.data = &driverState;
        options.unknown = m_options.unknownVisitor;

        auto visitor = std::make_shared<LyricAstSequenceVisitor>(
            lyric_schema::LyricAstId::Block, &options);

        // scan archetype
        RewriteProcessor processor;
        return processor.process(&archetypeState, visitor);

    } catch (tempo_utils::StatusException &ex) {
        return ex.getStatus();
    }
}
