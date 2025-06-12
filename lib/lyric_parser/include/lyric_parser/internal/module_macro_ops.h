#ifndef LYRIC_PARSER_INTERNAL_MODULE_MACRO_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_MACRO_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleMacroOps : public BaseOps {

    public:
        explicit ModuleMacroOps(ModuleArchetype *listener);

        void exitMacroArgs(ModuleParser::MacroArgsContext *ctx);

        void enterMacroCall(ModuleParser::MacroCallContext *ctx);
        void exitMacroCall(ModuleParser::MacroCallContext *ctx);
        void enterMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx);
        void exitMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx);

        void enterPragmaMacro(ModuleParser::PragmaMacroContext *ctx);
        void exitPragmaMacro(ModuleParser::PragmaMacroContext *ctx);
        void enterDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx);
        void exitDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx);
        void enterBlockMacro(ModuleParser::BlockMacroContext *ctx);
        void exitBlockMacro(ModuleParser::BlockMacroContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_MACRO_OPS_H
