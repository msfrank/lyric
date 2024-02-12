#ifndef LYRIC_PARSER_INTERNAL_MODULE_DEREF_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_DEREF_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleDerefOps {

    public:
        explicit ModuleDerefOps(ArchetypeState *state);
        virtual ~ModuleDerefOps() = default;

        void enterThisExpression(ModuleParser::ThisExpressionContext *ctx);
        void enterNameExpression(ModuleParser::NameExpressionContext *ctx);
        void enterCallExpression(ModuleParser::CallExpressionContext *ctx);
        void exitThisSpec(ModuleParser::ThisSpecContext *ctx);
        void exitNameSpec(ModuleParser::NameSpecContext *ctx);
        void exitCallSpec(ModuleParser::CallSpecContext *ctx);
        void exitDerefMember(ModuleParser::DerefMemberContext *ctx);
        void exitDerefMethod(ModuleParser::DerefMethodContext *ctx);
        void exitThisExpression(ModuleParser::ThisExpressionContext *ctx);
        void exitNameExpression(ModuleParser::NameExpressionContext *ctx);
        void exitCallExpression(ModuleParser::CallExpressionContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEREF_OPS_H
