#ifndef LYRIC_ASSEMBLER_CODE_FRAGMENT_H
#define LYRIC_ASSEMBLER_CODE_FRAGMENT_H

#include <vector>

#include <lyric_object/bytecode_builder.h>

#include "assembler_instructions.h"
#include "proc_handle.h"

namespace lyric_assembler {

    class CodeFragment {
    public:
        explicit CodeFragment(ProcHandle *procHandle);

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

        //
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

        //
        tempo_utils::Status appendLabel(std::string_view labelName);
        tempo_utils::Status patchTarget(tu_uint32 targetId, std::string_view labelName);

        tempo_utils::Status write(lyric_object::BytecodeBuilder &bytecodeBuilder) const;

    private:
        ProcHandle *m_procHandle;
        std::vector<std::shared_ptr<AbstractInstruction>> m_instructions;
        //absl::flat_hash_map<std::string,std::shared_ptr<LabelInstruction>> m_labels;
        tu_uint32 m_nextTargetId;

        struct LabelTargetSet {
            absl::flat_hash_set<tu_uint32> targets;
        };
        absl::flat_hash_map<std::string,LabelTargetSet> m_labelTargets;

        struct JumpLabel {
            std::string name;
        };
        absl::flat_hash_map<tu_uint32,JumpLabel> m_jumpLabels;

        tempo_utils::Result<tu_uint32> makeJump(lyric_object::Opcode opcode);
    };
}

#endif // LYRIC_ASSEMBLER_CODE_FRAGMENT_H
