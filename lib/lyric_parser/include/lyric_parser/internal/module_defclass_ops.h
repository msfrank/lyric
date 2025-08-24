#ifndef LYRIC_PARSER_INTERNAL_MODULE_DEFCLASS_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_DEFCLASS_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleDefclassOps : public BaseOps {

    public:
        explicit ModuleDefclassOps(ModuleArchetype *listener);

        void enterDefclassStatement(ModuleParser::DefclassStatementContext *ctx);
        void exitClassSuper(ModuleParser::ClassSuperContext *ctx);
        void enterClassInit(ModuleParser::ClassInitContext *ctx);
        void exitClassInit(ModuleParser::ClassInitContext *ctx);
        void enterClassVal(ModuleParser::ClassValContext *ctx);
        void exitClassVal(ModuleParser::ClassValContext *ctx);
        void enterClassVar(ModuleParser::ClassVarContext *ctx);
        void exitClassVar(ModuleParser::ClassVarContext *ctx);
        void enterClassDef(ModuleParser::ClassDefContext *ctx);
        void exitClassDef(ModuleParser::ClassDefContext *ctx);
        void enterClassDecl(ModuleParser::ClassDeclContext *ctx);
        void exitClassDecl(ModuleParser::ClassDeclContext *ctx);
        void enterClassImpl(ModuleParser::ClassImplContext *ctx);
        void exitClassImpl(ModuleParser::ClassImplContext *ctx);
        void exitDefclassStatement(ModuleParser::DefclassStatementContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_DEFCLASS_OPS_H
