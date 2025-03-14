#ifndef LYRIC_ASSEMBLER_NAMESPACE_SYMBOL_H
#define LYRIC_ASSEMBLER_NAMESPACE_SYMBOL_H

#include "abstract_symbol.h"
#include "object_state.h"
#include "base_symbol.h"
#include "block_handle.h"
#include "proc_handle.h"

namespace lyric_assembler {

    struct NamespaceSymbolPriv {
        lyric_object::AccessType access;
        bool isDeclOnly;
        TypeHandle *namespaceType;
        NamespaceSymbol *superNamespace;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl> symbols;
        std::unique_ptr<BlockHandle> namespaceBlock;
    };

    class NamespaceSymbol : public BaseSymbol<NamespaceSymbolPriv> {

    public:
        NamespaceSymbol(
            const lyric_common::SymbolUrl &nsUrl,
            TypeHandle *nsType,
            BlockHandle *rootBlock,
            ObjectState *state);
        NamespaceSymbol(
            const lyric_common::SymbolUrl &nsUrl,
            lyric_object::AccessType access,
            TypeHandle *nsType,
            NamespaceSymbol *superNs,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);
        NamespaceSymbol(
            const lyric_common::SymbolUrl &nsUrl,
            lyric_importer::NamespaceImport *namespaceImport,
            bool isCopied,
            ObjectState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        lyric_object::AccessType getAccessType() const;
        bool isDeclOnly() const;

        NamespaceSymbol *superNamespace() const;
        TypeHandle *namespaceType() const;
        BlockHandle *namespaceBlock() const;

        bool hasSymbol(const std::string &name) const;
        lyric_common::SymbolUrl getSymbol(const std::string &name) const;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator symbolsBegin() const;
        absl::flat_hash_map<std::string,lyric_common::SymbolUrl>::const_iterator symbolsEnd() const;
        tu_uint32 numSymbols() const;

        tempo_utils::Status putTarget(const lyric_common::SymbolUrl &symbolUrl);

        tempo_utils::Result<BindingSymbol *> declareBinding(
            const std::string &name,
            lyric_object::AccessType access,
            const std::vector<lyric_object::TemplateParameter> &templateParameters = {});

        tempo_utils::Result<NamespaceSymbol *> declareSubspace(
            const std::string &name,
            lyric_object::AccessType access);

    private:
        lyric_common::SymbolUrl m_namespaceUrl;
        lyric_importer::NamespaceImport *m_namespaceImport = nullptr;
        ObjectState *m_state;

        NamespaceSymbolPriv *load() override;
    };

    static inline const NamespaceSymbol *cast_symbol_to_namespace(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::NAMESPACE);
        return static_cast<const NamespaceSymbol *>(sym);    // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline NamespaceSymbol *cast_symbol_to_namespace(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::NAMESPACE);
        return static_cast<NamespaceSymbol *>(sym);          // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_NAMESPACE_SYMBOL_H