#ifndef LYRIC_PARSER_INTERNAL_MODULE_CONSTRUCT_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_CONSTRUCT_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleConstructOps {

    public:
        explicit ModuleConstructOps(ArchetypeState *state);
        virtual ~ModuleConstructOps() = default;

        void exitDerefNew(ModuleParser::DerefNewContext *ctx);

        void exitPairExpression(ModuleParser::PairExpressionContext *ctx);
        void exitLambdaExpression(ModuleParser::LambdaExpressionContext *ctx);
        void exitLambdaFromExpression(ModuleParser::LambdaFromExpressionContext *ctx);

        void exitDefaultInitializerTypedNew(ModuleParser::DefaultInitializerTypedNewContext *ctx);
        void exitDefaultInitializerNew(ModuleParser::DefaultInitializerNewContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_CONSTRUCT_OPS_H
