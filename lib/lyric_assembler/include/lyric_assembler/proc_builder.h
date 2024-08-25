#ifndef LYRIC_ASSEMBLER_PROC_BUILDER_H
#define LYRIC_ASSEMBLER_PROC_BUILDER_H

#include "code_fragment.h"

namespace lyric_assembler {

    // forward declarations
    class ProcHandle;

    class ProcBuilder {
    public:
        explicit ProcBuilder(ProcHandle *procHandle);

        ProcHandle *procHandle() const;
        CodeFragment *rootFragment() const;
        ObjectState *objectState() const;

        tempo_utils::Status build(lyric_object::BytecodeBuilder &bytecodeBuilder) const;

    private:
        ProcHandle *m_procHandle;
        std::unique_ptr<CodeFragment> m_rootFragment;
        ObjectState *m_state;
        tu_uint32 m_nextTargetId;

        struct LabelTargetSet {
            absl::flat_hash_set<tu_uint32> targets;
        };
        absl::flat_hash_map<std::string,LabelTargetSet> m_labelTargets;

        struct JumpLabel {
            std::string name;
        };
        absl::flat_hash_map<tu_uint32,JumpLabel> m_jumpLabels;

        tempo_utils::Status appendLabel(std::string_view labelName);
        tempo_utils::Result<tu_uint32> makeJump();
        tempo_utils::Status patchTarget(tu_uint32 targetId, std::string_view labelName);

        friend class CodeFragment;
    };
}

#endif // LYRIC_ASSEMBLER_PROC_BUILDER_H
