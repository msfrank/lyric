#ifndef LYRIC_PARSER_INTERNAL_MODULE_CONTROL_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_CONTROL_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"
#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleControlOps : public BaseOps {

    public:
        explicit ModuleControlOps(ModuleArchetype *listener);

        void exitIfStatement(ModuleParser::IfStatementContext *ctx);
        void exitIfThenElseExpression(ModuleParser::IfThenElseExpressionContext *ctx);
        void enterCondExpression(ModuleParser::CondExpressionContext *ctx);
        void exitCondWhen(ModuleParser::CondWhenContext *ctx);
        void exitCondElse(ModuleParser::CondElseContext *ctx);
        void enterCondIfStatement(ModuleParser::CondIfStatementContext *ctx);
        void exitCondIfWhen(ModuleParser::CondIfWhenContext *ctx);
        void exitCondIfElse(ModuleParser::CondIfElseContext *ctx);
        void enterWhileStatement(ModuleParser::WhileStatementContext *ctx);
        void exitWhileStatement(ModuleParser::WhileStatementContext *ctx);
        void enterForStatement(ModuleParser::ForStatementContext *ctx);
        void exitForStatement(ModuleParser::ForStatementContext * ctx);
        void exitReturnStatement(ModuleParser::ReturnStatementContext * ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_CONTROL_OPS_H
