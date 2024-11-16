
#include <lyric_compiler/assembler_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_rewriter/assembler_attrs.h>
#include <lyric_schema/assembler_schema.h>

lyric_compiler::AssemblerChoice::AssemblerChoice(
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::AssemblerChoice::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAssemblerNs))
        return {};
    auto *resource = lyric_schema::kLyricAssemblerVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    TU_LOG_INFO << "decide AssemblerChoice@" << this << ": "
                << resource->getNsUrl() << "#" << resource->getName();

    switch (astId) {

        case lyric_schema::LyricAssemblerId::Trap: {
            tu_uint32 trapNumber;
            TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_rewriter::kLyricAssemblerTrapNumber, trapNumber));
            TU_RETURN_IF_NOT_OK (m_fragment->trap(trapNumber, /* flags= */ 0));
            return {};
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid assembler node");
    }
}