#ifndef LYRIC_ASSEMBLER_NAMESPACE_SYMBOL_H
#define LYRIC_ASSEMBLER_NAMESPACE_SYMBOL_H

#include "abstract_symbol.h"
#include "assembly_state.h"
#include "base_symbol.h"
#include "block_handle.h"
#include "proc_handle.h"

namespace lyric_assembler {

    struct NamespaceSymbolPriv {
        lyric_object::AccessType access;
        TypeHandle *namespaceType;
        NamespaceSymbol *superNamespace;
        std::unique_ptr<BlockHandle> namespaceBlock;
    };

    class NamespaceSymbol : public BaseSymbol<NamespaceAddress,NamespaceSymbolPriv> {

    public:
        NamespaceSymbol(
            const lyric_common::SymbolUrl &nsUrl,
            lyric_object::AccessType access,
            NamespaceAddress address,
            TypeHandle *nsType,
            NamespaceSymbol *superNs,
            BlockHandle *parentBlock,
            AssemblyState *state,
            bool isRoot);
        NamespaceSymbol(
            const lyric_common::SymbolUrl &nsUrl,
            NamespaceAddress address,
            TypeHandle *nsType,
            ProcHandle *entryProc,
            AssemblyState *state);
        NamespaceSymbol(
            const lyric_common::SymbolUrl &nsUrl,
            lyric_importer::NamespaceImport *namespaceImport,
            AssemblyState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getAssignableType() const override;
        TypeSignature getTypeSignature() const override;
        void touch() override;

        lyric_object::AccessType getAccessType() const;

        NamespaceSymbol *superNamespace() const;
        TypeHandle *namespaceType() const;
        BlockHandle *namespaceBlock() const;

    private:
        lyric_common::SymbolUrl m_namespaceUrl;
        lyric_importer::NamespaceImport *m_namespaceImport = nullptr;
        AssemblyState *m_state;

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