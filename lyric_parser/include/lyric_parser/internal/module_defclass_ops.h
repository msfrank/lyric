#ifndef LYRIC_PARSER_INTERNAL_MODULE_DEFCLASS_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_DEFCLASS_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleDefclassOps {

    public:
        explicit ModuleDefclassOps(ArchetypeState *state);
        virtual ~ModuleDefclassOps() = default;

        void enterDefclassStatement(ModuleParser::DefclassStatementContext *ctx);
        void exitClassSuper(ModuleParser::ClassSuperContext *ctx);
        void enterClassInit(ModuleParser::ClassInitContext *ctx);
        void exitClassInit(ModuleParser::ClassInitContext *ctx);
        void enterClassVal(ModuleParser::ClassValContext *ctx);
        void exitClassVal(ModuleParser::ClassValContext *ctx);
        void enterClassVar(ModuleParser::ClassVarContext *ctx);
        void exitClassVar(ModuleParser::ClassVarContext *ctx);
        void enterClassDef(ModuleParser::ClassDefContext *ctx);
        void exitClassDef(ModuleParser::ClassDefContext *ctx);
        void exitDefclassStatement(ModuleParser::DefclassStatementContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEFCLASS_OPS_H
