#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/assembler_attrs.h>
#include <lyric_assembler/internal/opcode_macro.h>
#include <lyric_parser/lyric_parser.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_rewriter/lyric_rewriter.h>
#include <lyric_rewriter/macro_rewrite_driver.h>
#include <lyric_schema/assembler_schema.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

class OpcodeMacro : public ::testing::Test {
public:
    OpcodeMacro(): m_macroRegistry(std::make_shared<lyric_rewriter::MacroRegistry>()) {}
protected:
    lyric_parser::LyricArchetype archetype;
    lyric_parser::LyricArchetype rewritten;

    void registerMacro(std::string_view macroName, std::shared_ptr<lyric_rewriter::AbstractMacro> macro)
    {
        TU_RAISE_IF_NOT_OK (m_macroRegistry->registerMacroName(std::string(macroName), [=]() {
            return macro;
        }));
    }
    void parseAndRewrite(std::string_view utf8)
    {
        lyric_parser::LyricParser parser({});
        auto sourceUrl = tempo_utils::Url::fromString("/test");
        auto recorder = tempo_tracing::TraceRecorder::create();

        auto parseResult = parser.parseModule(utf8, sourceUrl, recorder);

        TU_ASSIGN_OR_RAISE (archetype, parseResult);

        m_macroRegistry->sealRegistry();
        auto builder = std::make_shared<lyric_rewriter::MacroRewriteDriverBuilder>(m_macroRegistry);

        lyric_rewriter::RewriterOptions options;
        lyric_rewriter::LyricRewriter rewriter(options);
        auto rewriteResult = rewriter.rewriteArchetype(archetype, sourceUrl, builder, recorder);

        TU_ASSIGN_OR_RAISE (rewritten, rewriteResult);
    }
private:
    std::shared_ptr<lyric_rewriter::MacroRegistry> m_macroRegistry;
};

TEST_F(OpcodeMacro, EvaluateI64AddMacro)
{
    registerMacro("I64Add", std::make_shared<lyric_assembler::internal::I64AddMacro>());
    parseAndRewrite(R"(
        @{
            I64Add()
        }
    )");

    auto blockNode = rewritten.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (0, blockNode.numAttrs());
    ASSERT_EQ (1, blockNode.numChildren());

    auto opNode = blockNode.getChild(0);
    ASSERT_TRUE (opNode.isClass(lyric_schema::kLyricAssemblerOpClass));
    ASSERT_EQ (1, opNode.numAttrs());
    ASSERT_EQ (0, opNode.numChildren());

    lyric_object::Opcode opcode;
    opNode.parseAttr(lyric_assembler::kLyricAssemblerOpcodeEnum, opcode);
    ASSERT_EQ (lyric_object::Opcode::OP_I64_ADD, opcode);
}

TEST_F(OpcodeMacro, EvaluatePickMacro)
{
    registerMacro("Pick", std::make_shared<lyric_assembler::internal::PickMacro>());
    parseAndRewrite(R"(
        @{
            Pick(3)
        }
    )");

    auto blockNode = rewritten.getRoot();
    ASSERT_TRUE (blockNode.isClass(lyric_schema::kLyricAstBlockClass));
    ASSERT_EQ (0, blockNode.numAttrs());
    ASSERT_EQ (1, blockNode.numChildren());

    auto opNode = blockNode.getChild(0);
    ASSERT_TRUE (opNode.isClass(lyric_schema::kLyricAssemblerOpClass));
    ASSERT_EQ (2, opNode.numAttrs());
    ASSERT_EQ (0, opNode.numChildren());

    lyric_object::Opcode opcode;
    opNode.parseAttr(lyric_assembler::kLyricAssemblerOpcodeEnum, opcode);
    ASSERT_EQ (lyric_object::Opcode::OP_PICK, opcode);

    tu_uint16 stackOffset;
    opNode.parseAttr(lyric_assembler::kLyricAssemblerStackOffset, stackOffset);
    ASSERT_EQ (3, stackOffset);
}
