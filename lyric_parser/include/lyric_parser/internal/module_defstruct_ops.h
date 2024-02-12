#ifndef LYRIC_PARSER_INTERNAL_MODULE_DEFSTRUCT_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_DEFSTRUCT_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleDefstructOps {

    public:
        explicit ModuleDefstructOps(ArchetypeState *state);
        virtual ~ModuleDefstructOps() = default;

        void enterDefstructStatement(ModuleParser::DefstructStatementContext *ctx);
        void exitStructSuper(ModuleParser::StructSuperContext *ctx);
        void enterStructInit(ModuleParser::StructInitContext *ctx);
        void exitStructInit(ModuleParser::StructInitContext *ctx);
        void enterStructVal(ModuleParser::StructValContext *ctx);
        void exitStructVal(ModuleParser::StructValContext *ctx);
        void enterStructDef(ModuleParser::StructDefContext *ctx);
        void exitStructDef(ModuleParser::StructDefContext *ctx);
        void exitDefstructStatement(ModuleParser::DefstructStatementContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEFSTRUCT_OPS_H