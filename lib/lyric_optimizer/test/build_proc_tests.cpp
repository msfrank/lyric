#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_bootstrap/bootstrap_helpers.h>
#include <lyric_optimizer/build_proc.h>
#include <lyric_optimizer/control_flow_graph.h>
#include <lyric_optimizer/parse_proc.h>
#include <lyric_test/matchers.h>
#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>

#include "base_optimizer_fixture.h"

class BuildProcTests : public BaseOptimizerFixture {
protected:
    BuildProcTests() = default;
};

TEST_F(BuildProcTests, Build)
{
    ASSERT_THAT (configure(), tempo_test::IsOk());

    lyric_assembler::CallSymbol *inputCall;
    TU_ASSIGN_OR_RAISE (inputCall, declareFunction(
        "input", /* isHidden= */ false, {}));

    lyric_assembler::ProcHandle *inputProc;
    TU_ASSIGN_OR_RAISE (inputProc, inputCall->defineCall({}));

    auto *inputFragment = inputProc->procFragment();

    inputFragment->immediateBool(true);
    inputFragment->returnToCaller();

    TU_LOG_INFO;
    TU_LOG_INFO << "input proc:";
    for (auto it = inputFragment->statementsBegin(); it != inputFragment->statementsEnd(); it++) {
        const auto &instruction = it->instruction;
        TU_LOG_INFO << "  " << instruction->toString();
    }

    TU_LOG_INFO;
    TU_LOG_INFO << "cfg:";
    lyric_optimizer::ControlFlowGraph cfg;
    TU_ASSIGN_OR_RAISE (cfg, lyric_optimizer::parse_proc(inputProc));
    cfg.print();

    lyric_assembler::CallSymbol *outputCall;
    TU_ASSIGN_OR_RAISE (outputCall, declareFunction(
        "output", /* isHidden= */ false, {}));

    lyric_assembler::ProcHandle *outputProc;
    TU_ASSIGN_OR_RAISE (outputProc, outputCall->defineCall({}));

    ASSERT_THAT (lyric_optimizer::build_proc(cfg, outputProc), tempo_test::IsOk());
    auto *outputFragment = outputProc->procFragment();

    TU_LOG_INFO;
    TU_LOG_INFO << "output proc:";
    for (auto it = outputFragment->statementsBegin(); it != outputFragment->statementsEnd(); it++) {
        const auto &instruction = it->instruction;
        TU_LOG_INFO << "  " << instruction->toString();
    }
}

TEST_F(BuildProcTests, BuildConditionalWithPhiFunction)
{
    ASSERT_THAT (configure(), tempo_test::IsOk());

    lyric_assembler::CallSymbol *inputCall;
    TU_ASSIGN_OR_RAISE (inputCall, declareFunction(
        "input", /* isHidden= */ false, {}));

    lyric_assembler::ProcHandle *inputProc;
    TU_ASSIGN_OR_RAISE (inputProc, inputCall->defineCall({}));

    auto *inputBlock = inputProc->procBlock();
    auto *inputFragment = inputProc->procFragment();

    lyric_assembler::DataReference cond;
    TU_ASSIGN_OR_RAISE (cond, inputBlock->declareTemporary(
        lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Bool")).orElseThrow(),
        true));
    lyric_assembler::DataReference result;
    TU_ASSIGN_OR_RAISE (result, inputBlock->declareTemporary(
        lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")).orElseThrow(),
        true));

    inputFragment->immediateBool(true);
    inputFragment->storeRef(cond);
    inputFragment->loadRef(cond);
    lyric_assembler::JumpTarget targetIfFalse;
    TU_ASSIGN_OR_RAISE (targetIfFalse, inputFragment->jumpIfFalse());
    inputFragment->immediateInt(1);
    inputFragment->immediateInt(2);
    inputFragment->intAdd();
    inputFragment->storeRef(result);
    lyric_assembler::JumpTarget targetJoin;
    TU_ASSIGN_OR_RAISE (targetJoin, inputFragment->unconditionalJump());
    lyric_assembler::JumpLabel labelIfFalse;
    TU_ASSIGN_OR_RAISE (labelIfFalse, inputFragment->appendLabel("ifFalse"));
    inputFragment->immediateInt(0);
    inputFragment->storeRef(result);
    lyric_assembler::JumpLabel labelJoin;
    TU_ASSIGN_OR_RAISE (labelJoin, inputFragment->appendLabel("join"));
    inputFragment->loadRef(result);
    inputFragment->patchTarget(targetIfFalse, labelIfFalse);
    inputFragment->patchTarget(targetJoin, labelJoin);
    inputFragment->returnToCaller();

    TU_LOG_INFO;
    TU_LOG_INFO << "input proc:";
    for (auto it = inputFragment->statementsBegin(); it != inputFragment->statementsEnd(); it++) {
        const auto &instruction = it->instruction;
        TU_LOG_INFO << "  " << instruction->toString();
    }

    TU_LOG_INFO;
    TU_LOG_INFO << "cfg:";
    lyric_optimizer::ControlFlowGraph cfg;
    TU_ASSIGN_OR_RAISE (cfg, lyric_optimizer::parse_proc(inputProc));
    cfg.print();

    lyric_assembler::CallSymbol *outputCall;
    TU_ASSIGN_OR_RAISE (outputCall, declareFunction(
        "output", /* isHidden= */ false, {}));

    lyric_assembler::ProcHandle *outputProc;
    TU_ASSIGN_OR_RAISE (outputProc, outputCall->defineCall({}));

    ASSERT_THAT (lyric_optimizer::build_proc(cfg, outputProc), tempo_test::IsOk());
    auto *outputFragment = outputProc->procFragment();

    TU_LOG_INFO;
    TU_LOG_INFO << "output proc:";
    for (auto it = outputFragment->statementsBegin(); it != outputFragment->statementsEnd(); it++) {
        const auto &instruction = it->instruction;
        TU_LOG_INFO << "  " << instruction->toString();
    }
}