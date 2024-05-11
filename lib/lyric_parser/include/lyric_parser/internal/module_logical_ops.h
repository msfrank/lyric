#ifndef LYRIC_PARSER_INTERNAL_MODULE_LOGICAL_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_LOGICAL_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleLogicalOps {

    public:
        explicit ModuleLogicalOps(ArchetypeState *state);
        virtual ~ModuleLogicalOps() = default;

        void exitBooleanAndExpression(ModuleParser::BooleanAndExpressionContext *ctx);
        void exitBooleanOrExpression(ModuleParser::BooleanOrExpressionContext *ctx);
        void exitBooleanNotExpression(ModuleParser::BooleanNotExpressionContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_LOGICAL_OPS_H
