#ifndef LYRIC_ASSEMBLER_PROC_BUILDER_H
#define LYRIC_ASSEMBLER_PROC_BUILDER_H

#include "code_fragment.h"

namespace lyric_assembler {

    // forward declarations
    class ProcHandle;

    class ProcBuilder {
    public:
        ProcBuilder(ProcHandle *procHandle, ObjectState *state);

        ProcHandle *procHandle() const;
        CodeFragment *rootFragment() const;
        ObjectState *objectState() const;

        absl::flat_hash_set<tu_uint32> getTargetsForLabel(std::string_view labelName) const;
        std::string getLabelForTarget(tu_uint32 targetId) const;

        tempo_utils::Status touch(ObjectWriter &writer) const;
        tempo_utils::Status build(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder) const;

    private:
        ProcHandle *m_procHandle;
        ObjectState *m_state;
        std::unique_ptr<CodeFragment> m_rootFragment;
        tu_uint32 m_nextId;

        struct LabelTargetSet {
            absl::flat_hash_set<tu_uint32> targets;
        };
        absl::flat_hash_map<std::string,LabelTargetSet> m_labelTargets;

        struct JumpLabel {
            std::string name;
        };
        absl::flat_hash_map<tu_uint32,JumpLabel> m_jumpLabels;

        tempo_utils::Result<std::string> makeLabel(std::string_view userLabel = {});
        tempo_utils::Result<tu_uint32> makeJump();
        tempo_utils::Status patchTarget(tu_uint32 targetId, std::string_view labelName);

        friend class CodeFragment;
    };
}

#endif // LYRIC_ASSEMBLER_PROC_BUILDER_H
