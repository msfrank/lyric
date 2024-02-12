#ifndef LYRIC_PARSER_INTERNAL_MODULE_DEFENUM_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_DEFENUM_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleDefenumOps {

    public:
        explicit ModuleDefenumOps(ArchetypeState *state);
        virtual ~ModuleDefenumOps() = default;

        void enterDefenumStatement(ModuleParser::DefenumStatementContext *ctx);
        void enterEnumInit(ModuleParser::EnumInitContext *ctx);
        void exitEnumInit(ModuleParser::EnumInitContext *ctx);
        void enterEnumVal(ModuleParser::EnumValContext *ctx);
        void exitEnumVal(ModuleParser::EnumValContext *ctx);
        void enterEnumDef(ModuleParser::EnumDefContext *ctx);
        void exitEnumDef(ModuleParser::EnumDefContext *ctx);
        void enterEnumCase(ModuleParser::EnumCaseContext *ctx);
        void exitEnumCase(ModuleParser::EnumCaseContext *ctx);
        void exitDefenumStatement(ModuleParser::DefenumStatementContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEFENUM_OPS_H