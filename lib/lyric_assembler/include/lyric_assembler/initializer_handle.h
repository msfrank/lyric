#ifndef LYRIC_ASSEMBLER_INITIALIZER_HANDLE_H
#define LYRIC_ASSEMBLER_INITIALIZER_HANDLE_H

#include "abstract_resolver.h"
#include "abstract_symbol.h"
#include "proc_handle.h"
#include "template_handle.h"

namespace lyric_assembler {

    class CallSymbol;

    class InitializerHandle {
    public:
        InitializerHandle(const std::string &name, CallSymbol *callSymbol);

        std::string getName() const;

        lyric_common::SymbolUrl getSymbolUrl() const;

        AbstractResolver *initializerResolver() const;
        TemplateHandle *initializerTemplate() const;
        const ProcHandle *initializerProc() const;

        TypeHandle *initializerType();
        ProcHandle *initializerProc();

        tempo_utils::Result<lyric_common::TypeDef> finalizeInitializer();

    private:
        std::string m_name;
        CallSymbol *m_callSymbol;
    };
}

#endif // LYRIC_ASSEMBLER_INITIALIZER_HANDLE_H
