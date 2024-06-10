#ifndef LYRIC_ASSEMBLER_METHOD_CALLABLE_H
#define LYRIC_ASSEMBLER_METHOD_CALLABLE_H

#include <lyric_common/symbol_url.h>

#include "abstract_callsite_reifier.h"
#include "abstract_callable.h"
#include "assembler_result.h"
#include "assembler_types.h"
#include "code_builder.h"

namespace lyric_assembler {

    // forward declarations
    class BlockHandle;
    class CallSymbol;

    class MethodCallable : public AbstractCallable {

        enum class InvokeType {
            INVALID,
            INLINE,
            VIRTUAL,
        };

    public:
        MethodCallable();
        MethodCallable(CallSymbol *call);
        MethodCallable(CallSymbol *call, ProcHandle *proc);

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
            const AbstractCallsiteReifier &reifier) override;

    private:
        InvokeType m_type;
        CallSymbol *m_call;
        ProcHandle *m_proc;
        SymbolBinding m_var;

        std::vector<lyric_object::Parameter> m_parameters;
        Option<lyric_object::Parameter> m_rest;
        std::vector<lyric_object::TemplateParameter> m_templateParameters;
        lyric_common::SymbolUrl m_templateUrl;

        void checkValid() const;
    };
}

#endif // LYRIC_ASSEMBLER_METHOD_CALLABLE_H
