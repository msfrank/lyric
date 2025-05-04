#ifndef LYRIC_PARSER_INTERNAL_MODULE_MACRO_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_MACRO_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleMacroOps {

    public:
        explicit ModuleMacroOps(ArchetypeState *state);
        virtual ~ModuleMacroOps() = default;

        void exitRewriteArgs(ModuleParser::RewriteArgsContext *ctx);

        void enterPragma(ModuleParser::PragmaContext *ctx);
        void exitPragma(ModuleParser::PragmaContext *ctx);
        void enterAnnotation(ModuleParser::AnnotationContext *ctx);
        void exitAnnotation(ModuleParser::AnnotationContext *ctx);
        void enterMacro(ModuleParser::MacroContext *ctx);
        void exitMacro(ModuleParser::MacroContext *ctx);

        void enterAnnotationList(ModuleParser::AnnotationListContext *ctx);
        void exitAnnotationList(ModuleParser::AnnotationListContext *ctx);
        void enterMacroList(ModuleParser::MacroListContext *ctx);
        void exitMacroList(ModuleParser::MacroListContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_MACRO_OPS_H
