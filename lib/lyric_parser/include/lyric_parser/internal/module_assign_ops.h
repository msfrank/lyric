#ifndef LYRIC_PARSER_INTERNAL_MODULE_ASSIGN_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_ASSIGN_OPS_H

#include <ModuleParserBaseListener.h>

#include "base_ops.h"

namespace lyric_parser::internal {

    class ModuleAssignOps : public BaseOps {

    public:
        explicit ModuleAssignOps(ModuleArchetype *listener);

        void enterGlobalStatement(ModuleParser::GlobalStatementContext *ctx);
        void exitGlobalStatement(ModuleParser::GlobalStatementContext *ctx);

        void enterDefaultNewVal(ModuleParser::DefaultNewValContext *ctx);
        void exitDefaultNewVal(ModuleParser::DefaultNewValContext *ctx);
        void enterTypedVal(ModuleParser::TypedValContext *ctx);
        void exitTypedVal(ModuleParser::TypedValContext *ctx);
        void enterUntypedVal(ModuleParser::UntypedValContext *ctx);
        void exitUntypedVal(ModuleParser::UntypedValContext *ctx);

        void enterDefaultNewVar(ModuleParser::DefaultNewVarContext *ctx);
        void exitDefaultNewVar(ModuleParser::DefaultNewVarContext *ctx);
        void enterTypedVar(ModuleParser::TypedVarContext *ctx);
        void exitTypedVar(ModuleParser::TypedVarContext *ctx);
        void enterUntypedVar(ModuleParser::UntypedVarContext *ctx);
        void exitUntypedVar(ModuleParser::UntypedVarContext *ctx);

        void parseVariableDefaultNew(ModuleParser::VariableDefaultNewContext *ctx);
        void parseNameAssignment(ModuleParser::NameAssignmentContext *ctx);
        void parseMemberAssignment(ModuleParser::MemberAssignmentContext *ctx);
        void parseAssignDefaultNew(ModuleParser::AssignDefaultNewContext *ctx);
        void parseDefaultNewSet(ModuleParser::DefaultNewSetContext *ctx);
        void parseExpressionSet(ModuleParser::ExpressionSetContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_ASSIGN_OPS_H
