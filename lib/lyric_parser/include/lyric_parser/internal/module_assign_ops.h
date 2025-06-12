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
        void enterUntypedVal(ModuleParser::UntypedValContext *ctx);
        void exitUntypedVal(ModuleParser::UntypedValContext *ctx);
        void enterTypedVal(ModuleParser::TypedValContext *ctx);
        void exitTypedVal(ModuleParser::TypedValContext *ctx);
        void enterUntypedVar(ModuleParser::UntypedVarContext *ctx);
        void exitUntypedVar(ModuleParser::UntypedVarContext *ctx);
        void enterTypedVar(ModuleParser::TypedVarContext *ctx);
        void exitTypedVar(ModuleParser::TypedVarContext *ctx);
        void parseNameAssignment(ModuleParser::NameAssignmentContext *ctx);
        void parseMemberAssignment(ModuleParser::MemberAssignmentContext *ctx);
        void parseSetStatement(ModuleParser::SetStatementContext *ctx);
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_ASSIGN_OPS_H
