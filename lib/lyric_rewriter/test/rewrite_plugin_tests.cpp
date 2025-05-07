#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/macro_rewrite_driver.h>
#include <lyric_rewriter/plugin_macro.h>
#include <lyric_schema/assembler_schema.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/logging.h>

TEST(RewritePlugin, PluginPragma)
{
    lyric_parser::LyricParser parser({});
    auto recorder = tempo_tracing::TraceRecorder::create();

    auto parseResult = parser.parseModule(R"(
        @@Plugin("/plugin")

        nil
    )", {}, recorder);

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

    lyric_rewriter::RewriterOptions options;
    auto loader = std::make_shared<lyric_bootstrap::BootstrapLoader>(LYRIC_BUILD_BOOTSTRAP_DIR);
    options.systemModuleCache = lyric_importer::ModuleCache::create(loader);

    lyric_rewriter::LyricRewriter rewriter(options);

    lyric_rewriter::MacroRegistry registry({
        {"Plugin", std::make_shared<lyric_rewriter::PluginMacro>()}
    });
    auto builder = std::make_shared<lyric_rewriter::MacroRewriteDriverBuilder>(&registry);

    auto sourceUrl = tempo_utils::Url::fromString("/test");
    auto rewriteArchetypeResult = rewriter.rewriteArchetype(archetype, sourceUrl, builder, recorder);
    ASSERT_TRUE (rewriteArchetypeResult.isResult());
    auto rewritten = rewriteArchetypeResult.getResult();

    ASSERT_EQ (1, rewritten.numPragmas());

    pragmaNode = rewritten.getPragma(0);
    ASSERT_TRUE (pragmaNode.isClass(lyric_schema::kLyricAssemblerPluginClass));
    ASSERT_EQ (0, pragmaNode.numChildren());

    lyric_common::ModuleLocation pluginLocation;
    ASSERT_THAT (pragmaNode.parseAttr(lyric_parser::kLyricAstModuleLocation, pluginLocation), tempo_test::IsOk());
    ASSERT_EQ ("/plugin", pluginLocation.toString());
}
