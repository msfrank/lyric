#ifndef LYRIC_PARSER_INTERNAL_MODULE_COMPARE_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_COMPARE_OPS_H

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleCompareOps {

    public:
        explicit ModuleCompareOps(ArchetypeState *state);
        virtual ~ModuleCompareOps() = default;

        void exitIsEqualExpression(ModuleParser::IsEqualExpressionContext *ctx);
        void exitIsLessThanExpression(ModuleParser::IsLessThanExpressionContext *ctx);
        void exitIsLessOrEqualExpression(ModuleParser::IsLessOrEqualExpressionContext *ctx);
        void exitIsGreaterThanExpression(ModuleParser::IsGreaterThanExpressionContext *ctx);
        void exitIsGreaterOrEqualExpression(ModuleParser::IsGreaterOrEqualExpressionContext *ctx);
        void exitIsAExpression(ModuleParser::IsAExpressionContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_COMPARE_OPS_H
