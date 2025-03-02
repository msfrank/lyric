#ifndef LYRIC_ASSEMBLER_STATIC_SYMBOL_H
#define LYRIC_ASSEMBLER_STATIC_SYMBOL_H

#include "abstract_symbol.h"
#include "object_state.h"
#include "base_symbol.h"
#include "callable_invoker.h"
#include "function_callable.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct StaticSymbolPriv {
        lyric_object::AccessType access;
        bool isVariable;
        TypeHandle *staticType;
        CallSymbol *initCall;
        bool isDeclOnly;
        std::unique_ptr<BlockHandle> staticBlock;
    };

    class StaticSymbol : public BaseSymbol<StaticSymbolPriv> {

    public:
        StaticSymbol(
            const lyric_common::SymbolUrl &staticUrl,
            lyric_object::AccessType access,
            bool isVariable,
            TypeHandle *staticType,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);
        StaticSymbol(
            const lyric_common::SymbolUrl &staticUrl,
            lyric_importer::StaticImport *staticImport,
            bool isCopied,
            ObjectState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        std::string getName() const;
        lyric_object::AccessType getAccessType() const;
        bool isVariable() const;
        bool isDeclOnly() const;

        lyric_common::SymbolUrl getInitializer() const;
        tempo_utils::Result<ProcHandle *> defineInitializer();
        tempo_utils::Status prepareInitializer(CallableInvoker &invoker);

        DataReference getReference() const;

    private:
        lyric_common::SymbolUrl m_staticUrl;
        lyric_importer::StaticImport *m_staticImport = nullptr;
        ObjectState *m_state;

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
