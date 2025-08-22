#ifndef LYRIC_PARSER_INTERNAL_MODULE_DEFINSTANCE_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_DEFINSTANCE_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleDefinstanceOps : public BaseOps {

    public:
        explicit ModuleDefinstanceOps(ModuleArchetype *listener);

        void enterDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx);
        void enterInstanceInit(ModuleParser::InstanceInitContext *ctx);
        void exitInstanceInit(ModuleParser::InstanceInitContext *ctx);
        void enterInstanceVal(ModuleParser::InstanceValContext *ctx);
        void exitInstanceVal(ModuleParser::InstanceValContext *ctx);
        void enterInstanceVar(ModuleParser::InstanceVarContext *ctx);
        void exitInstanceVar(ModuleParser::InstanceVarContext *ctx);
        void enterInstanceDef(ModuleParser::InstanceDefContext *ctx);
        void exitInstanceDef(ModuleParser::InstanceDefContext *ctx);
        void enterInstanceImpl(ModuleParser::InstanceImplContext *ctx);
        void exitInstanceImpl(ModuleParser::InstanceImplContext *ctx);
        void exitDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEFINSTANCE_OPS_H
