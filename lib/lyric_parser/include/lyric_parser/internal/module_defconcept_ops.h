#ifndef LYRIC_PARSER_INTERNAL_MODULE_DEFCONCEPT_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_DEFCONCEPT_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleDefconceptOps : public BaseOps {

    public:
        explicit ModuleDefconceptOps(ModuleArchetype *listener);

        void enterDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx);
        void enterConceptDecl(ModuleParser::ConceptDeclContext *ctx);
        void exitConceptDecl(ModuleParser::ConceptDeclContext *ctx);
        void enterConceptImpl(ModuleParser::ConceptImplContext *ctx);
        void exitConceptImpl(ModuleParser::ConceptImplContext *ctx);
        void exitDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEFCONCEPT_OPS_H
