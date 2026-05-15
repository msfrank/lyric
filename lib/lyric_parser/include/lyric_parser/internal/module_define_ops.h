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

        void enterImplExt(ModuleParser::ImplExtContext *ctx);
        void exitImplExt(ModuleParser::ImplExtContext *ctx);

        void parseModifierSpec(ModuleParser::ModifierSpecContext *ctx);

        void enterGlobalSpec(ModuleParser::GlobalSpecContext *ctx);
        void enterGlobalVal(ModuleParser::GlobalValContext *ctx);
        void exitGlobalVal(ModuleParser::GlobalValContext *ctx);
        void enterGlobalVar(ModuleParser::GlobalVarContext *ctx);
        void exitGlobalVar(ModuleParser::GlobalVarContext *ctx);
        void exitGlobalSpec(ModuleParser::GlobalSpecContext *ctx);

        void parseBindingAliasStatement(ModuleParser::BindingAliasStatementContext *ctx);
        void parseIndexAliasStatement(ModuleParser::IndexAliasStatementContext *ctx);
        void parseKeyAliasStatement(ModuleParser::KeyAliasStatementContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEFINE_OPS_H
