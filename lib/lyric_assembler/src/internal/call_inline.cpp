
#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/assembler_instructions.h>
#include <lyric_assembler/assembler_result.h>
#include <lyric_assembler/code_fragment.h>
#include <lyric_assembler/internal/call_inline.h>
#include <lyric_assembler/proc_builder.h>

tempo_utils::Status
lyric_assembler::internal::call_inline(
    CallSymbol *callSymbol,
    BlockHandle *block,
    CodeFragment *fragment)
{
    TU_ASSERT (callSymbol != nullptr);
    if (!callSymbol->isInline())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for inline call");
    if (callSymbol->restPlacement() != nullptr)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "invalid symbol for inline call");

    auto *dstFragment = fragment;

    absl::flat_hash_map<tu_uint32,DataReference> argToTmp;
    std::stack<tu_uint32> args;

    // allocate a temporary for each list argument
    for (auto it = callSymbol->listPlacementBegin(); it != callSymbol->listPlacementEnd(); it++) {
        auto &p = *it;
        TU_ASSERT (!argToTmp.contains(p.index));
        tu_uint32 address = p.index;
        DataReference ref;
        TU_ASSIGN_OR_RETURN (ref, block->declareTemporary(p.typeDef, /* isVariable= */ true));
        argToTmp[address] = ref;
        args.push(address);
    }

    // allocate a temporary for each named argument
    for (auto it = callSymbol->namedPlacementBegin(); it != callSymbol->namedPlacementEnd(); it++) {
        auto &p = *it;
        TU_ASSERT (!argToTmp.contains(p.index));
        tu_uint32 address = p.index;
        DataReference ref;
        TU_ASSIGN_OR_RETURN (ref, block->declareTemporary(p.typeDef, /* isVariable= */ true));
        argToTmp[address] = ref;
        args.push(address);
    }

    // store each argument in the associated temporary
    while (!args.empty()) {
        auto address = args.top();
        args.pop();
        TU_RETURN_IF_NOT_OK (dstFragment->storeRef(argToTmp[address]));
    }

    auto *srcProc = callSymbol->callProc();
    auto *srcCode = srcProc->procCode();
    auto *srcFragment = srcCode->rootFragment();

    absl::flat_hash_map<std::string,JumpLabel> labels;
    absl::flat_hash_map<tu_uint32,JumpTarget> targets;

    // copy instructions from src proc to dst proc
    for (auto it = srcFragment->statementsBegin(); it != srcFragment->statementsEnd(); it++) {
        const auto &statement = *it;

        if (statement.type != StatementType::Instruction)
            return AssemblerStatus::forCondition(
                AssemblerCondition::kAssemblerInvariant, "unhandled instruction for inline call");

        switch (statement.instruction->getType()) {

            // rewrite argument load
            case InstructionType::LoadData: {
                auto instruction = std::dynamic_pointer_cast<LoadDataInstruction>(statement.instruction);
                auto *symbol = instruction->getSymbol();
                if (symbol->getSymbolType() == SymbolType::ARGUMENT) {
                    auto *argumentVariable = cast_symbol_to_argument(symbol);
                    auto address = argumentVariable->getOffset().getOffset();
                    auto entry = argToTmp.find(address);
                    if (entry == argToTmp.cend())
                        return AssemblerStatus::forCondition(
                            AssemblerCondition::kAssemblerInvariant, "invalid argument address");
                    TU_RETURN_IF_NOT_OK (dstFragment->loadRef(entry->second));
                } else {
                    dstFragment->appendInstruction(statement.instruction);
                }
                break;
            }

            // rewrite argument store
            case InstructionType::StoreData: {
                auto instruction = std::dynamic_pointer_cast<StoreDataInstruction>(statement.instruction);
                auto *symbol = instruction->getSymbol();
                if (symbol->getSymbolType() == SymbolType::ARGUMENT) {
                    auto *argumentVariable = cast_symbol_to_argument(symbol);
                    auto address = argumentVariable->getOffset().getOffset();
                    auto entry = argToTmp.find(address);
                    if (entry == argToTmp.cend())
                        return AssemblerStatus::forCondition(
                            AssemblerCondition::kAssemblerInvariant, "invalid argument address");
                    TU_RETURN_IF_NOT_OK (dstFragment->storeRef(entry->second));
                } else {
                    dstFragment->appendInstruction(statement.instruction);
                }
                break;
            }

            case InstructionType::Label: {
                auto instruction = std::dynamic_pointer_cast<LabelInstruction>(statement.instruction);
                auto srcLabel = instruction->getName();
                JumpLabel dstLabel;
                TU_ASSIGN_OR_RETURN (dstLabel, dstFragment->appendLabel());
                labels[srcLabel] = dstLabel;
                break;
            }

            case InstructionType::Jump: {
                auto instruction = std::dynamic_pointer_cast<JumpInstruction>(statement.instruction);
                auto srcTarget = instruction->getTargetId();
                JumpTarget dstTarget;
                switch (instruction->getOpcode()) {
                    case lyric_object::Opcode::OP_IF_NIL:
                        TU_ASSIGN_OR_RETURN (dstTarget, dstFragment->jumpIfNil());
                        break;
                    case lyric_object::Opcode::OP_IF_NOTNIL:
                        TU_ASSIGN_OR_RETURN (dstTarget, dstFragment->jumpIfNotNil());
                        break;
                    case lyric_object::Opcode::OP_IF_TRUE:
                        TU_ASSIGN_OR_RETURN (dstTarget, dstFragment->jumpIfTrue());
                        break;
                    case lyric_object::Opcode::OP_IF_FALSE:
                        TU_ASSIGN_OR_RETURN (dstTarget, dstFragment->jumpIfFalse());
                        break;
                    case lyric_object::Opcode::OP_IF_ZERO:
                        TU_ASSIGN_OR_RETURN (dstTarget, dstFragment->jumpIfZero());
                        break;
                    case lyric_object::Opcode::OP_IF_NOTZERO:
                        TU_ASSIGN_OR_RETURN (dstTarget, dstFragment->jumpIfNotZero());
                        break;
                    case lyric_object::Opcode::OP_IF_GT:
                        TU_ASSIGN_OR_RETURN (dstTarget, dstFragment->jumpIfGreaterThan());
                        break;
                    case lyric_object::Opcode::OP_IF_GE:
                        TU_ASSIGN_OR_RETURN (dstTarget, dstFragment->jumpIfGreaterOrEqual());
                        break;
                    case lyric_object::Opcode::OP_IF_LT:
                        TU_ASSIGN_OR_RETURN (dstTarget, dstFragment->jumpIfLessThan());
                        break;
                    case lyric_object::Opcode::OP_IF_LE:
                        TU_ASSIGN_OR_RETURN (dstTarget, dstFragment->jumpIfLessOrEqual());
                        break;
                    case lyric_object::Opcode::OP_JUMP:
                        TU_ASSIGN_OR_RETURN (dstTarget, dstFragment->unconditionalJump());
                        break;
                    default:
                        return AssemblerStatus::forCondition(
                            AssemblerCondition::kAssemblerInvariant, "invalid jump instruction");
                }
                targets[srcTarget] = dstTarget;
                break;
            }

            default:
                dstFragment->appendInstruction(statement.instruction);
                break;
        }
    }

    // patch jump targets
    for (const auto &entry: targets) {
        auto srcTargetId = entry.first;
        auto &dstTarget = entry.second;
        auto srcLabel = srcCode->getLabelForTarget(srcTargetId);
        auto &dstLabel = labels.at(srcLabel);
        TU_RETURN_IF_NOT_OK (dstFragment->patchTarget(dstTarget, dstLabel));
    }

    return {};
}