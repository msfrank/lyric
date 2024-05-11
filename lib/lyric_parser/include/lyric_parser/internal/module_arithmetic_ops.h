#ifndef LYRIC_PARSER_INTERNAL_MODULE_ARITHMETIC_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_ARITHMETIC_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleArithmeticOps {

    public:
        explicit ModuleArithmeticOps(ArchetypeState *state);
        virtual ~ModuleArithmeticOps() = default;

        void exitAddExpression(ModuleParser::AddExpressionContext *ctx);
        void exitSubExpression(ModuleParser::SubExpressionContext *ctx);
        void exitMulExpression(ModuleParser::MulExpressionContext *ctx);
        void exitDivExpression(ModuleParser::DivExpressionContext *ctx);
        void exitNegExpression(ModuleParser::NegExpressionContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_ARITHMETIC_OPS_H
