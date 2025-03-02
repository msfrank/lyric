
#include <lyric_assembler/proc_builder.h>
#include <lyric_optimizer/operand_stack.h>
#include <lyric_optimizer/optimizer_result.h>
#include <lyric_optimizer/parse_proc.h>

struct ProposedBlock {
    std::string labelName;
    lyric_optimizer::BasicBlock basicBlock;
    std::vector<std::shared_ptr<lyric_assembler::AbstractInstruction>> instructions;
    std::shared_ptr<lyric_assembler::AbstractInstruction> trailer;
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

        TU_LOG_INFO << "scanning instruction " << instruction->toString();

        switch (instruction->getType()) {

            case lyric_assembler::InstructionType::Branch:
            case lyric_assembler::InstructionType::Jump:
            case lyric_assembler::InstructionType::Return: {

                // set branch/jump/return as the trailer
                currProposed->trailer = instruction;

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
                if (!currProposed->instructions.empty()) {
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

            case lyric_assembler::InstructionType::Noop:
            case lyric_assembler::InstructionType::NilImmediate:
            case lyric_assembler::InstructionType::UndefImmediate:
            case lyric_assembler::InstructionType::BoolImmediate:
            case lyric_assembler::InstructionType::IntImmediate:
            case lyric_assembler::InstructionType::FloatImmediate:
            case lyric_assembler::InstructionType::CharImmediate:
            case lyric_assembler::InstructionType::BoolOperation:
            case lyric_assembler::InstructionType::IntOperation:
            case lyric_assembler::InstructionType::FloatOperation:
            case lyric_assembler::InstructionType::CharOperation:
            case lyric_assembler::InstructionType::LogicalOperation:
            case lyric_assembler::InstructionType::TypeOperation:
            case lyric_assembler::InstructionType::StackOperation:
            case lyric_assembler::InstructionType::LoadLiteral:
            case lyric_assembler::InstructionType::LoadDescriptor:
            case lyric_assembler::InstructionType::LoadSynthetic:
            case lyric_assembler::InstructionType::LoadType:
            case lyric_assembler::InstructionType::LoadData:
            case lyric_assembler::InstructionType::StoreData:
            case lyric_assembler::InstructionType::Call:
            case lyric_assembler::InstructionType::New:
            case lyric_assembler::InstructionType::Trap:
            case lyric_assembler::InstructionType::Interrupt:
            case lyric_assembler::InstructionType::Halt:
            case lyric_assembler::InstructionType::Abort:
            {
                currProposed->instructions.push_back(instruction);
                break;
            }

            case lyric_assembler::InstructionType::Invalid:
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

        // set the label if specified
        if (!currProposed->labelName.empty()) {
            auto &currBlock = currProposed->basicBlock;
            currBlock.setLabel(currProposed->labelName);
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

tempo_utils::Result<std::shared_ptr<lyric_optimizer::AbstractDirective>>
translate_int_operation(std::shared_ptr<lyric_assembler::IntOperationInstruction> instruction)
{
    using namespace lyric_optimizer;
    switch (instruction->getOpcode()) {
        case lyric_object::Opcode::OP_I64_ADD:
            return std::static_pointer_cast<AbstractDirective>(std::make_shared<IntAdd>());
        case lyric_object::Opcode::OP_I64_NEG:
            return std::static_pointer_cast<AbstractDirective>(std::make_shared<IntNeg>());
        default:
            return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
                "invalid int operation");
    }
}

tempo_utils::Result<std::shared_ptr<lyric_optimizer::AbstractDirective>>
translate_instruction(
    std::shared_ptr<lyric_assembler::AbstractInstruction> instruction,
    lyric_optimizer::BasicBlock &basicBlock,
    lyric_optimizer::ControlFlowGraph &cfg)
{
    TU_ASSERT (instruction != nullptr);

    using namespace lyric_assembler;
    using namespace lyric_optimizer;

    switch (instruction->getType()) {
        case InstructionType::Noop:
            return std::static_pointer_cast<AbstractDirective>(std::make_shared<Noop>());

        case InstructionType::NilImmediate:
            return std::static_pointer_cast<AbstractDirective>(std::make_shared<Nil>());

        case InstructionType::UndefImmediate:
            return std::static_pointer_cast<AbstractDirective>(std::make_shared<Undef>());

        case InstructionType::BoolImmediate: {
            auto boolImmediate = std::static_pointer_cast<BoolImmediateInstruction>(instruction);
            return std::static_pointer_cast<AbstractDirective>(
                std::make_shared<Bool>(boolImmediate->boolValue()));
        }
        case InstructionType::IntImmediate: {
            auto intImmediate = std::static_pointer_cast<IntImmediateInstruction>(instruction);
            return std::static_pointer_cast<AbstractDirective>(
                std::make_shared<Int>(intImmediate->intValue()));
        }
        case InstructionType::FloatImmediate: {
            auto floatImmediate = std::static_pointer_cast<FloatImmediateInstruction>(instruction);
            return std::static_pointer_cast<AbstractDirective>(
                std::make_shared<Float>(floatImmediate->floatValue()));
        }
        case InstructionType::CharImmediate: {
            auto charImmediate = std::static_pointer_cast<CharImmediateInstruction>(instruction);
            return std::static_pointer_cast<AbstractDirective>(
                std::make_shared<Char>(charImmediate->charValue()));
        }
        case InstructionType::IntOperation: {
            return translate_int_operation(std::static_pointer_cast<IntOperationInstruction>(instruction));
        }
        case InstructionType::LoadData: {
            auto loadData = std::static_pointer_cast<LoadDataInstruction>(instruction);
            auto *symbol = loadData->getSymbol();
            Variable variable;
            switch (symbol->getSymbolType()) {
                case SymbolType::LOCAL: {
                    TU_ASSIGN_OR_RETURN (variable, basicBlock.resolveVariable(cast_symbol_to_local(symbol)));
                    break;
                }
                default:
                    return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
                        "invalid load target {}", symbol->getSymbolUrl().toString());
            }
            return std::static_pointer_cast<AbstractDirective>(
                std::make_shared<LoadLocal>(variable));
        }
        case InstructionType::StoreData: {
            auto storeData = std::static_pointer_cast<StoreDataInstruction>(instruction);
            auto *symbol = storeData->getSymbol();
            Variable variable;
            switch (symbol->getSymbolType()) {
                case SymbolType::LOCAL: {
                    TU_ASSIGN_OR_RETURN (variable, basicBlock.getOrDeclareVariable(cast_symbol_to_local(symbol)));
                    break;
                }
                default:
                    return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
                        "invalid store target {}", symbol->getSymbolUrl().toString());
            }
            return std::static_pointer_cast<AbstractDirective>(
                std::make_shared<StoreLocal>(variable));
        }

        default:
            return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
                "invalid instruction");
    }
}

static tempo_utils::Status
flush_operand_stack(
    std::unique_ptr<ProposedBlock> &proposedBlock,
    lyric_optimizer::OperandStack &stack,
    std::shared_ptr<lyric_optimizer::AbstractDirective> lastDirective = {})
{
    std::stack<std::shared_ptr<lyric_optimizer::AbstractDirective>> directives;

    while (!stack.isEmpty()) {
        directives.push(stack.popOperand());
    }

    auto &basicBlock = proposedBlock->basicBlock;
    while (!directives.empty()) {
        TU_RETURN_IF_NOT_OK (basicBlock.appendDirective(directives.top()));
        directives.pop();
    }

    if (lastDirective != nullptr) {
        basicBlock.appendDirective(lastDirective);
    }

    return {};
}

static tempo_utils::Status
translate_block_instructions(
    const lyric_assembler::ProcBuilder *code,
    std::vector<std::unique_ptr<ProposedBlock>> &proposedBlocks,
    lyric_optimizer::ControlFlowGraph &cfg)
{
    for (auto &currProposed : proposedBlocks) {

        lyric_optimizer::OperandStack stack;
        for (const auto &instruction : currProposed->instructions) {
            std::shared_ptr<lyric_optimizer::AbstractDirective> directive;
            TU_ASSIGN_OR_RETURN (directive, translate_instruction(instruction, currProposed->basicBlock, cfg));
            TU_RETURN_IF_NOT_OK (directive->applyOperands(stack));
            if (directive->isExpression()) {
                stack.pushOperand(directive);
            } else {
                TU_RETURN_IF_NOT_OK (flush_operand_stack(currProposed, stack, directive));
            }
        }
        TU_RETURN_IF_NOT_OK (flush_operand_stack(currProposed, stack));
    }

    return {};
}

tempo_utils::Result<lyric_optimizer::ControlFlowGraph>
lyric_optimizer::parse_proc(const lyric_assembler::ProcHandle *procHandle)
{
    auto *code = procHandle->procCode();
    auto *fragment = code->rootFragment();

    auto numArguments = procHandle->numListParameters() + procHandle->numNamedParameters();
    auto numLocals = procHandle->numLocals();
    auto numLexicals = procHandle->numLexicals();

    ControlFlowGraph cfg(numArguments, numLocals, numLexicals);

    std::vector<std::unique_ptr<ProposedBlock>> proposedBlocks;
    absl::flat_hash_map<std::string, BasicBlock> labelBlocks;
    TU_RETURN_IF_NOT_OK (scan_for_basic_blocks(fragment, proposedBlocks, labelBlocks, cfg));
    TU_RETURN_IF_NOT_OK (apply_control_flow(code, proposedBlocks, labelBlocks, cfg));
    TU_RETURN_IF_NOT_OK (translate_block_instructions(code, proposedBlocks, cfg));

    return cfg;
}
