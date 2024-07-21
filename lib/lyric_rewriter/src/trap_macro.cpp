
#include <lyric_rewriter/trap_macro.h>

lyric_rewriter::TrapMacro::TrapMacro()
{
}

tempo_utils::Status
lyric_rewriter::TrapMacro::rewriteBlock(
    const lyric_parser::ArchetypeNode *macroCallNode,
    MacroBlock &macroBlock,
    lyric_parser::ArchetypeState *state)
{
    TU_LOG_INFO << "rewrite trap macro";
    return {};
}
