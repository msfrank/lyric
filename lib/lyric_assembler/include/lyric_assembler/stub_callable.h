#ifndef LYRIC_ASSEMBLER_STUB_CALLABLE_H
#define LYRIC_ASSEMBLER_STUB_CALLABLE_H

#include <lyric_common/symbol_url.h>

#include "abstract_callsite_reifier.h"
#include "abstract_callable.h"
#include "assembler_types.h"

namespace lyric_assembler {

    // forward declarations
    class BlockHandle;
    class ActionSymbol;

    class StubCallable : public AbstractCallable {
    public:
        StubCallable();
        explicit StubCallable(ActionSymbol *actionSymbol);

        bool isValid() const;

        TemplateHandle *getTemplate() const override;
        std::vector<Parameter>::const_iterator listPlacementBegin() const override;
        std::vector<Parameter>::const_iterator listPlacementEnd() const override;
        std::vector<Parameter>::const_iterator namedPlacementBegin() const override;
        std::vector<Parameter>::const_iterator namedPlacementEnd() const override;
        const Parameter *restPlacement() const override;
        bool hasInitializer(const std::string &name) const override;
        lyric_common::SymbolUrl getInitializer(const std::string &name) const override;
        bool hasReceiver() const override;
        lyric_common::SymbolUrl getReceiver() const override;

        tempo_utils::Result<lyric_common::TypeDef> invoke(
            BlockHandle *block,
            const AbstractCallsiteReifier &reifier,
            CodeFragment *fragment) override;

        tempo_utils::Result<lyric_common::TypeDef> invokeCtor(
            BlockHandle *block,
            const AbstractCallsiteReifier &reifier,
            CodeFragment *fragment,
            tu_uint8 flags) override;

        tempo_utils::Result<lyric_common::TypeDef> invokeNew(
            BlockHandle *block,
            const AbstractCallsiteReifier &reifier,
            CodeFragment *fragment,
            tu_uint8 flags) override;

    private:
        ActionSymbol *m_actionSymbol;

        std::vector<lyric_object::Parameter> m_parameters;
        Option<lyric_object::Parameter> m_rest;
        std::vector<lyric_object::TemplateParameter> m_templateParameters;
        lyric_common::SymbolUrl m_templateUrl;

        void checkValid() const;
    };
}

#endif // LYRIC_ASSEMBLER_STUB_CALLABLE_H
