#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/macro_rewrite_driver.h>
#include <lyric_rewriter/trap_macro.h>
#include <lyric_schema/assembler_schema.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/logging.h>
#include "lyric_rewriter/assembler_attrs.h"

TEST(RewriteTrap, TrapInBlock)
{
    lyric_parser::LyricParser parser({});
    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        @{
            Trap(0)
        }
    )", {}, recorder);

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
    ASSERT_TRUE (arg0Node.isClass(lyric_schema::kLyricAstIntegerClass));

    std::string literalValue;
    ASSERT_THAT (arg0Node.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("0", literalValue);

    lyric_rewriter::RewriterOptions options;
    auto loader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
    options.systemModuleCache = lyric_importer::ModuleCache::create(loader);

    lyric_rewriter::LyricRewriter rewriter(options);

    lyric_rewriter::MacroRegistry registry({
        {"Trap", std::make_shared<lyric_rewriter::TrapMacro>()}
    });
    auto driver = std::make_shared<lyric_rewriter::MacroRewriteDriver>(&registry);

    auto sourceUrl = tempo_utils::Url::fromString("/test");
    auto rewriteArchetypeResult = rewriter.rewriteArchetype(archetype, sourceUrl, driver, recorder);
    ASSERT_TRUE (rewriteArchetypeResult.isResult());
    auto rewritten = rewriteArchetypeResult.getResult();

    blockNode = rewritten.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (0, blockNode.numAttrs());
    ASSERT_EQ (1, blockNode.numChildren());

    auto trapNode = blockNode.getChild(0);
    ASSERT_TRUE (trapNode.isClass(lyric_schema::kLyricAssemblerTrapClass));
    ASSERT_EQ (1, trapNode.numAttrs());
    ASSERT_EQ (0, trapNode.numChildren());

    tu_uint32 trapNumber;
    ASSERT_THAT (trapNode.parseAttr(lyric_rewriter::kLyricAssemblerTrapNumber, trapNumber), tempo_test::IsOk());
    ASSERT_EQ (0, trapNumber);
}
