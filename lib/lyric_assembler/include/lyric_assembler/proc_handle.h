#ifndef LYRIC_ASSEMBLER_PROC_HANDLE_H
#define LYRIC_ASSEMBLER_PROC_HANDLE_H

#include <lyric_common/symbol_url.h>

#include "object_state.h"
#include "block_handle.h"
#include "proc_builder.h"

namespace lyric_assembler {

    enum class LexicalTarget {
        Invalid,
        Argument,
        Local,
        Lexical,
    };

    struct ProcLexical {
        LexicalTarget lexicalTarget;
        uint32_t targetOffset;
        CallAddress activationCall;
    };

    class ProcHandle {

    public:
        explicit ProcHandle(const lyric_common::SymbolUrl &activation);
//        ProcHandle(const lyric_common::SymbolUrl &activation, const std::vector<tu_uint8> &bytecode);
//        ProcHandle(
//            const lyric_common::SymbolUrl &activation,
//            const std::vector<tu_uint8> &bytecode,
//            int numLocals);
        ProcHandle(const lyric_common::SymbolUrl &activation, ObjectState *state);
        ProcHandle(
            const lyric_common::SymbolUrl &activation,
            const absl::flat_hash_map<std::string, SymbolBinding> &initialBindings,
            tu_uint8 numListParameters,
            tu_uint8 numNamedParameters,
            bool hasRestParameter,
            ObjectState *state,
            BlockHandle *parent);

        BlockHandle *procBlock();
        ProcBuilder *procCode();

        lyric_common::SymbolUrl getActivation() const;
        int getArity() const;
        LocalOffset allocateLocal();
        int numLocals() const;
        int numListParameters() const;
        int numNamedParameters() const;
        bool hasRestParameter() const;

        LexicalOffset allocateLexical(
            LexicalTarget lexicalTarget,
            uint32_t targetOffset,
            CallAddress activationCall);
        std::vector<ProcLexical>::const_iterator lexicalsBegin() const;
        std::vector<ProcLexical>::const_iterator lexicalsEnd() const;
        int numLexicals() const;

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
        absl::flat_hash_set<lyric_common::TypeDef> m_exitTypes;
    };
}

#endif // LYRIC_ASSEMBLER_PROC_HANDLE_H
