#ifndef LYRIC_PARSER_INTERNAL_MODULE_LOGICAL_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_LOGICAL_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleLogicalOps : public BaseOps {

    public:
        explicit ModuleLogicalOps(ModuleArchetype *listener);

        void exitBooleanAndExpression(ModuleParser::BooleanAndExpressionContext *ctx);
        void exitBooleanOrExpression(ModuleParser::BooleanOrExpressionContext *ctx);
        void exitBooleanNotExpression(ModuleParser::BooleanNotExpressionContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_LOGICAL_OPS_H
