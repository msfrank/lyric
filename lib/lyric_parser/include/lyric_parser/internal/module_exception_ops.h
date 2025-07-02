#ifndef LYRIC_PARSER_INTERNAL_MODULE_EXCEPTION_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_EXCEPTION_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleExceptionOps : public BaseOps {

    public:
        explicit ModuleExceptionOps(ModuleArchetype *listener);

        void enterTryStatement(ModuleParser::TryStatementContext *ctx);
        void exitTryTarget(ModuleParser::TryTargetContext *ctx);
        void exitCatchOnUnwrap(ModuleParser::CatchOnUnwrapContext *ctx);
        void exitCatchOnType(ModuleParser::CatchOnTypeContext *ctx);
        void exitCatchElse(ModuleParser::CatchElseContext *ctx);
        void exitCatchFinally(ModuleParser::CatchFinallyContext *ctx);
        void exitExpectExpression(ModuleParser::ExpectExpressionContext *ctx);
        void exitRaiseExpression(ModuleParser::RaiseExpressionContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_EXCEPTION_OPS_H