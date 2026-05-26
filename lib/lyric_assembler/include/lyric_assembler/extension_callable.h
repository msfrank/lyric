#ifndef LYRIC_ASSEMBLER_EXTENSION_CALLABLE_H
#define LYRIC_ASSEMBLER_EXTENSION_CALLABLE_H

#include <lyric_common/symbol_url.h>

#include "abstract_callsite_reifier.h"
#include "abstract_callable.h"
#include "assembler_types.h"

namespace lyric_assembler {

    // forward declarations
    class CallSymbol;
    class CodeFragment;
    class ProcHandle;
    class TemplateHandle;

    class ExtensionCallable : public AbstractCallable {

        enum class InvokeType {
            INVALID,
            INLINE,
            REF,
            OFFSET,
        };

    public:
        ExtensionCallable();
        explicit ExtensionCallable(CallSymbol *callSymbol);
        ExtensionCallable(CallSymbol *callSymbol, const DataReference &implRef);
        ExtensionCallable(CallSymbol *callSymbol, tu_uint16 offset);

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
        InvokeType m_type;
        CallSymbol *m_callSymbol;
        DataReference m_implRef;
        tu_uint16 m_offset;

        std::vector<lyric_object::Parameter> m_parameters;
        Option<lyric_object::Parameter> m_rest;
        std::vector<lyric_object::TemplateParameter> m_templateParameters;
        lyric_common::SymbolUrl m_templateUrl;

        void checkValid() const;
    };
}

#endif // LYRIC_ASSEMBLER_EXTENSION_CALLABLE_H
