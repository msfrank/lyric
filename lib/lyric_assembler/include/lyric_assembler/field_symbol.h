#ifndef LYRIC_ASSEMBLER_FIELD_SYMBOL_H
#define LYRIC_ASSEMBLER_FIELD_SYMBOL_H

#include "abstract_symbol.h"
#include "object_state.h"
#include "base_symbol.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct FieldSymbolPriv {
        lyric_object::AccessType access;
        bool isVariable;
        bool isDeclOnly;
        lyric_common::SymbolUrl init;
        BlockHandle *parentBlock;
        TypeHandle *fieldType;
    };

    class FieldSymbol : public BaseSymbol<FieldSymbolPriv> {
    public:
        FieldSymbol(
            const lyric_common::SymbolUrl &fieldUrl,
            lyric_object::AccessType access,
            bool isVariable,
            TypeHandle *fieldType,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);
        FieldSymbol(
            const lyric_common::SymbolUrl &fieldUrl,
            lyric_importer::FieldImport *fieldImport,
            ObjectState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        std::string getName() const;
        lyric_object::AccessType getAccessType() const;
        bool isVariable() const;
        bool isDeclOnly() const;

        bool hasInitializer() const;
        lyric_common::SymbolUrl getInitializer() const;
        tempo_utils::Result<ProcHandle *> defineInitializer();

        DataReference getReference() const;

    private:
        lyric_common::SymbolUrl m_fieldUrl;
        lyric_importer::FieldImport * m_fieldImport = nullptr;
        ObjectState *m_state;

        FieldSymbolPriv *load() override;
    };

    static inline const FieldSymbol *cast_symbol_to_field(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::FIELD);
        return static_cast<const FieldSymbol *>(sym);       // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline FieldSymbol *cast_symbol_to_field(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::FIELD);
        return static_cast<FieldSymbol *>(sym);             // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_FIELD_SYMBOL_H
