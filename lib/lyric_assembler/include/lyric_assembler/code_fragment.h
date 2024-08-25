#ifndef LYRIC_ASSEMBLER_CODE_FRAGMENT_H
#define LYRIC_ASSEMBLER_CODE_FRAGMENT_H

#include <vector>

#include <lyric_object/bytecode_builder.h>

#include "abstract_instruction.h"
#include "proc_handle.h"

namespace lyric_assembler {

    // forward declarations
    class ProcBuilder;

    class CodeFragment {
    public:
        std::unique_ptr<CodeFragment> makeFragment();
        tempo_utils::Status appendFragment(std::unique_ptr<CodeFragment> &&fragment);

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
        tempo_utils::Status loadRef(const DataReference &ref);
        tempo_utils::Status loadType(const lyric_common::TypeDef &loadType);
        tempo_utils::Status storeRef(const DataReference &ref);

        // stack manipulation instructions
        tempo_utils::Status popValue();
        tempo_utils::Status dupValue();
        tempo_utils::Status pickValue(tu_uint16 pickOffset);
        tempo_utils::Status dropValue(tu_uint16 dropOffset);
        tempo_utils::Status rpickValue(tu_uint16 pickOffset);
        tempo_utils::Status rdropValue(tu_uint16 dropOffset);

        // branching instructions
        tempo_utils::Result<tu_uint32> unconditionalJump();
        tempo_utils::Result<tu_uint32> jumpIfNil();
        tempo_utils::Result<tu_uint32> jumpIfNotNil();
        tempo_utils::Result<tu_uint32> jumpIfTrue();
        tempo_utils::Result<tu_uint32> jumpIfFalse();
        tempo_utils::Result<tu_uint32> jumpIfZero();
        tempo_utils::Result<tu_uint32> jumpIfNotZero();
        tempo_utils::Result<tu_uint32> jumpIfLessThan();
        tempo_utils::Result<tu_uint32> jumpIfLessOrEqual();
        tempo_utils::Result<tu_uint32> jumpIfGreaterThan();
        tempo_utils::Result<tu_uint32> jumpIfGreaterOrEqual();

        // target patching
        tempo_utils::Status appendLabel(std::string_view labelName);
        tempo_utils::Status patchTarget(tu_uint32 targetId, std::string_view labelName);

        // call instructions
        tempo_utils::Status callStatic(CallSymbol *callSymbol, tu_uint16 placement, tu_uint8 flags);
        tempo_utils::Status callVirtual(CallSymbol *callSymbol, tu_uint16 placement, tu_uint8 flags);
        tempo_utils::Status callConcept(ActionSymbol *actionSymbol, tu_uint16 placement, tu_uint8 flags);
        tempo_utils::Status callExistential(ExistentialSymbol *existentialSymbol, tu_uint16 placement, tu_uint8 flags);
        tempo_utils::Status callInline(CallSymbol *callSymbol);

        // new instruction
        tempo_utils::Status constructNew(AbstractSymbol *newSymbol, tu_uint16 placement, tu_uint8 flags);

        // trap instruction
        tempo_utils::Status trap(tu_uint32 trapNumber, tu_uint8 flags);

    private:
        ProcBuilder *m_procBuilder;

        enum class StatementType {
            Instruction,
            Fragment,
        };
        struct Statement {
            StatementType type;
            std::shared_ptr<AbstractInstruction> instruction;
            std::unique_ptr<CodeFragment> fragment;
        };
        std::vector<Statement> m_statements;

        tempo_utils::Result<tu_uint32> makeJump(lyric_object::Opcode opcode);
        tempo_utils::Status build(
            lyric_object::BytecodeBuilder &bytecodeBuilder,
            absl::flat_hash_map<std::string,tu_uint16> &labelOffsets,
            absl::flat_hash_map<tu_uint32,tu_uint16> &patchOffsets) const;

        explicit CodeFragment(ProcBuilder *procBuilder);
        friend class ProcBuilder;
        friend class MacroInstruction;
    };
}

#endif // LYRIC_ASSEMBLER_CODE_FRAGMENT_H
