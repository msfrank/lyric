#ifndef LYRIC_ASSEMBLER_STATIC_SYMBOL_H
#define LYRIC_ASSEMBLER_STATIC_SYMBOL_H

#include "abstract_symbol.h"
#include "assembly_state.h"
#include "base_symbol.h"
#include "call_invoker.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct StaticSymbolPriv {
        bool isVariable;
        TypeHandle *staticType;
        CallSymbol *initCall;
        std::unique_ptr<BlockHandle> staticBlock;
    };

    class StaticSymbol : public BaseSymbol<StaticAddress,StaticSymbolPriv> {

    public:
        StaticSymbol(
            const lyric_common::SymbolUrl &staticUrl,
            bool isVariable,
            StaticAddress address,
            TypeHandle *staticType,
            BlockHandle *parentBlock,
            AssemblyState *state);
        StaticSymbol(
            const lyric_common::SymbolUrl &staticUrl,
            lyric_importer::StaticImport *staticImport,
            AssemblyState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getAssignableType() const override;
        TypeSignature getTypeSignature() const override;
        void touch() override;

        std::string getName() const;
        bool isVariable() const;

        lyric_common::SymbolUrl getInitializer() const;
        tempo_utils::Result<lyric_common::SymbolUrl> declareInitializer();
        tempo_utils::Result<CallInvoker> resolveInitializer();

    private:
        lyric_common::SymbolUrl m_staticUrl;
        lyric_importer::StaticImport *m_staticImport = nullptr;
        AssemblyState *m_state;

        StaticSymbolPriv *load() override;
    };

    static inline const StaticSymbol *cast_symbol_to_static(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::STATIC);
        return static_cast<const StaticSymbol *>(sym);      // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline StaticSymbol *cast_symbol_to_static(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::STATIC);
        return static_cast<StaticSymbol *>(sym);            // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_STATIC_SYMBOL_H
