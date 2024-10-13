#ifndef LYRIC_ASSEMBLER_ACTION_CALLABLE_H
#define LYRIC_ASSEMBLER_ACTION_CALLABLE_H

#include "abstract_callsite_reifier.h"
#include "abstract_callable.h"
#include "assembler_types.h"

namespace lyric_assembler {

    // forward declarations
    class ActionSymbol;
    class BlockHandle;
    class ConceptSymbol;

    class ActionCallable : public AbstractCallable {

    public:
        ActionCallable();
        ActionCallable(ActionSymbol *actionSymbol, ConceptSymbol *conceptSymbol);

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
        ActionSymbol *m_actionSymbol;
        ConceptSymbol *m_conceptSymbol;

        void checkValid() const;
    };
}

#endif // LYRIC_ASSEMBLER_ACTION_CALLABLE_H
