#ifndef LYRIC_PARSER_INTERNAL_MODULE_MATCH_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_MATCH_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleMatchOps {

    public:
        explicit ModuleMatchOps(ArchetypeState *state);
        virtual ~ModuleMatchOps() = default;

        void enterMatchExpression(ModuleParser::MatchExpressionContext *ctx);
        void exitMatchTarget(ModuleParser::MatchTargetContext *ctx);
        void exitMatchOnEnum(ModuleParser::MatchOnEnumContext *ctx);
        void exitMatchOnType(ModuleParser::MatchOnTypeContext *ctx);
        void exitMatchOnUnwrap(ModuleParser::MatchOnUnwrapContext *ctx);
        void exitMatchElse(ModuleParser::MatchElseContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_MATCH_OPS_H
