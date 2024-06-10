#ifndef LYRIC_ASSEMBLER_CTOR_CONSTRUCTABLE_H
#define LYRIC_ASSEMBLER_CTOR_CONSTRUCTABLE_H

#include <lyric_common/symbol_url.h>

#include "abstract_callsite_reifier.h"
#include "abstract_callable.h"
#include "assembler_result.h"
#include "assembler_types.h"
#include "block_handle.h"
#include "call_symbol.h"
#include "class_symbol.h"
#include "code_builder.h"
#include "enum_symbol.h"
#include "instance_symbol.h"
#include "struct_symbol.h"

namespace lyric_assembler {

    class CtorConstructable : public AbstractConstructable {

    public:
        CtorConstructable();
        CtorConstructable(CallSymbol *ctor, ClassSymbol *symbol);
        CtorConstructable(CallSymbol *ctor, EnumSymbol *symbol);
        CtorConstructable(CallSymbol *ctor, InstanceSymbol *symbol);
        CtorConstructable(CallSymbol *ctor, StructSymbol *symbol);

        bool isValid() const;

        lyric_assembler::TemplateHandle *getTemplate() const override;
        std::vector<Parameter>::const_iterator listPlacementBegin() const override;
        std::vector<Parameter>::const_iterator listPlacementEnd() const override;
        std::vector<Parameter>::const_iterator namedPlacementBegin() const override;
        std::vector<Parameter>::const_iterator namedPlacementEnd() const override;
        const Parameter *restPlacement() const override;
        bool hasInitializer(const std::string &name) const override;
        lyric_common::SymbolUrl getInitializer(const std::string &name) const override;

        tempo_utils::Result<lyric_common::TypeDef> invoke(
            BlockHandle *block,
            const AbstractCallsiteReifier &reifier,
            tu_uint8 flags) override;

        tempo_utils::Result<lyric_common::TypeDef> invokeNew(
            BlockHandle *block,
            const AbstractCallsiteReifier &reifier,
            tu_uint8 flags) override;

    private:
        CallSymbol *m_ctor;
        uint8_t m_newType;
        uint32_t m_newAddress;
        CallAddress m_ctorAddress;
        lyric_common::TypeDef m_ctorType;

        CtorConstructable(CallSymbol *ctor);

        void checkValid() const;
    };
}

#endif // LYRIC_ASSEMBLER_CTOR_CONSTRUCTABLE_H