#ifndef LYRIC_ASSEMBLER_SYMBOL_CACHE_H
#define LYRIC_ASSEMBLER_SYMBOL_CACHE_H

#include "abstract_symbol.h"
#include "assembler_tracer.h"
#include "assembler_types.h"

namespace lyric_assembler {

    class SymbolCache {
    public:
        explicit SymbolCache(AssemblerTracer *tracer);
        ~SymbolCache();

        bool hasSymbol(const lyric_common::SymbolUrl &symbolUrl) const;
        AbstractSymbol *getSymbol(const lyric_common::SymbolUrl &symbolUrl) const;
        tempo_utils::Status insertSymbol(const lyric_common::SymbolUrl &symbolUrl, AbstractSymbol *abstractSymbol);
        tempo_utils::Status touchSymbol(const lyric_common::SymbolUrl &sym);
        int numSymbols() const;

        bool hasEnvBinding(const std::string &name) const;
        SymbolBinding getEnvBinding(const std::string &name) const;
        tempo_utils::Status insertEnvBinding(const std::string &name, const SymbolBinding &binding);

        bool hasEnvInstance(const lyric_common::TypeDef &type) const;
        lyric_common::SymbolUrl getEnvInstance(const lyric_common::TypeDef &type) const;
        tempo_utils::Status insertEnvInstance(const lyric_common::TypeDef &type, const lyric_common::SymbolUrl &url);

    private:
        AssemblerTracer *m_tracer;
        absl::flat_hash_map<lyric_common::SymbolUrl, AbstractSymbol *> m_symcache;
        absl::flat_hash_map<std::string, SymbolBinding> m_envBindings;
        absl::flat_hash_map<lyric_common::TypeDef, lyric_common::SymbolUrl> m_envInstances;
    };
}

#endif // LYRIC_ASSEMBLER_SYMBOL_CACHE_H
