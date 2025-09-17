
#include <lyric_assembler/assembler_macro_visitor.h>
#include <lyric_schema/assembler_schema.h>

lyric_assembler::AssemblerMacroVisitor::AssemblerMacroVisitor(lyric_rewriter::AbstractProcessorState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Status
lyric_assembler::AssemblerMacroVisitor::enter(
    lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    TU_RETURN_IF_NOT_OK (m_state->enterNode(node, ctx));

    if (ctx.skipChildren())
        return {};

    std::vector<std::pair<lyric_parser::ArchetypeNode *,int>> children;
    for (int i = node->numChildren() - 1; i >= 0; i--) {
        children.emplace_back(node->getChild(i), i);
    }

    for (auto &childAndIndex : children) {
        std::shared_ptr<AbstractNodeVisitor> visitor;
        TU_ASSIGN_OR_RETURN (visitor, m_state->makeVisitor(childAndIndex.first));
        ctx.push(node, childAndIndex.second, childAndIndex.first, visitor);
    }

    return {};
}

tempo_utils::Status
lyric_assembler::AssemblerMacroVisitor::exit(
    lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    return m_state->exitNode(node, ctx);
}

std::shared_ptr<lyric_rewriter::AbstractNodeVisitor>
lyric_assembler::make_assembler_visitor(
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::AbstractProcessorState *state)
{
    lyric_schema::LyricAssemblerId assemblerId;
    TU_RAISE_IF_NOT_OK (node->parseId(lyric_schema::kLyricAssemblerVocabulary, assemblerId));

    switch (assemblerId) {
        case lyric_schema::LyricAssemblerId::AllocatorTrap:
        case lyric_schema::LyricAssemblerId::Plugin:
        case lyric_schema::LyricAssemblerId::LoadData:
        case lyric_schema::LyricAssemblerId::StoreData:
        case lyric_schema::LyricAssemblerId::Op:
        case lyric_schema::LyricAssemblerId::Trap:
            return std::make_shared<AssemblerMacroVisitor>(state);
        default:
            return {};
    }
}
