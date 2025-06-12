#ifndef LYRIC_PARSER_INTERNAL_MODULE_DEFINE_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_DEFINE_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleDefineOps : public BaseOps {

    public:
        explicit ModuleDefineOps(ModuleArchetype *listener);

        void exitTypenameStatement(ModuleParser::TypenameStatementContext *ctx);

        void enterDefStatement(ModuleParser::DefStatementContext *ctx);
        void exitDefStatement(ModuleParser::DefStatementContext *ctx);

        void enterImplDef(ModuleParser::ImplDefContext *ctx);
        void exitImplDef(ModuleParser::ImplDefContext *ctx);

        void exitDefaliasStatement(ModuleParser::DefaliasStatementContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEFINE_OPS_H
