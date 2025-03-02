#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_bootstrap/bootstrap_helpers.h>
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
    auto *block = procHandle->procBlock();
    auto *root = code->rootFragment();

    lyric_assembler::DataReference cond;
    TU_ASSIGN_OR_RAISE (cond, block->declareTemporary(
        lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Bool")), true));
    lyric_assembler::DataReference result;
    TU_ASSIGN_OR_RAISE (result, block->declareTemporary(
        lyric_common::TypeDef::forConcrete(lyric_bootstrap::preludeSymbol("Int")), true));

    root->immediateBool(true);
    root->storeRef(cond);
    root->loadRef(cond);

    lyric_assembler::JumpTarget targetIfFalse;
    TU_ASSIGN_OR_RAISE (targetIfFalse, root->jumpIfFalse());
    root->immediateInt(1);
    root->immediateInt(2);
    root->intAdd();
    root->storeRef(result);
    lyric_assembler::JumpTarget targetJoin;
    TU_ASSIGN_OR_RAISE (targetJoin, root->unconditionalJump());
    lyric_assembler::JumpLabel labelIfFalse;
    TU_ASSIGN_OR_RAISE (labelIfFalse, root->appendLabel("ifFalse"));
    root->immediateInt(0);
    root->storeRef(result);
    lyric_assembler::JumpLabel labelJoin;
    TU_ASSIGN_OR_RAISE (labelJoin, root->appendLabel("join"));
    root->loadRef(result);
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

    auto entryBlock = cfg.getEntryBlock();
    ASSERT_EQ (1, entryBlock.numDirectives());
    auto directive0 = entryBlock.getDirective(0);
    ASSERT_EQ (lyric_optimizer::DirectiveType::Bool, directive0->getType());
    ASSERT_TRUE (entryBlock.hasNextEdge());
    ASSERT_TRUE (entryBlock.hasBranchEdge());
    ASSERT_EQ (lyric_optimizer::BranchType::IfFalse, entryBlock.getBranchType());

    auto block1 = entryBlock.getNextBlock();
    ASSERT_EQ (1, block1.numDirectives());
    auto directive1 = block1.getDirective(0);
    ASSERT_EQ (lyric_optimizer::DirectiveType::IntAdd, directive1->getType());
    ASSERT_FALSE (block1.hasNextEdge());
    ASSERT_TRUE (block1.hasJumpEdge());
    ASSERT_EQ (entryBlock, block1.getPrevBlock());

    auto block2 = entryBlock.getBranchBlock();
    ASSERT_EQ ("ifFalse", block2.getLabel());
    ASSERT_EQ (1, block2.numDirectives());
    auto directive2 = block2.getDirective(0);
    ASSERT_EQ (lyric_optimizer::DirectiveType::Int, directive2->getType());
    ASSERT_TRUE (block2.hasNextEdge());

    auto block3 = block2.getNextBlock();
    ASSERT_EQ ("join", block3.getLabel());
    ASSERT_EQ (0, block3.numDirectives());
    ASSERT_TRUE (block3.hasReturnEdge());
    ASSERT_EQ (block2, block3.getPrevBlock());

    auto block4 = block3.getReturnBlock();
    ASSERT_EQ (block4, cfg.getExitBlock());
}