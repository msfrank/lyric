#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_optimizer/control_flow_graph.h>
#include <lyric_optimizer/parse_proc.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

#include "base_optimizer_fixture.h"

class ControlFlowGraphTests : public BaseOptimizerFixture {
protected:
    ControlFlowGraphTests() = default;
};

TEST_F(ControlFlowGraphTests, InitializeFromProc)
{
    ASSERT_THAT (configure(), tempo_test::IsOk());

    lyric_assembler::CallSymbol *callSymbol;
    TU_ASSIGN_OR_RAISE (callSymbol, declareFunction(
        "test", lyric_object::AccessType::Public, {}));

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RAISE (procHandle, callSymbol->defineCall({}));

    auto *code = procHandle->procCode();
    auto *root = code->rootFragment();

    root->immediateBool(true);
    lyric_assembler::JumpTarget targetIfFalse;
    TU_ASSIGN_OR_RAISE (targetIfFalse, root->jumpIfFalse());
    root->immediateInt(1);
    lyric_assembler::JumpTarget targetJoin;
    TU_ASSIGN_OR_RAISE (targetJoin, root->unconditionalJump());
    lyric_assembler::JumpLabel labelIfFalse;
    TU_ASSIGN_OR_RAISE (labelIfFalse, root->appendLabel("ifFalse"));
    root->immediateInt(0);
    lyric_assembler::JumpLabel labelJoin;
    TU_ASSIGN_OR_RAISE (labelJoin, root->appendLabel("join"));
    root->patchTarget(targetIfFalse, labelIfFalse);
    root->patchTarget(targetJoin, labelJoin);
    root->returnToCaller();

    TU_LOG_INFO;
    TU_LOG_INFO << "proc:";
    for (auto it = root->statementsBegin(); it != root->statementsEnd(); it++) {
        const auto &instruction = it->instruction;
        TU_LOG_INFO << "  " << instruction->toString();
    }

    TU_LOG_INFO;
    TU_LOG_INFO << "cfg:";
    lyric_optimizer::ControlFlowGraph cfg;
    TU_ASSIGN_OR_RAISE (cfg, lyric_optimizer::parse_proc(procHandle));
    cfg.print();
}