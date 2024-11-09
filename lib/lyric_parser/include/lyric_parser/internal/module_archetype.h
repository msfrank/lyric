#ifndef LYRIC_PARSER_INTERNAL_MODULE_ARCHETYPE_H
#define LYRIC_PARSER_INTERNAL_MODULE_ARCHETYPE_H

#include <ModuleParserBaseListener.h>

#include "../archetype_state.h"
#include "../parse_result.h"

#include "module_arithmetic_ops.h"
#include "module_assign_ops.h"
#include "module_compare_ops.h"
#include "module_constant_ops.h"
#include "module_construct_ops.h"
#include "module_control_ops.h"
#include "module_defclass_ops.h"
#include "module_defconcept_ops.h"
#include "module_defenum_ops.h"
#include "module_definstance_ops.h"
#include "module_defstruct_ops.h"
#include "module_define_ops.h"
#include "module_deref_ops.h"
#include "module_exception_ops.h"
#include "module_logical_ops.h"
#include "module_macro_ops.h"
#include "module_match_ops.h"
#include "module_parameter_ops.h"
#include "module_symbol_ops.h"

namespace lyric_parser::internal {

    class ModuleArchetype
        : private ModuleSymbolOps,
          private ModuleConstantOps,
          private ModuleLogicalOps,
          private ModuleArithmeticOps,
          private ModuleCompareOps,
          private ModuleAssignOps,
          private ModuleControlOps,
          private ModuleMatchOps,
          private ModuleDerefOps,
          private ModuleConstructOps,
          private ModuleDefclassOps,
          private ModuleDefconceptOps,
          private ModuleDefenumOps,
          private ModuleDefinstanceOps,
          private ModuleDefstructOps,
          private ModuleDefineOps,
          private ModuleParameterOps,
          private ModuleExceptionOps,
          private ModuleMacroOps,
          public ModuleParserBaseListener
    {

    public:
        explicit ModuleArchetype(ArchetypeState *state);
        virtual ~ModuleArchetype() = default;

        void enterRoot(ModuleParser::RootContext *ctx) override;
        void enterBlock(ModuleParser::BlockContext *ctx) override;
        void exitForm(ModuleParser::FormContext *ctx) override;
        void exitRoot(ModuleParser::RootContext *ctx) override;

        void exitSymbolIdentifier(ModuleParser::SymbolIdentifierContext *ctx) override;

        // implemented by ModuleSymbolOps
        void enterNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx) override;
        void exitNamespaceSpec(ModuleParser::NamespaceSpecContext *ctx) override;
        void exitNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx) override;

        void exitUsingPath(ModuleParser::UsingPathContext *ctx) override;
        void enterUsingFromStatement(ModuleParser::UsingFromStatementContext *ctx) override;
        void exitUsingFromStatement(ModuleParser::UsingFromStatementContext *ctx) override;
        void enterUsingLocalStatement(ModuleParser::UsingLocalStatementContext *ctx) override;
        void exitUsingLocalStatement(ModuleParser::UsingLocalStatementContext *ctx) override;

        void exitImportRef(ModuleParser::ImportRefContext *ctx) override;
        void exitImportModuleStatement(ModuleParser::ImportModuleStatementContext *ctx) override;
        void exitImportAllStatement(ModuleParser::ImportAllStatementContext *ctx) override;
        void enterImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx) override;
        void exitImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx) override;

        void exitExportRef(ModuleParser::ExportRefContext *ctx) override;
        void exitExportModuleStatement(ModuleParser::ExportModuleStatementContext *ctx) override;
        void exitExportAllStatement(ModuleParser::ExportAllStatementContext *ctx) override;
        void enterExportSymbolsStatement(ModuleParser::ExportSymbolsStatementContext *ctx) override;
        void exitExportSymbolsStatement(ModuleParser::ExportSymbolsStatementContext *ctx) override;

        // implemented by ModuleConstantOps
        void exitTrueLiteral(ModuleParser::TrueLiteralContext *ctx) override;
        void exitFalseLiteral(ModuleParser::FalseLiteralContext *ctx) override;
        void exitUndefLiteral(ModuleParser::UndefLiteralContext *ctx) override;
        void exitNilLiteral(ModuleParser::NilLiteralContext *ctx) override;
        void exitDecimalInteger(ModuleParser::DecimalIntegerContext *ctx) override;
        void exitHexInteger(ModuleParser::HexIntegerContext *ctx) override;
        void exitOctalInteger(ModuleParser::OctalIntegerContext *ctx) override;
        void exitDecimalFixedFloat(ModuleParser::DecimalFixedFloatContext *ctx) override;
        void exitDecimalScientificFloat(ModuleParser::DecimalScientificFloatContext *ctx) override;
        void exitHexFloat(ModuleParser::HexFloatContext *ctx) override;
        void exitCharLiteral(ModuleParser::CharLiteralContext *ctx) override;
        void exitStringLiteral(ModuleParser::StringLiteralContext *ctx) override;
        void exitUrlLiteral(ModuleParser::UrlLiteralContext *ctx) override;

        // implemented by ModuleLogicalOps
        void exitBooleanAndExpression(ModuleParser::BooleanAndExpressionContext *ctx) override;
        void exitBooleanOrExpression(ModuleParser::BooleanOrExpressionContext *ctx) override;
        void exitBooleanNotExpression(ModuleParser::BooleanNotExpressionContext *ctx) override;

        // implemented by ModuleArithmeticOps
        void exitAddExpression(ModuleParser::AddExpressionContext *ctx) override;
        void exitSubExpression(ModuleParser::SubExpressionContext *ctx) override;
        void exitMulExpression(ModuleParser::MulExpressionContext *ctx) override;
        void exitDivExpression(ModuleParser::DivExpressionContext *ctx) override;
        void exitNegExpression(ModuleParser::NegExpressionContext *ctx) override;

        // implemented by ModuleCompareOps
        void exitIsEqualExpression(ModuleParser::IsEqualExpressionContext *ctx) override;
        void exitIsLessThanExpression(ModuleParser::IsLessThanExpressionContext *ctx) override;
        void exitIsLessOrEqualExpression(ModuleParser::IsLessOrEqualExpressionContext *ctx) override;
        void exitIsGreaterThanExpression(ModuleParser::IsGreaterThanExpressionContext *ctx) override;
        void exitIsGreaterOrEqualExpression(ModuleParser::IsGreaterOrEqualExpressionContext *ctx) override;
        void exitIsAExpression(ModuleParser::IsAExpressionContext *ctx) override;

        // implemented by ModuleAssignOps
        void enterGlobalStatement(ModuleParser::GlobalStatementContext *ctx) override;
        void exitGlobalStatement(ModuleParser::GlobalStatementContext *ctx) override;
        void enterUntypedVal(ModuleParser::UntypedValContext *ctx) override;
        void exitUntypedVal(ModuleParser::UntypedValContext *ctx) override;
        void enterTypedVal(ModuleParser::TypedValContext *ctx) override;
        void exitTypedVal(ModuleParser::TypedValContext *ctx) override;
        void enterUntypedVar(ModuleParser::UntypedVarContext *ctx) override;
        void exitUntypedVar(ModuleParser::UntypedVarContext *ctx) override;
        void enterTypedVar(ModuleParser::TypedVarContext *ctx) override;
        void exitTypedVar(ModuleParser::TypedVarContext *ctx) override;
        void exitNameAssignment(ModuleParser::NameAssignmentContext *ctx) override;
        void exitMemberAssignment(ModuleParser::MemberAssignmentContext *ctx) override;
        void exitSetStatement(ModuleParser::SetStatementContext *ctx) override;

        // implemented by ModuleControlOps
        void exitIfStatement(ModuleParser::IfStatementContext *ctx) override;
        void exitIfThenElseExpression(ModuleParser::IfThenElseExpressionContext *ctx) override;
        void enterCondExpression(ModuleParser::CondExpressionContext *ctx) override;
        void exitCondWhen(ModuleParser::CondWhenContext *ctx) override;
        void exitCondElse(ModuleParser::CondElseContext *ctx) override;
        void enterCondIfStatement(ModuleParser::CondIfStatementContext *ctx) override;
        void exitCondIfWhen(ModuleParser::CondIfWhenContext *ctx) override;
        void exitCondIfElse(ModuleParser::CondIfElseContext *ctx) override;
        void enterWhileStatement(ModuleParser::WhileStatementContext *ctx) override;
        void exitWhileStatement(ModuleParser::WhileStatementContext *ctx) override;
        void enterForStatement(ModuleParser::ForStatementContext *ctx) override;
        void exitForStatement(ModuleParser::ForStatementContext * ctx) override;
        void exitReturnStatement(ModuleParser::ReturnStatementContext *ctx) override;

        // implemented by ModuleMatchOps
        void enterMatchExpression(ModuleParser::MatchExpressionContext *ctx) override;
        void exitMatchTarget(ModuleParser::MatchTargetContext *ctx) override;
        void exitMatchOnEnum(ModuleParser::MatchOnEnumContext *ctx) override;
        void exitMatchOnType(ModuleParser::MatchOnTypeContext *ctx) override;
        void exitMatchOnUnwrap(ModuleParser::MatchOnUnwrapContext *ctx) override;
        void exitMatchElse(ModuleParser::MatchElseContext *ctx) override;

        // implemented by ModuleExceptionOps
        void enterTryStatement(ModuleParser::TryStatementContext *ctx) override;
        void exitTryTarget(ModuleParser::TryTargetContext *ctx) override;
        void exitCatchOnUnwrap(ModuleParser::CatchOnUnwrapContext *ctx) override;
        void exitCatchOnType(ModuleParser::CatchOnTypeContext *ctx) override;
        void exitCatchElse(ModuleParser::CatchElseContext *ctx) override;
        void exitCatchFinally(ModuleParser::CatchFinallyContext *ctx) override;

        // implemented by ModuleDerefOps
        void enterLiteralExpression(ModuleParser::LiteralExpressionContext *ctx) override;
        void enterGroupingExpression(ModuleParser::GroupingExpressionContext *ctx) override;
        void enterThisExpression(ModuleParser::ThisExpressionContext *ctx) override;
        void enterNameExpression(ModuleParser::NameExpressionContext *ctx) override;
        void enterCallExpression(ModuleParser::CallExpressionContext *ctx) override;
        void exitDerefLiteral(ModuleParser::DerefLiteralContext *ctx) override;
        void exitDerefGrouping(ModuleParser::DerefGroupingContext *ctx) override;
        void exitThisSpec(ModuleParser::ThisSpecContext *ctx) override;
        void exitNameSpec(ModuleParser::NameSpecContext *ctx) override;
        void exitCallSpec(ModuleParser::CallSpecContext *ctx) override;
        void exitDerefMethod(ModuleParser::DerefMethodContext *ctx) override;
        void exitDerefMember(ModuleParser::DerefMemberContext *ctx) override;
        void exitLiteralExpression(ModuleParser::LiteralExpressionContext *ctx) override;
        void exitGroupingExpression(ModuleParser::GroupingExpressionContext *ctx) override;
        void exitThisExpression(ModuleParser::ThisExpressionContext *ctx) override;
        void exitNameExpression(ModuleParser::NameExpressionContext *ctx) override;
        void exitCallExpression(ModuleParser::CallExpressionContext *ctx) override;
        void exitSymbolExpression(ModuleParser::SymbolExpressionContext *ctx) override;

        // implemented by ModuleConstructOps
        void exitDerefNew(ModuleParser::DerefNewContext *ctx) override;
        void exitPairExpression(ModuleParser::PairExpressionContext *ctx) override;
        void exitLambdaExpression(ModuleParser::LambdaExpressionContext *ctx) override;
        void exitDefaultInitializerTypedNew(ModuleParser::DefaultInitializerTypedNewContext *ctx) override;
        void exitDefaultInitializerNew(ModuleParser::DefaultInitializerNewContext *ctx) override;

        // implemented by ModuleDefineOps
        void enterDefStatement(ModuleParser::DefStatementContext *ctx) override;
        void exitDefStatement(ModuleParser::DefStatementContext *ctx) override;
        void enterImplDef(ModuleParser::ImplDefContext *ctx) override;
        void exitImplDef(ModuleParser::ImplDefContext *ctx) override;
        void exitDefaliasStatement(ModuleParser::DefaliasStatementContext *ctx) override;

        // implemented by ModuleDefclassOps
        void enterDefclassStatement(ModuleParser::DefclassStatementContext *ctx) override;
        void exitClassSuper(ModuleParser::ClassSuperContext *ctx) override;
        void enterClassInit(ModuleParser::ClassInitContext *ctx) override;
        void exitClassInit(ModuleParser::ClassInitContext *ctx) override;
        void enterClassVal(ModuleParser::ClassValContext *ctx) override;
        void exitClassVal(ModuleParser::ClassValContext *ctx) override;
        void enterClassVar(ModuleParser::ClassVarContext *ctx) override;
        void exitClassVar(ModuleParser::ClassVarContext *ctx) override;
        void enterClassDef(ModuleParser::ClassDefContext *ctx) override;
        void exitClassDef(ModuleParser::ClassDefContext *ctx) override;
        void enterClassImpl(ModuleParser::ClassImplContext *ctx) override;
        void exitClassImpl(ModuleParser::ClassImplContext *ctx) override;
        void exitDefclassStatement(ModuleParser::DefclassStatementContext *ctx) override;

        // implemented by ModuleDefconceptOps
        void enterDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx) override;
        void enterConceptDecl(ModuleParser::ConceptDeclContext *ctx) override;
        void exitConceptDecl(ModuleParser::ConceptDeclContext *ctx) override;
        void enterConceptImpl(ModuleParser::ConceptImplContext *ctx) override;
        void exitConceptImpl(ModuleParser::ConceptImplContext *ctx) override;
        void exitDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx) override;

        // implemented by ModuleDefenumOps
        void enterDefenumStatement(ModuleParser::DefenumStatementContext *ctx) override;
        void enterEnumInit(ModuleParser::EnumInitContext *ctx) override;
        void exitEnumInit(ModuleParser::EnumInitContext *ctx) override;
        void enterEnumVal(ModuleParser::EnumValContext *ctx) override;
        void exitEnumVal(ModuleParser::EnumValContext *ctx) override;
        void enterEnumDef(ModuleParser::EnumDefContext *ctx) override;
        void exitEnumDef(ModuleParser::EnumDefContext *ctx) override;
        void enterEnumCase(ModuleParser::EnumCaseContext *ctx) override;
        void exitEnumCase(ModuleParser::EnumCaseContext *ctx) override;
        void enterEnumImpl(ModuleParser::EnumImplContext *ctx) override;
        void exitEnumImpl(ModuleParser::EnumImplContext *ctx) override;
        void exitDefenumStatement(ModuleParser::DefenumStatementContext *ctx) override;

        // implemented by ModuleDefinstanceOps
        void enterDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx) override;
        void enterInstanceVal(ModuleParser::InstanceValContext *ctx) override;
        void exitInstanceVal(ModuleParser::InstanceValContext *ctx) override;
        void enterInstanceVar(ModuleParser::InstanceVarContext *ctx) override;
        void exitInstanceVar(ModuleParser::InstanceVarContext *ctx) override;
        void enterInstanceDef(ModuleParser::InstanceDefContext *ctx) override;
        void exitInstanceDef(ModuleParser::InstanceDefContext *ctx) override;
        void enterInstanceImpl(ModuleParser::InstanceImplContext *ctx) override;
        void exitInstanceImpl(ModuleParser::InstanceImplContext *ctx) override;
        void exitDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx) override;

        // implemented by ModuleDefstructOps
        void enterDefstructStatement(ModuleParser::DefstructStatementContext *ctx) override;
        void exitStructSuper(ModuleParser::StructSuperContext *ctx) override;
        void enterStructInit(ModuleParser::StructInitContext *ctx) override;
        void exitStructInit(ModuleParser::StructInitContext *ctx) override;
        void enterStructVal(ModuleParser::StructValContext *ctx) override;
        void exitStructVal(ModuleParser::StructValContext *ctx) override;
        void enterStructDef(ModuleParser::StructDefContext *ctx) override;
        void exitStructDef(ModuleParser::StructDefContext *ctx) override;
        void enterStructImpl(ModuleParser::StructImplContext *ctx) override;
        void exitStructImpl(ModuleParser::StructImplContext *ctx) override;
        void exitDefstructStatement(ModuleParser::DefstructStatementContext *ctx) override;

        // implemented by ModuleParameterOps
        void enterParamSpec(ModuleParser::ParamSpecContext *ctx) override;
        void exitPositionalParam(ModuleParser::PositionalParamContext *ctx) override;
        void exitNamedParam(ModuleParser::NamedParamContext *ctx) override;
        void exitRenamedParam(ModuleParser::RenamedParamContext *ctx) override;
        void exitNamedCtx(ModuleParser::NamedCtxContext *ctx) override;
        void exitRenamedCtx(ModuleParser::RenamedCtxContext *ctx) override;
        void exitRest(ModuleParser::RestContext *ctx) override;
        void exitParamSpec(ModuleParser::ParamSpecContext *ctx) override;

        // implemented by ModuleMacroOps
        void enterMacro(ModuleParser::MacroContext *ctx) override;
        void enterAnnotationList(ModuleParser::AnnotationListContext *ctx) override;
        void exitMacroCall(ModuleParser::MacroCallContext *ctx) override;
        void exitAnnotationList(ModuleParser::AnnotationListContext *ctx) override;
        void exitMacro(ModuleParser::MacroContext *ctx) override;
    private:
        ArchetypeState *m_state;
    };
}

#endif // LYRIC_PARSER_INTERNAL_MODULE_ARCHETYPE_H
