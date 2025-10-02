#ifndef LYRIC_ASSEMBLER_PROC_HANDLE_H
#define LYRIC_ASSEMBLER_PROC_HANDLE_H

#include <lyric_common/symbol_url.h>

#include "check_handle.h"
#include "cleanup_handle.h"
#include "object_state.h"
#include "block_handle.h"
#include "proc_builder.h"

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
        const ProcBuilder *procCode() const;
        ProcBuilder *procCode();

        lyric_common::SymbolUrl getActivationUrl() const;
        const CallSymbol *getActivationCall() const;

        int getArity() const;

        int numListParameters() const;
        int numNamedParameters() const;
        bool hasRestParameter() const;

        LocalOffset allocateLocal();
        int numLocals() const;

        LexicalOffset allocateLexical(
            LexicalTarget lexicalTarget,
            uint32_t targetOffset,
            CallSymbol *activationCall);
        std::vector<ProcLexical>::const_iterator lexicalsBegin() const;
        std::vector<ProcLexical>::const_iterator lexicalsEnd() const;
        int numLexicals() const;

        tempo_utils::Result<CheckHandle *> declareCheck(const JumpLabel &checkStart);
        int numChecks() const;

        tempo_utils::Result<CleanupHandle *> declareCleanup(const JumpLabel &cleanupStart);
        int numCleanups() const;

        void putExitType(const lyric_common::TypeDef &exitType);
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator exitTypesBegin() const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator exitTypesEnd() const;

    private:
        lyric_common::SymbolUrl m_activation;
        tu_uint8 m_numListParameters;
        tu_uint8 m_numNamedParameters;
        bool m_hasRestParameter;
        std::unique_ptr<ProcBuilder> m_code;
        std::unique_ptr<BlockHandle> m_block;
        int m_numLocals;
        std::vector<ProcLexical> m_lexicals;
        std::vector<std::unique_ptr<CheckHandle>> m_checks;
        std::vector<std::unique_ptr<CleanupHandle>> m_cleanups;
        absl::flat_hash_set<lyric_common::TypeDef> m_exitTypes;
        ObjectState *m_state;
    };
}

#endif // LYRIC_ASSEMBLER_PROC_HANDLE_H
