
#include <lyric_compiler/block_compiler_context.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/constant_utils.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/operator_utils.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_compiler::BlockCompilerContext::BlockCompilerContext(
    CompilerScanDriver *driver,
    lyric_assembler::BlockHandle *parentBlock)
    : m_driver(driver),
      m_parentBlock(parentBlock)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_parentBlock != nullptr);
    m_block = std::make_unique<lyric_assembler::BlockHandle>(
        parentBlock->blockProc(), parentBlock->blockCode(), parentBlock, parentBlock->blockState());
}

lyric_assembler::BlockHandle *
lyric_compiler::BlockCompilerContext::getBlock() const
{
    return m_block.get();
}

tempo_utils::Status
lyric_compiler::BlockCompilerContext::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {

        // constant forms
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

        // terminal deref forms
        case lyric_schema::LyricAstId::This:
            return compile_this(getBlock(), m_driver);
        case lyric_schema::LyricAstId::Name:
            return compile_name(node, getBlock(), m_driver);

        // control forms
        case lyric_schema::LyricAstId::Block:
            return declareBlock(node);

//        // definition forms
//        case lyric_schema::LyricAstId::Def:
//            return m_driver->pushFunction(node, getBlock());
//        case lyric_schema::LyricAstId::DefClass:
//            return m_driver->pushClass(node, getBlock());
//        case lyric_schema::LyricAstId::DefConcept:
//            return m_driver->pushConcept(node, getBlock());
//        case lyric_schema::LyricAstId::DefEnum:
//            return m_driver->pushEnum(node, getBlock());
//        case lyric_schema::LyricAstId::DefInstance:
//            return m_driver->pushInstance(node, getBlock());
//        case lyric_schema::LyricAstId::DefStruct:
//            return m_driver->pushStruct(node, getBlock());
//        case lyric_schema::LyricAstId::Namespace:
//            return m_driver->pushNamespace(node, getBlock());

        default:
            break;
    }

    return {};
}

tempo_utils::Status
lyric_compiler::BlockCompilerContext::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    // determine whether we have reached the last form in the block
    auto remaining = (ctx.parentNode()->numChildren() - ctx.childIndex()) - 1;

    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {

        // unary operator forms
        case lyric_schema::LyricAstId::Neg:
        case lyric_schema::LyricAstId::Not:
            TU_RETURN_IF_NOT_OK (compile_unary_operator(astId, getBlock(), m_driver));

        // binary operator forms
        case lyric_schema::LyricAstId::IsEq:
        case lyric_schema::LyricAstId::IsGe:
        case lyric_schema::LyricAstId::IsGt:
        case lyric_schema::LyricAstId::IsLe:
        case lyric_schema::LyricAstId::IsLt:
        case lyric_schema::LyricAstId::Add:
        case lyric_schema::LyricAstId::Sub:
        case lyric_schema::LyricAstId::Mul:
        case lyric_schema::LyricAstId::Div:
        case lyric_schema::LyricAstId::And:
        case lyric_schema::LyricAstId::Or:
            TU_RETURN_IF_NOT_OK (compile_binary_operator(astId, getBlock(), m_driver));

        // control forms
        case lyric_schema::LyricAstId::Block:
            TU_RETURN_IF_NOT_OK (m_driver->popContext());

        // definition forms

        default:
            break;
    }

    // if this is not the last form in the block then pop any intermediate value off the stack
    if (remaining > 0) {
        auto resultType = m_driver->peekResult();
        TU_RETURN_IF_NOT_OK (m_driver->popResult());
        if (resultType.getType() != lyric_common::TypeDefType::NoReturn) {
            auto *blockCode = m_block->blockCode();
            auto *fragment = blockCode->rootFragment();
            TU_RETURN_IF_NOT_OK (fragment->popValue());
        }
    }

    return {};
}

tempo_utils::Status
lyric_compiler::BlockCompilerContext::declareBlock(const lyric_parser::ArchetypeNode *node)
{
    auto block = std::make_unique<BlockCompilerContext>(m_driver, m_block.get());
    return m_driver->pushContext(std::move(block));
}
