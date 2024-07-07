#ifndef LYRIC_PARSER_INTERNAL_MODULE_MACRO_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_MACRO_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleMacroOps {

    public:
        explicit ModuleMacroOps(ArchetypeState *state);
        virtual ~ModuleMacroOps() = default;

        void enterMacro(ModuleParser::MacroContext *ctx);
        void enterAnnotationList(ModuleParser::AnnotationListContext *ctx);
        void exitMacroCall(ModuleParser::MacroCallContext *ctx);
        void exitAnnotationList(ModuleParser::AnnotationListContext *ctx);
        void exitMacro(ModuleParser::MacroContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_MACRO_OPS_H
