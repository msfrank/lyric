#ifndef LYRIC_PARSER_INTERNAL_MODULE_DEFENUM_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_DEFENUM_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleDefenumOps : public BaseOps {

    public:
        explicit ModuleDefenumOps(ModuleArchetype *listener);

        void enterDefenumStatement(ModuleParser::DefenumStatementContext *ctx);
        void enterEnumInit(ModuleParser::EnumInitContext *ctx);
        void exitEnumInit(ModuleParser::EnumInitContext *ctx);
        void enterEnumVal(ModuleParser::EnumValContext *ctx);
        void exitEnumVal(ModuleParser::EnumValContext *ctx);
        void enterEnumDef(ModuleParser::EnumDefContext *ctx);
        void exitEnumDef(ModuleParser::EnumDefContext *ctx);
        void enterEnumCase(ModuleParser::EnumCaseContext *ctx);
        void exitEnumCase(ModuleParser::EnumCaseContext *ctx);
        void enterEnumImpl(ModuleParser::EnumImplContext *ctx);
        void exitEnumImpl(ModuleParser::EnumImplContext *ctx);
        void exitDefenumStatement(ModuleParser::DefenumStatementContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEFENUM_OPS_H