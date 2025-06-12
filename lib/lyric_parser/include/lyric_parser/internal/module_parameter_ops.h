#ifndef LYRIC_PARSER_INTERNAL_MODULE_PARAMETER_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_PARAMETER_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleParameterOps : public BaseOps {

    public:
        explicit ModuleParameterOps(ModuleArchetype *listener);

        void enterParamSpec(ModuleParser::ParamSpecContext *ctx);
        void exitPositionalParam(ModuleParser::PositionalParamContext *ctx);
        void exitNamedParam(ModuleParser::NamedParamContext *ctx);
        void exitRenamedParam(ModuleParser::RenamedParamContext *ctx);
        void exitNamedCtx(ModuleParser::NamedCtxContext *ctx);
        void exitRenamedCtx(ModuleParser::RenamedCtxContext *ctx);
        void exitRest(ModuleParser::RestContext *ctx);
        void exitParamSpec(ModuleParser::ParamSpecContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_PARAMETER_OPS_H
