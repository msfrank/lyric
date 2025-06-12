#ifndef LYRIC_PARSER_INTERNAL_MODULE_CONSTANT_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_CONSTANT_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"
#include "module_archetype.h"

namespace lyric_parser::internal {

    class ModuleConstantOps : public BaseOps {

    public:
        explicit ModuleConstantOps(ModuleArchetype *moduleArchetype);

        void parseTrueLiteral(ModuleParser::TrueLiteralContext *ctx);
        void parseFalseLiteral(ModuleParser::FalseLiteralContext *ctx);
        void parseUndefLiteral(ModuleParser::UndefLiteralContext *ctx);
        void parseNilLiteral(ModuleParser::NilLiteralContext *ctx);

        void parseDecimalInteger(ModuleParser::DecimalIntegerContext *ctx);
        void parseHexInteger(ModuleParser::HexIntegerContext *ctx);
        void parseOctalInteger(ModuleParser::OctalIntegerContext *ctx);
        void parseDecimalFixedFloat(ModuleParser::DecimalFixedFloatContext *ctx);
        void parseDecimalScientificFloat(ModuleParser::DecimalScientificFloatContext *ctx);
        void parseHexFloat(ModuleParser::HexFloatContext *ctx);
        void parseInvalidNumber(ModuleParser::InvalidNumberContext *ctx);

        void parseCharLiteral(ModuleParser::CharLiteralContext *ctx);
        void parseStringLiteral(ModuleParser::StringLiteralContext *ctx);
        void parseUrlLiteral(ModuleParser::UrlLiteralContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_CONSTANT_OPS_H
