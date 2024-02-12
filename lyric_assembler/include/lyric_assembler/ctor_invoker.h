#ifndef LYRIC_ASSEMBLER_CTOR_INVOKER_H
#define LYRIC_ASSEMBLER_CTOR_INVOKER_H

#include <lyric_common/symbol_url.h>

#include "abstract_callsite_reifier.h"
#include "abstract_invoker.h"
#include "assembler_result.h"
#include "assembler_types.h"
#include "code_builder.h"

namespace lyric_assembler {

    // forward declarations
    class BlockHandle;
    class CallSymbol;
    class ClassSymbol;
    class EnumSymbol;
    class InstanceSymbol;
    class StructSymbol;

    class CtorInvoker : public AbstractInvoker {

    public:
        CtorInvoker();
        CtorInvoker(CallSymbol *ctor, ClassSymbol *symbol);
        CtorInvoker(CallSymbol *ctor, EnumSymbol *symbol);
        CtorInvoker(CallSymbol *ctor, InstanceSymbol *symbol);
        CtorInvoker(CallSymbol *ctor, StructSymbol *symbol);

        bool isValid() const;

        std::vector<lyric_object::Parameter> getParameters() const;
        Option<lyric_object::Parameter> getRest() const;
        lyric_common::SymbolUrl getTemplateUrl() const;
        std::vector<lyric_object::TemplateParameter> getTemplateParameters() const;
        std::vector<lyric_common::TypeDef> getTemplateArguments() const;

        std::vector<lyric_object::Parameter>::const_iterator placementBegin() const override;
        std::vector<lyric_object::Parameter>::const_iterator placementEnd() const override;
        bool hasInitializer(const std::string &name) const override;
        lyric_common::SymbolUrl getInitializer(const std::string &name) const override;

        tempo_utils::Result<lyric_common::TypeDef> invoke(
            BlockHandle *block,
            const AbstractCallsiteReifier &reifier,
            uint8_t flags = 0);

        tempo_utils::Result<lyric_common::TypeDef> invokeNew(
            BlockHandle *block,
            const AbstractCallsiteReifier &reifier,
            uint8_t flags = 0);

    private:
        CallSymbol *m_ctor;
        uint8_t m_newType;
        uint32_t m_newAddress;
        CallAddress m_ctorAddress;
        lyric_common::TypeDef m_ctorType;

        std::vector<lyric_object::Parameter> m_parameters;
        Option<lyric_object::Parameter> m_rest;
        std::vector<lyric_object::TemplateParameter> m_templateParameters;
        std::vector<lyric_common::TypeDef> m_templateArguments;
        lyric_common::SymbolUrl m_templateUrl;

        CtorInvoker(CallSymbol *ctor);
    };
}

#endif // LYRIC_ASSEMBLER_CTOR_INVOKER_H