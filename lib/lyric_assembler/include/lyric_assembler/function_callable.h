#ifndef LYRIC_ASSEMBLER_FUNCTION_CALLABLE_H
#define LYRIC_ASSEMBLER_FUNCTION_CALLABLE_H

#include <lyric_common/symbol_url.h>

#include "abstract_callsite_reifier.h"
#include "abstract_callable.h"
#include "assembler_types.h"

namespace lyric_assembler {

    // forward declarations
    class BlockHandle;
    class CallSymbol;
    class CodeFragment;
    class ProcHandle;

    class FunctionCallable : public AbstractCallable {

        enum class InvokeType {
            INVALID,
            INLINE,
            STATIC,
        };

    public:
        FunctionCallable();
        explicit FunctionCallable(CallSymbol *callSymbol, bool isInlined);

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
            CodeFragment *fragment) override;

    private:
        InvokeType m_type;
        CallSymbol *m_callSymbol;

        void checkValid() const;
    };
}

#endif // LYRIC_ASSEMBLER_FUNCTION_CALLABLE_H
