#ifndef LYRIC_ASSEMBLER_PROC_HANDLE_H
#define LYRIC_ASSEMBLER_PROC_HANDLE_H

#include <lyric_common/symbol_url.h>

#include "assembly_state.h"
#include "block_handle.h"
#include "code_builder.h"

namespace lyric_assembler {

    enum class LexicalTarget {
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
        ProcHandle(const lyric_common::SymbolUrl &activation, const std::vector<tu_uint8> &bytecode);
        ProcHandle(
            const lyric_common::SymbolUrl &activation,
            const std::vector<tu_uint8> &bytecode,
            int numLocals);
        ProcHandle(const lyric_common::SymbolUrl &activation, AssemblyState *state);
        ProcHandle(
            const lyric_common::SymbolUrl &activation,
            const absl::flat_hash_map<std::string, SymbolBinding> &parameters,
            int arity,
            AssemblyState *state,
            BlockHandle *parent);
        ~ProcHandle();

        BlockHandle *procBlock();
        CodeBuilder *procCode();

        lyric_common::SymbolUrl getActivation() const;
        int getArity() const;
        LocalOffset allocateLocal();
        int numLocals() const;

        LexicalOffset allocateLexical(
            LexicalTarget lexicalTarget,
            uint32_t targetOffset,
            CallAddress activationCall);
        std::vector<ProcLexical>::const_iterator lexicalsBegin() const;
        std::vector<ProcLexical>::const_iterator lexicalsEnd() const;
        int numLexicals() const;

    private:
        lyric_common::SymbolUrl m_activation;
        int m_arity;
        CodeBuilder m_code;
        BlockHandle *m_block;
        int m_numLocals;
        std::vector<ProcLexical> m_lexicals;
    };
}

#endif // LYRIC_ASSEMBLER_PROC_HANDLE_H
