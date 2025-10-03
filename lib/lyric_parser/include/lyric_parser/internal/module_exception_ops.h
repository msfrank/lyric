#ifndef LYRIC_PARSER_INTERNAL_MODULE_EXCEPTION_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_EXCEPTION_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleExceptionOps : public BaseOps {

    public:
        explicit ModuleExceptionOps(ModuleArchetype *listener);

        void enterTryStatement(ModuleParser::TryStatementContext *ctx);
        void exitTryBlock(ModuleParser::TryBlockContext *ctx);

        void enterTryCatch(ModuleParser::TryCatchContext *ctx);
        void exitCatchOnUnwrap(ModuleParser::CatchOnUnwrapContext *ctx);
        void exitCatchOnType(ModuleParser::CatchOnTypeContext *ctx);
        void exitCatchElse(ModuleParser::CatchElseContext *ctx);
        void exitTryCatch(ModuleParser::TryCatchContext *ctx);

        void enterTryFinally(ModuleParser::TryFinallyContext *ctx);
        void exitTryFinally(ModuleParser::TryFinallyContext *ctx);

        void exitExpectExpression(ModuleParser::ExpectExpressionContext *ctx);
        void exitRaiseExpression(ModuleParser::RaiseExpressionContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_EXCEPTION_OPS_H