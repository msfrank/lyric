#ifndef LYRIC_PARSER_INTERNAL_MODULE_ARITHMETIC_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_ARITHMETIC_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleArithmeticOps : public BaseOps {

    public:
        explicit ModuleArithmeticOps(ModuleArchetype *listener);

        void parseAddExpression(ModuleParser::AddExpressionContext *ctx);
        void parseSubExpression(ModuleParser::SubExpressionContext *ctx);
        void parseMulExpression(ModuleParser::MulExpressionContext *ctx);
        void parseDivExpression(ModuleParser::DivExpressionContext *ctx);
        void parseNegExpression(ModuleParser::NegExpressionContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_ARITHMETIC_OPS_H
