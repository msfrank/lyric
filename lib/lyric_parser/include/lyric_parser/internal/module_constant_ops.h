#ifndef LYRIC_PARSER_INTERNAL_MODULE_CONSTANT_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_CONSTANT_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleConstantOps {

    public:
        explicit ModuleConstantOps(ArchetypeState *state);
        virtual ~ModuleConstantOps() = default;

        void exitTrueLiteral(ModuleParser::TrueLiteralContext *ctx);
        void exitFalseLiteral(ModuleParser::FalseLiteralContext *ctx);
        void exitUndefLiteral(ModuleParser::UndefLiteralContext *ctx);
        void exitNilLiteral(ModuleParser::NilLiteralContext *ctx);

        void exitSymbolLiteral(ModuleParser::SymbolLiteralContext *ctx);

        void exitDecimalInteger(ModuleParser::DecimalIntegerContext *ctx);
        void exitHexInteger(ModuleParser::HexIntegerContext *ctx);
        void exitOctalInteger(ModuleParser::OctalIntegerContext *ctx);
        void exitDecimalFixedFloat(ModuleParser::DecimalFixedFloatContext *ctx);
        void exitDecimalScientificFloat(ModuleParser::DecimalScientificFloatContext *ctx);
        void exitHexFloat(ModuleParser::HexFloatContext *ctx);

        void exitCharLiteral(ModuleParser::CharLiteralContext *ctx);
        void exitStringLiteral(ModuleParser::StringLiteralContext *ctx);
        void exitUrlLiteral(ModuleParser::UrlLiteralContext *ctx);

//        void enterSeqLiteral(ModuleParser::SeqLiteralContext *ctx);
//        void exitSeqSpec(ModuleParser::SeqSpecContext *ctx);
//        void exitSeqLiteral(ModuleParser::SeqLiteralContext *ctx);
//
//        void enterMapLiteral(ModuleParser::MapLiteralContext *ctx);
//        void exitMapSpec(ModuleParser::MapSpecContext *ctx);
//        void exitMapLiteral(ModuleParser::MapLiteralContext *ctx);
//
//        void enterRowLiteral(ModuleParser::RowLiteralContext *ctx);
//        void exitRowSpec(ModuleParser::RowSpecContext *ctx);
//        void exitRowLiteral(ModuleParser::RowLiteralContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_CONSTANT_OPS_H
