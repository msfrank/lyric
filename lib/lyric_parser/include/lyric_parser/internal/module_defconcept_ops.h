#ifndef LYRIC_PARSER_INTERNAL_MODULE_DEFCONCEPT_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_DEFCONCEPT_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleDefconceptOps {

    public:
        explicit ModuleDefconceptOps(ArchetypeState *state);
        virtual ~ModuleDefconceptOps() = default;

        void enterDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx);
        void enterConceptDef(ModuleParser::ConceptDefContext *ctx);
        void exitConceptDef(ModuleParser::ConceptDefContext *ctx);
        void enterConceptImpl(ModuleParser::ConceptImplContext *ctx);
        void exitConceptImpl(ModuleParser::ConceptImplContext *ctx);
        void exitDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEFCONCEPT_OPS_H
