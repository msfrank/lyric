#ifndef LYRIC_ASSEMBLER_SYMBOL_CACHE_H
#define LYRIC_ASSEMBLER_SYMBOL_CACHE_H

#include "abstract_symbol.h"
#include "assembler_types.h"
#include "object_state.h"

namespace lyric_assembler {

    class SymbolCache {
    public:
        explicit SymbolCache(ObjectState *state);
        ~SymbolCache();

        tempo_utils::Status insertSymbol(
            const lyric_common::SymbolUrl &symbolUrl,
            AbstractSymbol *abstractSymbol,
            TypenameSymbol *existingTypename = nullptr);

        bool hasSymbol(const lyric_common::SymbolUrl &symbolUrl) const;
        AbstractSymbol *getSymbolOrNull(const lyric_common::SymbolUrl &symbolUrl) const;
        tempo_utils::Result<AbstractSymbol *> getOrImportSymbol(const lyric_common::SymbolUrl &symbolUrl) const;
        absl::flat_hash_map<lyric_common::SymbolUrl, AbstractSymbol *>::const_iterator symbolsBegin() const;
        absl::flat_hash_map<lyric_common::SymbolUrl, AbstractSymbol *>::const_iterator symbolsEnd() const;
        int numSymbols() const;

        tempo_utils::Result<TypenameSymbol *> putTypename(const lyric_common::SymbolUrl &typenameUrl);

//        bool hasEnvBinding(const std::string &name) const;
//        SymbolBinding getEnvBinding(const std::string &name) const;
//        tempo_utils::Status insertEnvBinding(const std::string &name, const SymbolBinding &binding);
//
//        bool hasEnvInstance(const lyric_common::TypeDef &type) const;
//        lyric_common::SymbolUrl getEnvInstance(const lyric_common::TypeDef &type) const;
//        tempo_utils::Status insertEnvInstance(const lyric_common::TypeDef &type, const lyric_common::SymbolUrl &url);

    private:
        ObjectState *m_state;
        absl::flat_hash_map<lyric_common::SymbolUrl, AbstractSymbol *> m_symcache;
        std::queue<TypenameSymbol *> m_typenames;
    };
}

#endif // LYRIC_ASSEMBLER_SYMBOL_CACHE_H
