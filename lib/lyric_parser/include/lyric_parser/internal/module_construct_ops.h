#ifndef LYRIC_PARSER_INTERNAL_MODULE_CONSTRUCT_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_CONSTRUCT_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleConstructOps : public BaseOps {

    public:
        explicit ModuleConstructOps(ModuleArchetype *listener);

        void parseDefaultThisBase(ModuleParser::DefaultThisBaseContext *ctx);
        void parseNamedThisBase(ModuleParser::NamedThisBaseContext *ctx);
        void parseDefaultSuperBase(ModuleParser::DefaultSuperBaseContext *ctx);
        void parseNamedSuperBase(ModuleParser::NamedSuperBaseContext *ctx);

        void parseDerefNew(ModuleParser::DerefNewContext *ctx);

        void parsePairExpression(ModuleParser::PairExpressionContext *ctx);
        void parseLambdaExpression(ModuleParser::LambdaExpressionContext *ctx);
        void parseLambdaFromExpression(ModuleParser::LambdaFromExpressionContext *ctx);

        void parseDefaultInitializerTypedNew(ModuleParser::DefaultInitializerTypedNewContext *ctx);
        void parseDefaultInitializerNew(ModuleParser::DefaultInitializerNewContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_CONSTRUCT_OPS_H
