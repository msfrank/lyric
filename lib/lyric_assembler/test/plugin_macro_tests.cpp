#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/internal/plugin_macro.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/macro_rewrite_driver.h>
#include <lyric_schema/assembler_schema.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

TEST(PluginMacro, PluginPragma)
{
    lyric_parser::LyricParser parser({});
    auto sourceUrl = tempo_utils::Url::fromString("/test");
    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        @@Plugin("/plugin")

        nil
    )", sourceUrl, recorder);

    ASSERT_TRUE(parseResult.isResult());
    auto archetype = parseResult.getResult();

    ASSERT_EQ (1, archetype.numPragmas());

    auto pragmaNode = archetype.getPragma(0);
    ASSERT_TRUE (pragmaNode.isClass(lyric_schema::kLyricAstPragmaClass));
    ASSERT_EQ (1, pragmaNode.numChildren());

    auto arg0Node = pragmaNode.getChild(0);
    std::string literalValue;
    ASSERT_THAT (arg0Node.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("/plugin", literalValue);


    auto registry = std::make_shared<lyric_rewriter::MacroRegistry>();
    registry->registerMacroName("Plugin", []() {
        return std::make_shared<lyric_assembler::internal::PluginMacro>();
    });
    registry->sealRegistry();
    auto builder = std::make_shared<lyric_rewriter::MacroRewriteDriverBuilder>(registry);

    lyric_rewriter::RewriterOptions options;
    lyric_rewriter::LyricRewriter rewriter(options);
    auto rewriteArchetypeResult = rewriter.rewriteArchetype(archetype, sourceUrl, builder, recorder);
    ASSERT_THAT (rewriteArchetypeResult, tempo_test::IsResult());
    auto rewritten = rewriteArchetypeResult.getResult();

    ASSERT_EQ (1, rewritten.numPragmas());

    pragmaNode = rewritten.getPragma(0);
    ASSERT_TRUE (pragmaNode.isClass(lyric_schema::kLyricAssemblerPluginClass));
    ASSERT_EQ (0, pragmaNode.numChildren());

    lyric_common::ModuleLocation pluginLocation;
    ASSERT_THAT (pragmaNode.parseAttr(lyric_parser::kLyricAstModuleLocation, pluginLocation), tempo_test::IsOk());
    ASSERT_EQ ("/plugin", pluginLocation.toString());
}
