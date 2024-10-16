
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/entry_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::EntryHandler::EntryHandler(CompilerScanDriver *driver)
    : BaseGrouping(driver)
{
}

tempo_utils::Status
lyric_compiler::EntryHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
{
    if (!node->isClass(lyric_schema::kLyricAstBlockClass))
        return CompilerStatus::forCondition(
            CompilerCondition::kCompilerInvariant, "invalid node for entry");

    TU_LOG_INFO << "before EntryHandler@" << this;

    auto *driver = getDriver();
    auto *entryCall = driver->getEntryCall();
    auto *entryProc = entryCall->callProc();
    auto *entryBlock = entryProc->procBlock();
    auto *entryCode = entryProc->procCode();
    auto *fragment = entryCode->rootFragment();

    auto numChildren = node->numChildren();
    TU_ASSERT (numChildren > 0);

    for (int i = 0; i < numChildren; i++) {
        auto any = std::make_unique<FormChoice>(
            FormType::Any, fragment, entryBlock, driver);
        ctx.appendChoice(std::move(any));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::EntryHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    TU_LOG_INFO << "after EntryHandler@" << this;
    return {};
}