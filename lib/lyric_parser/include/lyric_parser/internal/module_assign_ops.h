#ifndef LYRIC_PARSER_INTERNAL_MODULE_ASSIGN_OPS_H
#define LYRIC_PARSER_INTERNAL_MODULE_ASSIGN_OPS_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"

namespace lyric_parser::internal {

    class ModuleAssignOps {

    public:
        explicit ModuleAssignOps(ArchetypeState *state);
        virtual ~ModuleAssignOps() = default;

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
        void exitNameAssignment(ModuleParser::NameAssignmentContext *ctx);
        void exitMemberAssignment(ModuleParser::MemberAssignmentContext *ctx);
        void exitSetStatement(ModuleParser::SetStatementContext *ctx);

    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_ASSIGN_OPS_H
