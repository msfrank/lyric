#ifndef LYRIC_PARSER_INTERNAL_MODULE_DEFINSTANCE_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_DEFINSTANCE_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleDefinstanceOps {

    public:
        explicit ModuleDefinstanceOps(ArchetypeState *state);
        virtual ~ModuleDefinstanceOps() = default;

        void enterDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx);
        void enterInstanceVal(ModuleParser::InstanceValContext *ctx);
        void exitInstanceVal(ModuleParser::InstanceValContext *ctx);
        void enterInstanceVar(ModuleParser::InstanceVarContext *ctx);
        void exitInstanceVar(ModuleParser::InstanceVarContext *ctx);
        void enterInstanceDef(ModuleParser::InstanceDefContext *ctx);
        void exitInstanceDef(ModuleParser::InstanceDefContext *ctx);
        void enterInstanceImpl(ModuleParser::InstanceImplContext *ctx);
        void enterImplDef(ModuleParser::ImplDefContext *ctx);
        void exitImplDef(ModuleParser::ImplDefContext *ctx);
        void exitInstanceImpl(ModuleParser::InstanceImplContext *ctx);
        void exitDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEFINSTANCE_OPS_H
