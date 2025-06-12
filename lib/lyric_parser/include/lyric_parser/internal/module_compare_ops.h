#ifndef LYRIC_PARSER_INTERNAL_MODULE_COMPARE_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_COMPARE_OPS_H

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleCompareOps : public BaseOps {

    public:
        explicit ModuleCompareOps(ModuleArchetype *listener);

        void parseIsEqualExpression(ModuleParser::IsEqualExpressionContext *ctx);
        void parseIsLessThanExpression(ModuleParser::IsLessThanExpressionContext *ctx);
        void parseIsLessOrEqualExpression(ModuleParser::IsLessOrEqualExpressionContext *ctx);
        void parseIsGreaterThanExpression(ModuleParser::IsGreaterThanExpressionContext *ctx);
        void parseIsGreaterOrEqualExpression(ModuleParser::IsGreaterOrEqualExpressionContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_COMPARE_OPS_H
