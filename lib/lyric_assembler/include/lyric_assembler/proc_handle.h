#ifndef LYRIC_ASSEMBLER_PROC_HANDLE_H
#define LYRIC_ASSEMBLER_PROC_HANDLE_H

#include <lyric_common/symbol_url.h>

#include "check_handle.h"
#include "cleanup_handle.h"
#include "object_state.h"
#include "block_handle.h"

namespace lyric_assembler {

    class CheckHandle;
    class CleanupHandle;

    enum class LexicalTarget {
        Invalid,
        Argument,
        Local,
        Lexical,
    };

    struct ProcLexical {
        LexicalTarget lexicalTarget;
        uint32_t targetOffset;
        CallSymbol *activationCall;
    };

    class ProcHandle {

    public:
        ProcHandle(const lyric_common::SymbolUrl &activation, ObjectState *state);
        ProcHandle(
            const lyric_common::SymbolUrl &activation,
            BlockHandle *parent,
            ObjectState *state);
        ProcHandle(
            const lyric_common::SymbolUrl &activation,
            const absl::flat_hash_map<std::string, SymbolBinding> &initialBindings,
            tu_uint8 numListParameters,
            tu_uint8 numNamedParameters,
            bool hasRestParameter,
            ObjectState *state,
            BlockHandle *parent);

        const BlockHandle *procBlock() const;
        BlockHandle *procBlock();
        const CodeFragment *procFragment() const;
        CodeFragment *procFragment();
        ObjectState *objectState() const;

        lyric_common::SymbolUrl getActivationUrl() const;
        const CallSymbol *getActivationCall() const;

        /*
         * proc parameters
         */

        int numListParameters() const;
        int numNamedParameters() const;
        bool hasRestParameter() const;
        int getArity() const;

        /*
         * proc variables
         */

        LocalOffset allocateLocal();
        int numLocals() const;

        LexicalOffset allocateLexical(
            LexicalTarget lexicalTarget,
            uint32_t targetOffset,
            CallSymbol *activationCall);
        std::vector<ProcLexical>::const_iterator lexicalsBegin() const;
        std::vector<ProcLexical>::const_iterator lexicalsEnd() const;
        int numLexicals() const;

        /*
         * labels and jumps
         */

        tempo_utils::Result<std::string> makeLabel(std::string_view userLabel = {});
        tempo_utils::Result<tu_uint32> makeJump();
        absl::flat_hash_set<tu_uint32> getTargetsForLabel(std::string_view labelName) const;
        std::string getLabelForTarget(tu_uint32 targetId) const;
        tempo_utils::Status patchTarget(tu_uint32 targetId, std::string_view labelName);

        /*
         * exceptions and cleanup
         */

        tempo_utils::Result<CheckHandle *> declareCheck(const JumpLabel &checkStart);
        int numChecks() const;

        tempo_utils::Result<CleanupHandle *> declareCleanup(const JumpLabel &cleanupStart);
        int numCleanups() const;

        /*
         * proc exit
         */

        void putExitType(const lyric_common::TypeDef &exitType);
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator exitTypesBegin() const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator exitTypesEnd() const;

        /*
         * serialization
         */

        tempo_utils::Status touch(ObjectWriter &writer) const;
        tempo_utils::Status build(
            const ObjectWriter &writer,
            lyric_object::BytecodeBuilder &bytecodeBuilder) const;

    private:
        lyric_common::SymbolUrl m_activation;
        tu_uint8 m_numListParameters;
        tu_uint8 m_numNamedParameters;
        bool m_hasRestParameter;
        std::unique_ptr<BlockHandle> m_block;
        std::unique_ptr<CodeFragment> m_fragment;
        ObjectState *m_state;

        int m_numLocals;
        std::vector<ProcLexical> m_lexicals;

        std::vector<std::unique_ptr<CheckHandle>> m_checks;
        std::vector<std::unique_ptr<CleanupHandle>> m_cleanups;

        absl::flat_hash_set<lyric_common::TypeDef> m_exitTypes;

        tu_uint32 m_nextId;
        absl::flat_hash_map<std::string, absl::flat_hash_set<tu_uint32>> m_labelTargets;
        absl::flat_hash_map<tu_uint32,std::string> m_jumpLabels;

    };
}

#endif // LYRIC_ASSEMBLER_PROC_HANDLE_H
