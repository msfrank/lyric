
#include <queue>
#include <stack>

// NOTE: this must come before topological_sort.hpp!
#include <boost/graph/vector_as_graph.hpp>

#include <boost/graph/topological_sort.hpp>

#include <lyric_optimizer/build_proc.h>
#include <lyric_optimizer/internal/cfg_types.h>

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

static int
get_vertex_index(
    const lyric_optimizer::Instance &instance,
    absl::flat_hash_map<lyric_optimizer::Instance,int> &instancesMap)
{
    int vertexIndex;
    auto entry = instancesMap.find(instance);
    if (entry != instancesMap.cend())
        return entry->second;

    vertexIndex = instancesMap.size();
    instancesMap[instance] = vertexIndex;
    return vertexIndex;
}

static void
add_instance_dependency(
    int sourceIndex,
    int targetIndex,
    std::vector<std::list<int>> &dependencies)
{
    auto requiredSize = std::max(sourceIndex, targetIndex) + 1;
    if (dependencies.size() <= sourceIndex) {
        dependencies.resize(requiredSize);
    }
    auto &targets = dependencies.at(sourceIndex);
    targets.push_back(targetIndex);
}

tempo_utils::Status
apply_phi_functions(
    const lyric_optimizer::BasicBlock &basicBlock,
    lyric_assembler::ProcHandle *procHandle)
{
    auto successors = basicBlock.listSuccessorBlocks();

    std::queue<lyric_optimizer::Instance> phisFound;

    // find the phi functions in each successor block
    for (const auto &successor : successors) {
        for (auto it = successor.phiFunctionsBegin(); it != successor.phiFunctionsEnd(); it++) {
            phisFound.push(*it);
        }
    }

    std::vector<std::list<int>> dependencies;
    absl::flat_hash_map<lyric_optimizer::Instance,int> instancesMap;

    // recursively inspect the arguments of each phi function and build a dependency graph
    while (!phisFound.empty()) {
        auto instance = phisFound.front();
        phisFound.pop();

        int targetIndex = get_vertex_index(instance, instancesMap);

        auto value = instance.getValue();
        if (!value.hasValue())
            continue;

        auto directive = value.getValue();
        if (directive->getType() != lyric_optimizer::DirectiveType::PhiFunction)
            continue;
        auto phiFunction = std::static_pointer_cast<lyric_optimizer::PhiFunction>(directive);

        for (auto it = phiFunction->argumentsBegin(); it != phiFunction->argumentsEnd(); it++) {
            const auto &argument = *it;
            int sourceIndex = get_vertex_index(argument, instancesMap);
            add_instance_dependency(sourceIndex, targetIndex, dependencies);
            auto argumentValue = argument.getValue();
            auto argumentDirective = argumentValue.getValue();
            if (argumentDirective->getType() == lyric_optimizer::DirectiveType::PhiFunction) {
                phisFound.push(argument);
            }
        }
    }

    // order the instances based on phi function dependencies
    std::deque<int> topologicalOrder;
    boost::topological_sort(dependencies, std::front_inserter(topologicalOrder),
        boost::vertex_index_map(boost::identity_property_map()));

    // create vector containing instances ordered by target index
    std::vector<lyric_optimizer::Instance> instances(dependencies.size());
    for (const auto &entry : instancesMap) {
        instances[entry.second] = entry.first;
    }

    // apply each phi function in order
    for (auto index : topologicalOrder) {
        auto &instance = instances.at(index);
        TU_LOG_VV << "apply phi " << instance.toString();
    }

    return {};
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

            // apply phi functions from successor blocks
            TU_RETURN_IF_NOT_OK (apply_phi_functions(currBlock, procHandle));

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
