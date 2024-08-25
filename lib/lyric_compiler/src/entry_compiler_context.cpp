
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/entry_compiler_context.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::EntryCompilerContext::EntryCompilerContext(
    CompilerScanDriver *driver,
    lyric_assembler::CallSymbol *entry,
    lyric_assembler::NamespaceSymbol *root)
    : m_driver(driver),
      m_entry(entry),
      m_root(root)
{
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_entry != nullptr);
    TU_ASSERT (m_root != nullptr);
}

lyric_assembler::BlockHandle *
lyric_compiler::EntryCompilerContext::getBlock() const
{
    return m_root->namespaceBlock();
}

tempo_utils::Status
lyric_compiler::EntryCompilerContext::enter(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_rewriter::VisitorContext &ctx)
{
    return CompilerStatus::forCondition(
        CompilerCondition::kCompilerInvariant, "encountered unexpected trailing node");
}

tempo_utils::Status
lyric_compiler::EntryCompilerContext::exit(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    const lyric_rewriter::VisitorContext &ctx)
{
    return CompilerStatus::forCondition(
        CompilerCondition::kCompilerInvariant, "encountered unexpected trailing node");
}
