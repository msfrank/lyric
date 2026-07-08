
#include <lyric_assembler/object_root.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/entry_handler.h>
#include <lyric_parser/ast_attrs.h>

#include "lyric_assembler/entry_handle.h"
#include "lyric_compiler/proc_handler.h"

lyric_compiler::EntryHandler::EntryHandler(CompilerScanDriver *driver)
    : BaseGrouping(driver)
{
}

tempo_utils::Status
lyric_compiler::EntryHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before EntryHandler@" << this;

    auto *driver = getDriver();
    auto *objectRoot = driver->getObjectRoot();

    TU_ASSIGN_OR_RETURN (m_entryHandle, objectRoot->declareEntry());

    auto *entryProc = m_entryHandle->entryProc();
    m_fragment = entryProc->procFragment();

    auto handler = std::make_unique<ProcHandler>(
        entryProc, /* requiresResult= */ false, entryProc->procBlock(), driver);
    ctx.appendGrouping(std::move(handler));

    return {};
}

tempo_utils::Status
lyric_compiler::EntryHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after EntryHandler@" << this;

    auto numStatements = m_fragment->numStatements();
    bool unfinished = true;

    if (numStatements > 0) {
        auto lastStatement = m_fragment->getStatement(m_fragment->numStatements() - 1);
        switch (lastStatement.instruction->getType()) {
            case lyric_assembler::InstructionType::Jump:
            case lyric_assembler::InstructionType::Return:
            case lyric_assembler::InstructionType::Interrupt:
            case lyric_assembler::InstructionType::Abort:
            case lyric_assembler::InstructionType::Halt:
                unfinished = false;
                break;
            default:
                unfinished = true;
        }
    }

    // add HALT op if the last statement does not unconditionally transfer control or terminate
    if (unfinished) {
        TU_RETURN_IF_NOT_OK (m_fragment->invokeHalt());
    }

    return {};
}