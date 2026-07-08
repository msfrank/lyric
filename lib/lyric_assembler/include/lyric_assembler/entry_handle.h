#ifndef LYRIC_ASSEMBLER_ENTRY_HANDLE_H
#define LYRIC_ASSEMBLER_ENTRY_HANDLE_H

#include "abstract_resolver.h"

namespace lyric_assembler {

    class CallSymbol;
    class ProcHandle;

    class EntryHandle {
    public:
        EntryHandle(const std::string &name, CallSymbol *callSymbol);

        std::string getName() const;

        lyric_common::SymbolUrl getSymbolUrl() const;

        AbstractResolver *entryResolver() const;
        const ProcHandle *entryProc() const;

        ProcHandle *entryProc();

        tempo_utils::Result<lyric_common::TypeDef> finalizeEntry();

    private:
        std::string m_name;
        CallSymbol *m_callSymbol;
    };
}

#endif // LYRIC_ASSEMBLER_ENTRY_HANDLE_H
