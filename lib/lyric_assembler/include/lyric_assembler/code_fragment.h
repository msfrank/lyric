#ifndef LYRIC_ASSEMBLER_CODE_FRAGMENT_H
#define LYRIC_ASSEMBLER_CODE_FRAGMENT_H

#include <vector>

#include <lyric_object/bytecode_builder.h>

#include "abstract_instruction.h"

namespace lyric_assembler {

    // forward declarations
    class ProcBuilder;
    class CodeFragment;

    enum class StatementType {
        Instruction,
        Fragment,
    };

    struct Statement {
        StatementType type;
        std::shared_ptr<AbstractInstruction> instruction;
        std::unique_ptr<CodeFragment> fragment;
    };

    class CodeFragment {
    public:
        tempo_utils::Status insertInstruction(int index, std::shared_ptr<AbstractInstruction> instruction);
        tempo_utils::Status appendInstruction(std::shared_ptr<AbstractInstruction> instruction);

        tempo_utils::Result<JumpLabel> insertLabel(int index, std::string_view userLabel = {});
        tempo_utils::Result<JumpLabel> appendLabel(std::string_view userLabel = {});

        std::unique_ptr<CodeFragment> makeFragment();
        tempo_utils::Status insertFragment(int index, std::unique_ptr<CodeFragment> &&fragment);
        tempo_utils::Status appendFragment(std::unique_ptr<CodeFragment> &&fragment);

        const Statement& getStatement(int index) const;
        void removeStatement(int index);
        std::vector<Statement>::const_iterator statementsBegin() const;
        std::vector<Statement>::const_iterator statementsEnd() const;
        int numStatements() const;

        tempo_utils::Status noOperation();

        // immediate instructions
        tempo_utils::Status immediateNil();
        tempo_utils::Status immediateUndef();
        tempo_utils::Status immediateBool(bool b);
        tempo_utils::Status immediateInt(int64_t i64);
        tempo_utils::Status immediateFloat(double dbl);
        tempo_utils::Status immediateChar(UChar32 chr);

        // load and store instructions
        tempo_utils::Status loadString(const std::string &str);
        tempo_utils::Status loadUrl(const tempo_utils::Url &url);
        tempo_utils::Status loadData(AbstractSymbol *symbol);
        tempo_utils::Status loadDescriptor(AbstractSymbol *symbol);
        tempo_utils::Status loadRef(const DataReference &ref);
        tempo_utils::Status loadThis();
        tempo_utils::Status loadType(const lyric_common::TypeDef &loadType);
        tempo_utils::Status storeData(AbstractSymbol *symbol);
        tempo_utils::Status storeRef(const DataReference &ref);

        // stack manipulation instructions
        tempo_utils::Status popValue();
        tempo_utils::Status dupValue();
        tempo_utils::Status pickValue(tu_uint16 pickOffset);
        tempo_utils::Status dropValue(tu_uint16 dropOffset);
        tempo_utils::Status rpickValue(tu_uint16 pickOffset);
        tempo_utils::Status rdropValue(tu_uint16 dropOffset);

        // integer math
        tempo_utils::Status intAdd();
        tempo_utils::Status intSubtract();
        tempo_utils::Status intMultiply();
        tempo_utils::Status intDivide();
        tempo_utils::Status intNegate();

        // rational math
        tempo_utils::Status floatAdd();
        tempo_utils::Status floatSubtract();
        tempo_utils::Status floatMultiply();
        tempo_utils::Status floatDivide();
        tempo_utils::Status floatNegate();

        // comparisons
        tempo_utils::Status boolCompare();
        tempo_utils::Status intCompare();
        tempo_utils::Status floatCompare();
        tempo_utils::Status charCompare();
        tempo_utils::Status typeCompare();

        // logical operations
        tempo_utils::Status logicalAnd();
        tempo_utils::Status logicalOr();
        tempo_utils::Status logicalNot();

        // branching instructions
        tempo_utils::Result<JumpTarget> unconditionalJump();
        tempo_utils::Result<JumpTarget> jumpIfNil();
        tempo_utils::Result<JumpTarget> jumpIfNotNil();
        tempo_utils::Result<JumpTarget> jumpIfTrue();
        tempo_utils::Result<JumpTarget> jumpIfFalse();
        tempo_utils::Result<JumpTarget> jumpIfZero();
        tempo_utils::Result<JumpTarget> jumpIfNotZero();
        tempo_utils::Result<JumpTarget> jumpIfLessThan();
        tempo_utils::Result<JumpTarget> jumpIfLessOrEqual();
        tempo_utils::Result<JumpTarget> jumpIfGreaterThan();
        tempo_utils::Result<JumpTarget> jumpIfGreaterOrEqual();

        // target patching
        tempo_utils::Status patchTarget(JumpTarget jumpTarget, JumpLabel jumpLabel);

        // call instructions
        tempo_utils::Status callStatic(CallSymbol *callSymbol, tu_uint16 placement, tu_uint8 flags);
        tempo_utils::Status callVirtual(CallSymbol *callSymbol, tu_uint16 placement, tu_uint8 flags);
        tempo_utils::Status callConcept(ActionSymbol *actionSymbol, tu_uint16 placement, tu_uint8 flags);
        tempo_utils::Status callExistential(CallSymbol *callSymbol, tu_uint16 placement, tu_uint8 flags);

        // new instruction
        tempo_utils::Status constructNew(AbstractSymbol *newSymbol, tu_uint16 placement, tu_uint8 flags);

        // trap instruction
        tempo_utils::Status trap(tu_uint32 trapNumber, tu_uint8 flags);

        // return instruction
        tempo_utils::Status returnToCaller();

        // interpreter services instructions
        tempo_utils::Status invokeTypeOf();
        tempo_utils::Status invokeInterrupt();
        tempo_utils::Status invokeHalt();
        tempo_utils::Status invokeAbort();

    private:
        ProcBuilder *m_procBuilder;
        std::vector<Statement> m_statements;

        tempo_utils::Result<JumpTarget> makeJump(lyric_object::Opcode opcode);
        tempo_utils::Status touch(ObjectWriter &writer) const;
        tempo_utils::Status build(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            absl::flat_hash_map<std::string,tu_uint16> &labelOffsets,
            absl::flat_hash_map<tu_uint32,tu_uint16> &patchOffsets) const;

        explicit CodeFragment(ProcBuilder *procBuilder);
        friend class ProcBuilder;
        friend class MacroInstruction;
    };
}

#endif // LYRIC_ASSEMBLER_CODE_FRAGMENT_H
