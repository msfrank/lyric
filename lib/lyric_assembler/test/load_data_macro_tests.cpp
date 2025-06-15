#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/internal/load_data_macro.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/macro_rewrite_driver.h>
#include <lyric_schema/assembler_schema.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

class LoadDataMacro : public ::testing::Test {
protected:
    lyric_parser::LyricArchetype archetype;
    lyric_parser::LyricArchetype rewritten;
    void parseAndRewrite(std::string_view utf8)
    {
        lyric_parser::LyricParser parser({});
        auto sourceUrl = tempo_utils::Url::fromString("/test");
        auto recorder = tempo_tracing::TraceRecorder::create();

        auto parseResult = parser.parseModule(utf8, sourceUrl, recorder);

        TU_ASSIGN_OR_RAISE (archetype, parseResult);

        auto registry = std::make_shared<lyric_rewriter::MacroRegistry>();
        registry->registerMacroName("LoadData", []() {
            return std::make_shared<lyric_assembler::internal::LoadDataMacro>();
        });
        registry->sealRegistry();
        auto builder = std::make_shared<lyric_rewriter::MacroRewriteDriverBuilder>(registry);

        lyric_rewriter::RewriterOptions options;
        lyric_rewriter::LyricRewriter rewriter(options);
        auto rewriteResult = rewriter.rewriteArchetype(archetype, sourceUrl, builder, recorder);

        TU_ASSIGN_OR_RAISE (rewritten, rewriteResult);
    }
};

TEST_F(LoadDataMacro, LoadUndef)
{
    parseAndRewrite(R"(
        @{
            LoadData(undef)
        }
    )");

    auto blockNode = rewritten.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (0, blockNode.numAttrs());
    ASSERT_EQ (1, blockNode.numChildren());

    auto loadDataNode = blockNode.getChild(0);
    ASSERT_TRUE (loadDataNode.isClass(lyric_schema::kLyricAssemblerLoadDataClass));
    ASSERT_EQ (0, loadDataNode.numAttrs());
    ASSERT_EQ (1, loadDataNode.numChildren());

    auto literalNode = loadDataNode.getChild(0);
    ASSERT_TRUE (literalNode.isClass(lyric_schema::kLyricAstUndefClass));
    ASSERT_EQ (0, literalNode.numAttrs());
    ASSERT_EQ (0, literalNode.numChildren());
}

TEST_F(LoadDataMacro, LoadLiteralBool)
{
    parseAndRewrite(R"(
        @{
            LoadData(true)
        }
    )");

    auto blockNode = rewritten.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (0, blockNode.numAttrs());
    ASSERT_EQ (1, blockNode.numChildren());

    auto loadDataNode = blockNode.getChild(0);
    ASSERT_TRUE (loadDataNode.isClass(lyric_schema::kLyricAssemblerLoadDataClass));
    ASSERT_EQ (0, loadDataNode.numAttrs());
    ASSERT_EQ (1, loadDataNode.numChildren());

    auto literalNode = loadDataNode.getChild(0);
    ASSERT_TRUE (literalNode.isClass(lyric_schema::kLyricAstTrueClass));
    ASSERT_EQ (0, literalNode.numAttrs());
    ASSERT_EQ (0, literalNode.numChildren());
}

TEST_F(LoadDataMacro, LoadLiteralInt)
{
    parseAndRewrite(R"(
        @{
            LoadData(42)
        }
    )");

    auto blockNode = rewritten.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (0, blockNode.numAttrs());
    ASSERT_EQ (1, blockNode.numChildren());

    auto loadDataNode = blockNode.getChild(0);
    ASSERT_TRUE (loadDataNode.isClass(lyric_schema::kLyricAssemblerLoadDataClass));
    ASSERT_EQ (0, loadDataNode.numAttrs());
    ASSERT_EQ (1, loadDataNode.numChildren());

    auto literalNode = loadDataNode.getChild(0);
    ASSERT_TRUE (literalNode.isClass(lyric_schema::kLyricAstIntegerClass));
    ASSERT_EQ (3, literalNode.numAttrs());
    ASSERT_EQ (0, literalNode.numChildren());

    std::string literalValue;
    ASSERT_THAT (literalNode.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("42", literalValue);
}

TEST_F(LoadDataMacro, LoadLiteralFloat)
{
    parseAndRewrite(R"(
        @{
            LoadData(3.14159)
        }
    )");

    auto blockNode = rewritten.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (0, blockNode.numAttrs());
    ASSERT_EQ (1, blockNode.numChildren());

    auto loadDataNode = blockNode.getChild(0);
    ASSERT_TRUE (loadDataNode.isClass(lyric_schema::kLyricAssemblerLoadDataClass));
    ASSERT_EQ (0, loadDataNode.numAttrs());
    ASSERT_EQ (1, loadDataNode.numChildren());

    auto literalNode = loadDataNode.getChild(0);
    ASSERT_TRUE (literalNode.isClass(lyric_schema::kLyricAstFloatClass));
    ASSERT_EQ (3, literalNode.numAttrs());
    ASSERT_EQ (0, literalNode.numChildren());

    std::string literalValue;
    ASSERT_THAT (literalNode.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("3.14159", literalValue);
}

TEST_F(LoadDataMacro, LoadLiteralString)
{
    parseAndRewrite(R"(
        @{
            LoadData("hello, world")
        }
    )");

    auto blockNode = rewritten.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (0, blockNode.numAttrs());
    ASSERT_EQ (1, blockNode.numChildren());

    auto loadDataNode = blockNode.getChild(0);
    ASSERT_TRUE (loadDataNode.isClass(lyric_schema::kLyricAssemblerLoadDataClass));
    ASSERT_EQ (0, loadDataNode.numAttrs());
    ASSERT_EQ (1, loadDataNode.numChildren());

    auto literalNode = loadDataNode.getChild(0);
    ASSERT_TRUE (literalNode.isClass(lyric_schema::kLyricAstStringClass));
    ASSERT_EQ (1, literalNode.numAttrs());
    ASSERT_EQ (0, literalNode.numChildren());

    std::string literalValue;
    ASSERT_THAT (literalNode.parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue), tempo_test::IsOk());
    ASSERT_EQ ("hello, world", literalValue);
}

TEST_F(LoadDataMacro, LoadName)
{
    parseAndRewrite(R"(
        @{
            LoadData(local)
        }
    )");

    auto blockNode = rewritten.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (0, blockNode.numAttrs());
    ASSERT_EQ (1, blockNode.numChildren());

    auto loadDataNode = blockNode.getChild(0);
    ASSERT_TRUE (loadDataNode.isClass(lyric_schema::kLyricAssemblerLoadDataClass));
    ASSERT_EQ (0, loadDataNode.numAttrs());
    ASSERT_EQ (1, loadDataNode.numChildren());

    auto dataDerefNode = loadDataNode.getChild(0);
    ASSERT_TRUE (dataDerefNode.isClass(lyric_schema::kLyricAstDataDerefClass));
    ASSERT_EQ (0, dataDerefNode.numAttrs());
    ASSERT_EQ (1, dataDerefNode.numChildren());

    auto name1Node = dataDerefNode.getChild(0);
    ASSERT_TRUE (name1Node.isClass(lyric_schema::kLyricAstNameClass));
    std::string identifier1;
    ASSERT_THAT (name1Node.parseAttr(lyric_parser::kLyricAstIdentifier, identifier1), tempo_test::IsOk());
    ASSERT_EQ ("local", identifier1);
}

TEST_F(LoadDataMacro, LoadSymbolDescriptor)
{
    parseAndRewrite(R"(
        @{
            LoadData(#Equality.Equals)
        }
    )");

    auto blockNode = rewritten.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (0, blockNode.numAttrs());
    ASSERT_EQ (1, blockNode.numChildren());

    auto loadDataNode = blockNode.getChild(0);
    ASSERT_TRUE (loadDataNode.isClass(lyric_schema::kLyricAssemblerLoadDataClass));
    ASSERT_EQ (0, loadDataNode.numAttrs());
    ASSERT_EQ (1, loadDataNode.numChildren());

    auto symbolDerefNode = loadDataNode.getChild(0);
    ASSERT_TRUE (symbolDerefNode.isClass(lyric_schema::kLyricAstSymbolDerefClass));
    ASSERT_EQ (0, symbolDerefNode.numAttrs());
    ASSERT_EQ (2, symbolDerefNode.numChildren());

    auto name1Node = symbolDerefNode.getChild(0);
    ASSERT_TRUE (name1Node.isClass(lyric_schema::kLyricAstNameClass));
    std::string identifier1;
    ASSERT_THAT (name1Node.parseAttr(lyric_parser::kLyricAstIdentifier, identifier1), tempo_test::IsOk());
    ASSERT_EQ ("Equality", identifier1);

    auto name2Node = symbolDerefNode.getChild(1);
    ASSERT_TRUE (name2Node.isClass(lyric_schema::kLyricAstNameClass));
    std::string identifier2;
    ASSERT_THAT (name2Node.parseAttr(lyric_parser::kLyricAstIdentifier, identifier2), tempo_test::IsOk());
    ASSERT_EQ ("Equals", identifier2);
}
