
#include <lyric_compiler/block_compiler_context.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/constant_utils.h>
#include <lyric_compiler/deref_compiler_context.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/operator_utils.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_compiler::DerefCompilerContext::DerefCompilerContext(
    CompilerScanDriver *driver,
    lyric_assembler::BlockHandle *block)
    : m_driver(driver),
      m_block(block),
      m_initial(true)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_block != nullptr);
}

lyric_assembler::BlockHandle *
lyric_compiler::DerefCompilerContext::getBlock() const
{
    return m_block;
}

tempo_utils::Status
lyric_compiler::DerefCompilerContext::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    if (m_initial)
        return enterInitial(state, node, ctx);
    return enterNext(state, node, ctx);
}

tempo_utils::Status
lyric_compiler::DerefCompilerContext::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    if (m_initial) {
        auto status = exitInitial(state, node, ctx);
        m_initial = false;
        return status;
    }
    return exitNext(state, node, ctx);
}

tempo_utils::Status
lyric_compiler::DerefCompilerContext::enterInitial(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {

        // deref constant
        case lyric_schema::LyricAstId::Nil:
            return compile_nil(getBlock(), m_driver);
        case lyric_schema::LyricAstId::Undef:
            return compile_undef(getBlock(), m_driver);
        case lyric_schema::LyricAstId::True:
            return compile_true(getBlock(), m_driver);
        case lyric_schema::LyricAstId::False:
            return compile_false(getBlock(), m_driver);
        case lyric_schema::LyricAstId::Integer:
            return compile_integer(node, getBlock(), m_driver);
        case lyric_schema::LyricAstId::Float:
            return compile_float(node, getBlock(), m_driver);
        case lyric_schema::LyricAstId::Char:
            return compile_char(node, getBlock(), m_driver);
        case lyric_schema::LyricAstId::String:
            return compile_string(node, getBlock(), m_driver);
        case lyric_schema::LyricAstId::Url:
            return compile_url(node, getBlock(), m_driver);
        case lyric_schema::LyricAstId::SymbolRef:
            return compile_symbol(node, getBlock(), m_driver);

        // deref receiver
        case lyric_schema::LyricAstId::This:
            return compile_this(getBlock(), m_driver);

        // deref binding
        case lyric_schema::LyricAstId::Name:
            return compile_name(node, getBlock(), m_driver);

        // apply function
        case lyric_schema::LyricAstId::Call:

        // apply construction
        case lyric_schema::LyricAstId::New:

        // deref grouping
        case lyric_schema::LyricAstId::Block:
            return declareBlock(node);

        default:
            break;
    }

    return {};
}

tempo_utils::Status
lyric_compiler::DerefCompilerContext::exitInitial(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {

        // deref grouping
        case lyric_schema::LyricAstId::Block:
            TU_RETURN_IF_NOT_OK (m_driver->popContext());

        default:
            break;
    }

    return {};
}

tempo_utils::Status
lyric_compiler::DerefCompilerContext::enterNext(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    TU_UNREACHABLE();
}

tempo_utils::Status
lyric_compiler::DerefCompilerContext::exitNext(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    TU_UNREACHABLE();
}

tempo_utils::Status
lyric_compiler::DerefCompilerContext::declareBlock(const lyric_parser::ArchetypeNode *node)
{
    auto block = std::make_unique<BlockCompilerContext>(m_driver, m_block);
    return m_driver->pushContext(std::move(block));
}
