#ifndef LYRIC_PARSER_INTERNAL_MODULE_DEFINE_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_DEFINE_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleDefineOps {

    public:
        explicit ModuleDefineOps(ArchetypeState *state);
        virtual ~ModuleDefineOps() = default;

        void enterDefStatement(ModuleParser::DefStatementContext *ctx);
        void exitDefStatement(ModuleParser::DefStatementContext *ctx);

        void exitDefaliasStatement(ModuleParser::DefaliasStatementContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEFINE_OPS_H
