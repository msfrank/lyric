#ifndef LYRIC_ASSEMBLER_SYMBOL_CACHE_H
#define LYRIC_ASSEMBLER_SYMBOL_CACHE_H

#include <queue>

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

        tempo_utils::Result<ActionSymbol *> getOrImportAction(const lyric_common::SymbolUrl &actionUrl) const;
        tempo_utils::Result<BindingSymbol *> getOrImportBinding(const lyric_common::SymbolUrl &bindingUrl) const;
        tempo_utils::Result<CallSymbol *> getOrImportCall(const lyric_common::SymbolUrl &callUrl) const;
        tempo_utils::Result<ClassSymbol *> getOrImportClass(const lyric_common::SymbolUrl &classUrl) const;
        tempo_utils::Result<ConceptSymbol *> getOrImportConcept(const lyric_common::SymbolUrl &conceptUrl) const;
        tempo_utils::Result<EnumSymbol *> getOrImportEnum(const lyric_common::SymbolUrl &enumUrl) const;
        tempo_utils::Result<ExistentialSymbol *> getOrImportExistential(const lyric_common::SymbolUrl &existentialUrl) const;
        tempo_utils::Result<FieldSymbol *> getOrImportField(const lyric_common::SymbolUrl &fieldUrl) const;
        tempo_utils::Result<InstanceSymbol *> getOrImportInstance(const lyric_common::SymbolUrl &instanceUrl) const;
        tempo_utils::Result<NamespaceSymbol *> getOrImportNamespace(const lyric_common::SymbolUrl &namespaceUrl) const;
        tempo_utils::Result<StaticSymbol *> getOrImportStatic(const lyric_common::SymbolUrl &staticUrl) const;
        tempo_utils::Result<StructSymbol *> getOrImportStruct(const lyric_common::SymbolUrl &structUrl) const;

        tempo_utils::Result<TypenameSymbol *> putTypename(const lyric_common::SymbolUrl &typenameUrl);

    private:
        ObjectState *m_state;
        absl::flat_hash_map<lyric_common::SymbolUrl, AbstractSymbol *> m_symcache;
        std::queue<TypenameSymbol *> m_typenames;
    };
}

#endif // LYRIC_ASSEMBLER_SYMBOL_CACHE_H
