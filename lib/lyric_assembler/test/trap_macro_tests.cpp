#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/assembler_attrs.h>
#include <lyric_assembler/internal/trap_macro.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/macro_rewrite_driver.h>
#include <lyric_schema/assembler_schema.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

TEST(TrapMacro, TrapInBlock)
{
    lyric_parser::LyricParser parser({});
    auto sourceUrl = tempo_utils::Url::fromString("/test");
    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        @{
            Trap("FOO_TRAP")
        }
    )", sourceUrl, recorder);

    ASSERT_TRUE(parseResult.isResult());
    auto archetype = parseResult.getResult();

    auto blockNode = archetype.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (0, blockNode.numAttrs());
    ASSERT_EQ (1, blockNode.numChildren());

    auto macroListNode = blockNode.getChild(0);
    ASSERT_TRUE (macroListNode.isClass(lyric_schema::kLyricAstMacroListClass));
    ASSERT_EQ (0, macroListNode.numAttrs());
    ASSERT_EQ (1, macroListNode.numChildren());

    auto macroCallNode = macroListNode.getChild(0);
    ASSERT_TRUE (macroCallNode.isClass(lyric_schema::kLyricAstMacroCallClass));
    ASSERT_EQ (1, macroCallNode.numChildren());

    auto arg0Node = macroCallNode.getChild(0);
    ASSERT_TRUE (arg0Node.isClass(lyric_schema::kLyricAstStringClass));

    std::string literalValue;
    ASSERT_THAT (arg0Node.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("FOO_TRAP", literalValue);

    auto registry = std::make_shared<lyric_rewriter::MacroRegistry>();
    registry->registerMacroName("Trap", []() {
        return std::make_shared<lyric_assembler::internal::TrapMacro>();
    });
    registry->sealRegistry();
    auto builder = std::make_shared<lyric_rewriter::MacroRewriteDriverBuilder>(registry);

    lyric_rewriter::RewriterOptions options;
    lyric_rewriter::LyricRewriter rewriter(options);
    auto rewriteArchetypeResult = rewriter.rewriteArchetype(archetype, sourceUrl, builder, recorder);
    ASSERT_THAT (rewriteArchetypeResult, tempo_test::IsResult());
    auto rewritten = rewriteArchetypeResult.getResult();

    blockNode = rewritten.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (0, blockNode.numAttrs());
    ASSERT_EQ (1, blockNode.numChildren());

    auto trapNode = blockNode.getChild(0);
    ASSERT_TRUE (trapNode.isClass(lyric_schema::kLyricAssemblerTrapClass));
    ASSERT_EQ (1, trapNode.numAttrs());
    ASSERT_EQ (0, trapNode.numChildren());

    std::string trapName;
    ASSERT_THAT (trapNode.parseAttr(lyric_assembler::kLyricAssemblerTrapName, trapName), tempo_test::IsOk());
    ASSERT_EQ ("FOO_TRAP", trapName);
}
