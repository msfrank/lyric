
#include <lyric_analyzer/internal/analyze_defclass.h>

tempo_utils::Status
lyric_analyzer::internal::analyze_defclass(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    EntryPoint &entryPoint)
{
    return AnalyzerStatus::forCondition(AnalyzerCondition::kAnalyzerInvariant, "unimplemented");
}
