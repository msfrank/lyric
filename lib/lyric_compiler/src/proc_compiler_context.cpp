
#include <lyric_compiler/block_compiler_context.h>
//#include <lyric_compiler/namespace_compiler_context.h>
#include <lyric_compiler/proc_compiler_context.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

lyric_compiler::ProcCompilerContext::ProcCompilerContext(
    CompilerScanDriver *driver,
    lyric_assembler::ProcHandle *procHandle)
    : m_driver(driver),
      m_procHandle(procHandle)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_procHandle != nullptr);
}

lyric_assembler::BlockHandle *
lyric_compiler::ProcCompilerContext::getBlock() const
{
    return m_procHandle->procBlock();
}

tempo_utils::Status
lyric_compiler::ProcCompilerContext::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {
        case lyric_schema::LyricAstId::Block: {
            auto block = std::make_unique<BlockCompilerContext>(m_driver, m_procHandle->procBlock());
            return m_driver->pushContext(std::move(block));
        }
        default:
            break;
    }

    return {};
}

tempo_utils::Status
lyric_compiler::ProcCompilerContext::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
//    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());
//
//    auto astId = resource->getId();
//    switch (astId) {
//        case lyric_schema::LyricAstId::Block:
//            m_driver->popContext();
//            break;
//        case lyric_schema::LyricAstId::DefStatic:
//            return m_driver->declareStatic(node, getBlock());
//        default:
//            break;
//    }
    return {};
}
