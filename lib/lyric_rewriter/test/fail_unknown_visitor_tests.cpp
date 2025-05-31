#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/fail_unknown_visitor.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/rewriter_result.h>
#include <tempo_test/result_matchers.h>

class TestScanDriver : public lyric_rewriter::AbstractScanDriver {
public:
    TestScanDriver() = default;
    tempo_utils::Status enter(const lyric_parser::ArchetypeState *state, const lyric_parser::ArchetypeNode *node,
        lyric_rewriter::VisitorContext &ctx) override
    {
        return {};
    }
    tempo_utils::Status exit(const lyric_parser::ArchetypeState *state, const lyric_parser::ArchetypeNode *node,
        const lyric_rewriter::VisitorContext &ctx) override
    {
        return {};
    }
    tempo_utils::Status finish() override { return {}; }
};

class TestScanDriverBuilder : public lyric_rewriter::AbstractScanDriverBuilder {
public:
    tempo_utils::Status applyPragma(const lyric_parser::ArchetypeState *state,
        const lyric_parser::ArchetypeNode *node) override {
        TU_UNREACHABLE();
    }
    tempo_utils::Result<std::shared_ptr<lyric_rewriter::AbstractScanDriver>> makeScanDriver() override
    {
        return std::shared_ptr<lyric_rewriter::AbstractScanDriver>(new TestScanDriver());
    }
};

TEST(FailUnknownVisitor, FailsWhenEncounteringUnhandledNode)
{
    lyric_parser::LyricParser parser({});
    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        42
    )", {}, recorder);

    ASSERT_TRUE(parseResult.isResult());
    auto archetype = parseResult.getResult();

    auto blockNode = archetype.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (0, blockNode.numAttrs());
    ASSERT_EQ (1, blockNode.numChildren());

    auto intNode = blockNode.getChild(0);
    ASSERT_TRUE (intNode.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (0, intNode.numChildren());

    lyric_rewriter::RewriterOptions options;
    options.visitorRegistry = std::make_shared<lyric_rewriter::VisitorRegistry>(true);
    options.visitorRegistry->setMakeUnknownVisitorFunc([](auto *n, auto *s) {
        return std::make_shared<lyric_rewriter::FailUnknownVisitor>(s);
    });
    options.visitorRegistry->sealRegistry();

    lyric_rewriter::LyricRewriter rewriter(options);
    auto sourceUrl = tempo_utils::Url::fromString("/test");
    auto builder = std::shared_ptr<lyric_rewriter::AbstractScanDriverBuilder>(new TestScanDriverBuilder());
    auto scanArchetypeResult = rewriter.scanArchetype(archetype, sourceUrl, builder, recorder);

    ASSERT_THAT (scanArchetypeResult, tempo_test::ContainsStatus(lyric_rewriter::RewriterCondition::kSyntaxError));
}
