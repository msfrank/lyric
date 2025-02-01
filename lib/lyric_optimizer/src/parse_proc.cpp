
#include <lyric_assembler/proc_builder.h>
#include <lyric_optimizer/optimizer_result.h>
#include <lyric_optimizer/parse_proc.h>

struct ProposedBlock {
    std::string labelName;
    lyric_optimizer::BasicBlock basicBlock;
    std::shared_ptr<lyric_assembler::ControlInstruction> trailer;
};

static tempo_utils::Status
scan_for_basic_blocks(
    const lyric_assembler::CodeFragment *fragment,
    std::vector<std::unique_ptr<ProposedBlock>> &proposedBlocks,
    absl::flat_hash_map<std::string, lyric_optimizer::BasicBlock> &labelBlocks,
    lyric_optimizer::ControlFlowGraph &cfg)
{
    auto numStatements = fragment->numStatements();

    auto entryBlock = std::make_unique<ProposedBlock>();
    entryBlock->basicBlock = cfg.getEntryBlock();
    auto *currProposed = entryBlock.get();

    proposedBlocks.push_back(std::move(entryBlock));

    for (auto i = 0; i < numStatements; i++) {
        const auto &statement = fragment->getStatement(i);
        auto &instruction = statement.instruction;

        switch (instruction->getType()) {

            case lyric_assembler::InstructionType::Branch:
            case lyric_assembler::InstructionType::Jump:
            case lyric_assembler::InstructionType::Return: {

                // set branch/jump/return as the trailer
                currProposed->trailer = std::static_pointer_cast<lyric_assembler::ControlInstruction>(instruction);

                // if this is not the last instruction then create the next block
                if (i < numStatements - 1) {
                    auto nextProposed = std::make_unique<ProposedBlock>();
                    TU_ASSIGN_OR_RETURN (nextProposed->basicBlock, cfg.addBasicBlock());
                    currProposed = nextProposed.get();
                    proposedBlocks.push_back(std::move(nextProposed));
                    TU_LOG_INFO << "created block with io=" << i + 1;
                }

                break;
            }

            case lyric_assembler::InstructionType::Label: {
                // if the current block is not empty then create the next block
                if (currProposed->basicBlock.numInstructions() > 0) {
                    auto nextProposed = std::make_unique<ProposedBlock>();
                    TU_ASSIGN_OR_RETURN (nextProposed->basicBlock, cfg.addBasicBlock());
                    currProposed = nextProposed.get();
                    proposedBlocks.push_back(std::move(nextProposed));
                    TU_LOG_INFO << "created block with io=" << i;
                }

                // set label
                auto labelInstruction = std::static_pointer_cast<lyric_assembler::LabelInstruction>(instruction);
                currProposed->labelName = labelInstruction->getName();

                // map label name to basic block
                labelBlocks[currProposed->labelName] = currProposed->basicBlock;

                break;
            }

            case lyric_assembler::InstructionType::NoOperands:
            case lyric_assembler::InstructionType::BoolImmediate:
            case lyric_assembler::InstructionType::IntImmediate:
            case lyric_assembler::InstructionType::FloatImmediate:
            case lyric_assembler::InstructionType::CharImmediate:
            case lyric_assembler::InstructionType::LoadLiteral:
            case lyric_assembler::InstructionType::LoadData:
            case lyric_assembler::InstructionType::LoadDescriptor:
            case lyric_assembler::InstructionType::LoadSynthetic:
            case lyric_assembler::InstructionType::LoadType:
            case lyric_assembler::InstructionType::StoreData:
            case lyric_assembler::InstructionType::StackModification:
            case lyric_assembler::InstructionType::Call:
            case lyric_assembler::InstructionType::New:
            case lyric_assembler::InstructionType::Trap: {
                auto basicInstruction = std::static_pointer_cast<lyric_assembler::BasicInstruction>(instruction);
                currProposed->basicBlock.appendInstruction(basicInstruction);
                break;
            }

            default:
                return lyric_optimizer::OptimizerStatus::forCondition(
                    lyric_optimizer::OptimizerCondition::kOptimizerInvariant, "invalid instruction");
        }
    }

    return {};
}

static lyric_optimizer::BranchType opcode_to_branch_type(lyric_object::Opcode opcode)
{
    switch (opcode) {
        case lyric_object::Opcode::OP_IF_NIL:
            return lyric_optimizer::BranchType::IfNil;
        case lyric_object::Opcode::OP_IF_NOTNIL:
            return lyric_optimizer::BranchType::IfNotNil;
        case lyric_object::Opcode::OP_IF_TRUE:
            return lyric_optimizer::BranchType::IfTrue;
        case lyric_object::Opcode::OP_IF_FALSE:
            return lyric_optimizer::BranchType::IfFalse;
        case lyric_object::Opcode::OP_IF_ZERO:
            return lyric_optimizer::BranchType::IfZero;
        case lyric_object::Opcode::OP_IF_NOTZERO:
            return lyric_optimizer::BranchType::IfNotZero;
        case lyric_object::Opcode::OP_IF_GT:
            return lyric_optimizer::BranchType::IfGreaterThan;
        case lyric_object::Opcode::OP_IF_GE:
            return lyric_optimizer::BranchType::IfGreaterOrEqual;
        case lyric_object::Opcode::OP_IF_LT:
            return lyric_optimizer::BranchType::IfLessThan;
        case lyric_object::Opcode::OP_IF_LE:
            return lyric_optimizer::BranchType::IfLessOrEqual;
        default:
            return lyric_optimizer::BranchType::Invalid;
    }
}

static tempo_utils::Status
apply_control_flow(
    const lyric_assembler::ProcBuilder *code,
    std::vector<std::unique_ptr<ProposedBlock>> &proposedBlocks,
    const absl::flat_hash_map<std::string, lyric_optimizer::BasicBlock> &labelBlocks,
    lyric_optimizer::ControlFlowGraph &cfg)
{
    ProposedBlock *prevProposed = nullptr;

    for (auto &currProposed : proposedBlocks) {

        if (prevProposed != nullptr) {
            auto &prevBlock = prevProposed->basicBlock;
            auto &currBlock = currProposed->basicBlock;
            prevBlock.setNextBlock(currBlock);
            prevProposed = nullptr;
        }

        // if block is not empty then add edges based on the type of the last instruction
        if (currProposed->trailer != nullptr) {
            auto trailer = currProposed->trailer;
            switch (trailer->getType()) {
                case lyric_assembler::InstructionType::Branch: {
                    auto branchInstruction = std::static_pointer_cast<lyric_assembler::BranchInstruction>(trailer);
                    auto targetId = branchInstruction->getTargetId();
                    auto labelName = code->getLabelForTarget(targetId);
                    auto &currBlock = currProposed->basicBlock;
                    auto &targetBlock = labelBlocks.at(labelName);
                    TU_RETURN_IF_NOT_OK (currBlock.setBranchBlock(
                        targetBlock, opcode_to_branch_type(branchInstruction->getOpcode()), labelName));
                    // set prevProposed so that we add edge to the next block or the exit block
                    prevProposed = currProposed.get();
                    break;
                }

                case lyric_assembler::InstructionType::Jump: {
                    auto jumpInstruction = std::static_pointer_cast<lyric_assembler::JumpInstruction>(trailer);
                    auto targetId = jumpInstruction->getTargetId();
                    auto labelName = code->getLabelForTarget(targetId);
                    auto &currBlock = currProposed->basicBlock;
                    auto &targetBlock = labelBlocks.at(labelName);
                    TU_RETURN_IF_NOT_OK (currBlock.setJumpBlock(targetBlock, labelName));
                    break;
                }

                case lyric_assembler::InstructionType::Return: {
                    auto &currBlock = currProposed->basicBlock;
                    currBlock.setReturnBlock();
                    break;
                }

                default: {
                    // set prevBlock so that we add edge to the next block or the exit block
                    prevProposed = currProposed.get();
                    break;
                }
            }
        } else {
            // set prevBlock so that we add edge to the next block or the exit block
            prevProposed = currProposed.get();
        }
    }

    //
    if (prevProposed != nullptr) {
        auto &prevBlock = prevProposed->basicBlock;
        prevBlock.setNextBlock(cfg.getExitBlock());
    }

    return {};
}

tempo_utils::Result<lyric_optimizer::ControlFlowGraph>
lyric_optimizer::parse_proc(const lyric_assembler::ProcHandle *procHandle)
{
    auto *code = procHandle->procCode();
    auto *fragment = code->rootFragment();

    ControlFlowGraph cfg;
    std::vector<std::unique_ptr<ProposedBlock>> proposedBlocks;
    absl::flat_hash_map<std::string, BasicBlock> labelBlocks;
    TU_RETURN_IF_NOT_OK (scan_for_basic_blocks(fragment, proposedBlocks, labelBlocks, cfg));
    TU_RETURN_IF_NOT_OK (apply_control_flow(code, proposedBlocks, labelBlocks, cfg));

    return cfg;
}
