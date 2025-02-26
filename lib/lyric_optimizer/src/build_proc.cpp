
#include <lyric_optimizer/build_proc.h>

tempo_utils::Result<lyric_optimizer::BasicBlock>
find_leader_block(lyric_optimizer::BasicBlock block)
{
    TU_ASSERT (block.isValid());

    while (block.hasPrevEdge()) {
        block = block.getPrevBlock();
    }
    if (!block.hasLabel())
        return lyric_optimizer::OptimizerStatus::forCondition(
            lyric_optimizer::OptimizerCondition::kOptimizerInvariant,
            "expected label for leader block");
    return block;
}

tempo_utils::Result<lyric_assembler::JumpTarget>
write_branch(lyric_optimizer::BranchType branch, lyric_assembler::CodeFragment *codeFragment)
{
    switch (branch) {
        case lyric_optimizer::BranchType::IfNil:
            return codeFragment->jumpIfNil();
        case lyric_optimizer::BranchType::IfNotNil:
            return codeFragment->jumpIfNotNil();
        case lyric_optimizer::BranchType::IfTrue:
            return codeFragment->jumpIfTrue();
        case lyric_optimizer::BranchType::IfFalse:
            return codeFragment->jumpIfFalse();
        case lyric_optimizer::BranchType::IfZero:
            return codeFragment->jumpIfZero();
        case lyric_optimizer::BranchType::IfNotZero:
            return codeFragment->jumpIfNotZero();
        case lyric_optimizer::BranchType::IfGreaterThan:
            return codeFragment->jumpIfGreaterThan();
        case lyric_optimizer::BranchType::IfGreaterOrEqual:
            return codeFragment->jumpIfGreaterOrEqual();
        case lyric_optimizer::BranchType::IfLessThan:
            return codeFragment->jumpIfLessThan();
        case lyric_optimizer::BranchType::IfLessOrEqual:
            return codeFragment->jumpIfLessOrEqual();
        default:
            return lyric_optimizer::OptimizerStatus::forCondition(
                lyric_optimizer::OptimizerCondition::kOptimizerInvariant, "invalid branch type");
    }
}

tempo_utils::Status
lyric_optimizer::build_proc(const ControlFlowGraph &cfg, lyric_assembler::ProcHandle *procHandle)
{
    TU_ASSERT (procHandle != nullptr);

    auto *procBuilder = procHandle->procCode();
    auto *codeFragment = procBuilder->rootFragment();

    absl::flat_hash_map<std::string, lyric_assembler::JumpLabel> blockJumpLabels;
    absl::flat_hash_map<std::string, lyric_assembler::JumpTarget> blockJumpTargets;

    std::queue<BasicBlock> leaderBlocks;
    leaderBlocks.push(cfg.getEntryBlock());

    while (!leaderBlocks.empty()) {
        BasicBlock currBlock = leaderBlocks.front();
        leaderBlocks.pop();

        if (blockJumpLabels.contains(currBlock.getLabel()))
            continue;

        do {
            // if block has a label then write the jump label
            if (currBlock.hasLabel()) {
                auto labelName = currBlock.getLabel();
                lyric_assembler::JumpLabel jumpLabel;
                TU_ASSIGN_OR_RETURN (jumpLabel, codeFragment->appendLabel(labelName));
                blockJumpLabels[labelName] = jumpLabel;
            }

            for (int i = 0; i < currBlock.numDirectives(); i++) {
                auto directive = currBlock.getDirective(i);
                TU_RETURN_IF_NOT_OK (directive->buildCode(codeFragment, procHandle));
            }

            // if block has a transfer edge then get the transfer target and block
            lyric_assembler::JumpTarget transferTarget;
            BasicBlock transferBlock;
            if (currBlock.hasJumpEdge()) {
                TU_ASSIGN_OR_RETURN (transferTarget, codeFragment->unconditionalJump());
                transferBlock = currBlock.getJumpBlock();
            } else if (currBlock.hasBranchEdge()) {
                TU_ASSIGN_OR_RETURN (transferTarget, write_branch(currBlock.getBranchType(), codeFragment));
                transferBlock = currBlock.getBranchBlock();
            }

            // if transfer block is valid then add it to the list of leader blocks
            if (transferBlock.isValid()) {
                blockJumpTargets[transferBlock.getLabel()] = transferTarget;
                BasicBlock transferLeader;
                TU_ASSIGN_OR_RETURN (transferLeader, find_leader_block(transferBlock));
                if (!blockJumpLabels.contains(transferLeader.getLabel())) {
                    leaderBlocks.push(transferLeader);
                }
            }

            // if block has a return edge then add the return instruction and break from the loop
            if (currBlock.hasReturnEdge()) {
                TU_RETURN_IF_NOT_OK (codeFragment->returnToCaller());
                break;
            }

            // try to get next block. if next block is invalid then we break from the loop.
            currBlock = currBlock.getNextBlock();

        } while (currBlock.isValid());
    }

    for (const auto &labelEntry : blockJumpLabels) {
        auto targetEntry = blockJumpTargets.find(labelEntry.first);
        if (targetEntry == blockJumpTargets.cend())
            return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant);
        TU_RETURN_IF_NOT_OK (codeFragment->patchTarget(targetEntry->second, labelEntry.second));
    }

    return {};
}
