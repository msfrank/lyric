#ifndef LYRIC_PARSER_INTERNAL_MODULE_DEFSTRUCT_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_DEFSTRUCT_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleDefstructOps : public BaseOps {

    public:
        explicit ModuleDefstructOps(ModuleArchetype *listener);

        void enterDefstructStatement(ModuleParser::DefstructStatementContext *ctx);
        void enterStructInit(ModuleParser::StructInitContext *ctx);
        void exitStructInit(ModuleParser::StructInitContext *ctx);
        void enterStructVal(ModuleParser::StructValContext *ctx);
        void exitStructVal(ModuleParser::StructValContext *ctx);
        void enterStructDef(ModuleParser::StructDefContext *ctx);
        void exitStructDef(ModuleParser::StructDefContext *ctx);
        void enterStructImpl(ModuleParser::StructImplContext *ctx);
        void exitStructImpl(ModuleParser::StructImplContext *ctx);
        void exitDefstructStatement(ModuleParser::DefstructStatementContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEFSTRUCT_OPS_H