#ifndef LYRIC_ASSEMBLER_SYMBOL_CACHE_H
#define LYRIC_ASSEMBLER_SYMBOL_CACHE_H

#include "abstract_symbol.h"
#include "assembler_tracer.h"
#include "assembler_types.h"
#include "object_state.h"

namespace lyric_assembler {

    class SymbolCache {
    public:
        SymbolCache(ObjectState *state, AssemblerTracer *tracer);
        ~SymbolCache();

        tempo_utils::Status insertSymbol(
            const lyric_common::SymbolUrl &symbolUrl,
            AbstractSymbol *abstractSymbol,
            LinkageSymbol *existingLinkage = nullptr);

        bool hasSymbol(const lyric_common::SymbolUrl &symbolUrl) const;
        AbstractSymbol *getSymbolOrNull(const lyric_common::SymbolUrl &symbolUrl) const;
        tempo_utils::Result<AbstractSymbol *> getOrImportSymbol(const lyric_common::SymbolUrl &symbolUrl) const;
        absl::flat_hash_map<lyric_common::SymbolUrl, AbstractSymbol *>::const_iterator symbolsBegin() const;
        absl::flat_hash_map<lyric_common::SymbolUrl, AbstractSymbol *>::const_iterator symbolsEnd() const;
        int numSymbols() const;

//        bool hasEnvBinding(const std::string &name) const;
//        SymbolBinding getEnvBinding(const std::string &name) const;
//        tempo_utils::Status insertEnvBinding(const std::string &name, const SymbolBinding &binding);
//
//        bool hasEnvInstance(const lyric_common::TypeDef &type) const;
//        lyric_common::SymbolUrl getEnvInstance(const lyric_common::TypeDef &type) const;
//        tempo_utils::Status insertEnvInstance(const lyric_common::TypeDef &type, const lyric_common::SymbolUrl &url);

    private:
        ObjectState *m_state;
        AssemblerTracer *m_tracer;
        absl::flat_hash_map<lyric_common::SymbolUrl, AbstractSymbol *> m_symcache;
        std::queue<AbstractSymbol *> m_removed;
//        absl::flat_hash_map<std::string, SymbolBinding> m_envBindings;
//        absl::flat_hash_map<lyric_common::TypeDef, lyric_common::SymbolUrl> m_envInstances;
    };
}

#endif // LYRIC_ASSEMBLER_SYMBOL_CACHE_H
