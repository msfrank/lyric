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

    auto *inputCode = inputProc->procCode();
    auto *inputRoot = inputCode->rootFragment();

    inputRoot->immediateBool(true);
    inputRoot->returnToCaller();

    TU_LOG_INFO;
    TU_LOG_INFO << "input proc:";
    for (auto it = inputRoot->statementsBegin(); it != inputRoot->statementsEnd(); it++) {
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
    auto *outputCode = outputProc->procCode();
    auto *outputRoot = outputCode->rootFragment();

    TU_LOG_INFO;
    TU_LOG_INFO << "output proc:";
    for (auto it = outputRoot->statementsBegin(); it != outputRoot->statementsEnd(); it++) {
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

    auto *inputCode = inputProc->procCode();
    auto *inputRoot = inputCode->rootFragment();
    auto *inputBlock = inputProc->procBlock();

    lyric_assembler::DataReference cond;
    TU_ASSIGN_OR_RAISE (cond, inputBlock->declareTemporary(
        lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Bool")), true));
    lyric_assembler::DataReference result;
    TU_ASSIGN_OR_RAISE (result, inputBlock->declareTemporary(
        lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), true));

    inputRoot->immediateBool(true);
    inputRoot->storeRef(cond);
    inputRoot->loadRef(cond);
    lyric_assembler::JumpTarget targetIfFalse;
    TU_ASSIGN_OR_RAISE (targetIfFalse, inputRoot->jumpIfFalse());
    inputRoot->immediateInt(1);
    inputRoot->immediateInt(2);
    inputRoot->intAdd();
    inputRoot->storeRef(result);
    lyric_assembler::JumpTarget targetJoin;
    TU_ASSIGN_OR_RAISE (targetJoin, inputRoot->unconditionalJump());
    lyric_assembler::JumpLabel labelIfFalse;
    TU_ASSIGN_OR_RAISE (labelIfFalse, inputRoot->appendLabel("ifFalse"));
    inputRoot->immediateInt(0);
    inputRoot->storeRef(result);
    lyric_assembler::JumpLabel labelJoin;
    TU_ASSIGN_OR_RAISE (labelJoin, inputRoot->appendLabel("join"));
    inputRoot->loadRef(result);
    inputRoot->patchTarget(targetIfFalse, labelIfFalse);
    inputRoot->patchTarget(targetJoin, labelJoin);
    inputRoot->returnToCaller();

    TU_LOG_INFO;
    TU_LOG_INFO << "input proc:";
    for (auto it = inputRoot->statementsBegin(); it != inputRoot->statementsEnd(); it++) {
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
    auto *outputCode = outputProc->procCode();
    auto *outputRoot = outputCode->rootFragment();

    TU_LOG_INFO;
    TU_LOG_INFO << "output proc:";
    for (auto it = outputRoot->statementsBegin(); it != outputRoot->statementsEnd(); it++) {
        const auto &instruction = it->instruction;
        TU_LOG_INFO << "  " << instruction->toString();
    }
}