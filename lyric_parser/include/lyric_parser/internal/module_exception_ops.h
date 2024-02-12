#ifndef LYRIC_PARSER_INTERNAL_MODULE_EXCEPTION_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_EXCEPTION_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleExceptionOps {

    public:
        explicit ModuleExceptionOps(ArchetypeState *state);
        virtual ~ModuleExceptionOps() = default;

        void enterTryStatement(ModuleParser::TryStatementContext *ctx);
        void exitTryTarget(ModuleParser::TryTargetContext *ctx);
        void exitCatchOnUnwrap(ModuleParser::CatchOnUnwrapContext *ctx);
        void exitCatchOnType(ModuleParser::CatchOnTypeContext *ctx);
        void exitCatchElse(ModuleParser::CatchElseContext *ctx);
        void exitCatchFinally(ModuleParser::CatchFinallyContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_EXCEPTION_OPS_H