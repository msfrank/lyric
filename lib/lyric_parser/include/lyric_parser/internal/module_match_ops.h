#ifndef LYRIC_PARSER_INTERNAL_MODULE_MATCH_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_MATCH_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleMatchOps : public BaseOps {

    public:
        explicit ModuleMatchOps(ModuleArchetype *listener);

        void enterMatchExpression(ModuleParser::MatchExpressionContext *ctx);
        void exitMatchTarget(ModuleParser::MatchTargetContext *ctx);
        void exitMatchOnEnum(ModuleParser::MatchOnEnumContext *ctx);
        void exitMatchOnType(ModuleParser::MatchOnTypeContext *ctx);
        void exitMatchOnUnwrap(ModuleParser::MatchOnUnwrapContext *ctx);
        void exitMatchElse(ModuleParser::MatchElseContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_MATCH_OPS_H
