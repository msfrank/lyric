
#include <lyric_assembler/assembler_attrs.h>
#include <lyric_assembler/internal/opcode_macro.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/parse_literal.h>
#include <lyric_rewriter/rewriter_result.h>

#include "lyric_schema/assembler_schema.h"

lyric_assembler::internal::NoOperandOpcodeMacro::NoOperandOpcodeMacro(lyric_object::Opcode opcode)
    : m_opcode(opcode)
{
}

tempo_utils::Status
lyric_assembler::internal::NoOperandOpcodeMacro::rewritePragma(
    const lyric_parser::ArchetypeNode *pragmaNode,
    lyric_rewriter::PragmaContext &ctx,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "{} macro is not valid in pragma context", opcode_to_name(m_opcode));
}

tempo_utils::Status
lyric_assembler::internal::NoOperandOpcodeMacro::rewriteDefinition(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_parser::ArchetypeNode *definitionNode,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "{} macro is not valid in definition context", opcode_to_name(m_opcode));
}

tempo_utils::Status
lyric_assembler::internal::NoOperandOpcodeMacro::rewriteBlock(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_rewriter::MacroBlock &macroBlock,
    lyric_parser::ArchetypeState *state)
{
    TU_LOG_VV << "rewrite " << lyric_object::opcode_to_name(m_opcode) << " macro";

    if (macroCallNode->numChildren() != 0)
        return lyric_rewriter::RewriterStatus::forCondition(
            lyric_rewriter::RewriterCondition::kSyntaxError,
            "expected 0 arguments for {} macro", lyric_object::opcode_to_name(m_opcode));

    lyric_parser::ArchetypeNode *opNode;
    TU_ASSIGN_OR_RETURN (opNode, state->appendNode(lyric_schema::kLyricAssemblerOpClass, {}));
    TU_RETURN_IF_NOT_OK (opNode->putAttr(kLyricAssemblerOpcodeEnum, m_opcode));
    TU_RETURN_IF_NOT_OK (macroBlock.appendNode(opNode));

    return {};
}

lyric_assembler::internal::StackOffsetOpcodeMacro::StackOffsetOpcodeMacro(lyric_object::Opcode opcode)
    : m_opcode(opcode)
{
}

tempo_utils::Status
lyric_assembler::internal::StackOffsetOpcodeMacro::rewritePragma(
    const lyric_parser::ArchetypeNode *pragmaNode,
    lyric_rewriter::PragmaContext &ctx,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "{} macro is not valid in pragma context", opcode_to_name(m_opcode));
}

tempo_utils::Status
lyric_assembler::internal::StackOffsetOpcodeMacro::rewriteDefinition(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_parser::ArchetypeNode *definitionNode,
    lyric_parser::ArchetypeState *state)
{
    return lyric_rewriter::RewriterStatus::forCondition(
        lyric_rewriter::RewriterCondition::kRewriterInvariant,
        "{} macro is not valid in definition context", opcode_to_name(m_opcode));
}

tempo_utils::Status
lyric_assembler::internal::StackOffsetOpcodeMacro::rewriteBlock(
    const lyric_parser::ArchetypeNode *macroCallNode,
    lyric_rewriter::MacroBlock &macroBlock,
    lyric_parser::ArchetypeState *state)
{
    TU_LOG_VV << "rewrite " << lyric_object::opcode_to_name(m_opcode) << " macro";

    if (macroCallNode->numChildren() != 1)
        return lyric_rewriter::RewriterStatus::forCondition(
            lyric_rewriter::RewriterCondition::kSyntaxError,
            "expected 1 argument for {} macro", lyric_object::opcode_to_name(m_opcode));
    auto *arg0 = macroCallNode->getChild(0);

    std::string literalValue;
    TU_RETURN_IF_NOT_OK (arg0->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));
    tu_uint32 u32;
    if (!absl::SimpleAtoi(literalValue, &u32) || u32 > std::numeric_limits<tu_uint16>::max())
        return lyric_rewriter::RewriterStatus::forCondition(
            lyric_rewriter::RewriterCondition::kSyntaxError,
            "invalid offset '{}' for {} macro", literalValue, lyric_object::opcode_to_name(m_opcode));
    auto stackOffset = static_cast<tu_uint16>(u32);

    lyric_parser::ArchetypeNode *opNode;
    TU_ASSIGN_OR_RETURN (opNode, state->appendNode(lyric_schema::kLyricAssemblerOpClass, {}));
    TU_RETURN_IF_NOT_OK (opNode->putAttr(kLyricAssemblerOpcodeEnum, m_opcode));
    TU_RETURN_IF_NOT_OK (opNode->putAttr(kLyricAssemblerStackOffset, stackOffset));
    TU_RETURN_IF_NOT_OK (macroBlock.appendNode(opNode));

    return {};
}
