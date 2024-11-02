#include <unicode/ustring.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

#include <ModuleParser.h>

#include <lyric_parser/internal/module_archetype.h>
#include <lyric_parser/internal/module_arithmetic_ops.h>
#include <lyric_parser/internal/module_assign_ops.h>
#include <lyric_parser/internal/module_compare_ops.h>
#include <lyric_parser/internal/module_constant_ops.h>
#include <lyric_parser/internal/module_construct_ops.h>
#include <lyric_parser/internal/module_control_ops.h>
#include <lyric_parser/internal/module_define_ops.h>
#include <lyric_parser/internal/module_deref_ops.h>
#include <lyric_parser/internal/module_symbol_ops.h>
#include <lyric_parser/internal/module_logical_ops.h>
#include <lyric_parser/internal/module_match_ops.h>
#include <lyric_parser/internal/module_parameter_ops.h>
#include <lyric_parser/internal/parser_utils.h>

lyric_parser::internal::ModuleArchetype::ModuleArchetype(ArchetypeState *state)
    : ModuleSymbolOps(state),
      ModuleConstantOps(state),
      ModuleLogicalOps(state),
      ModuleArithmeticOps(state),
      ModuleCompareOps(state),
      ModuleAssignOps(state),
      ModuleControlOps(state),
      ModuleMatchOps(state),
      ModuleDerefOps(state),
      ModuleConstructOps(state),
      ModuleDefclassOps(state),
      ModuleDefconceptOps(state),
      ModuleDefenumOps(state),
      ModuleDefinstanceOps(state),
      ModuleDefstructOps(state),
      ModuleDefineOps(state),
      ModuleParameterOps(state),
      ModuleExceptionOps(state),
      ModuleMacroOps(state),
      m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

void
lyric_parser::internal::ModuleArchetype::enterRoot(ModuleParser::RootContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleArchetype::enterBlock(ModuleParser::BlockContext *ctx)
{
    auto location = get_token_location(ctx->getStart());
    auto *blockNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstBlockClass, location);
    m_state->pushNode(blockNode);
}

void
lyric_parser::internal::ModuleArchetype::exitForm(ModuleParser::FormContext *ctx)
{
    // if stack is empty, then mark source as incomplete
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *formNode = m_state->popNode();

    // if ancestor node is not a kBlock, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *blockNode = m_state->peekNode();
    m_state->checkNodeOrThrow(blockNode, lyric_schema::kLyricAstBlockClass);

    // otherwise append form to the block
    blockNode->appendChild(formNode);
}

void
lyric_parser::internal::ModuleArchetype::exitRoot(ModuleParser::RootContext *ctx)
{
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();

    // if ancestor node is not a kBlock, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto blockNode = m_state->peekNode();
    m_state->checkNodeOrThrow(blockNode, lyric_schema::kLyricAstBlockClass);

    // pop the kBlock off the stack
    m_state->popNode();

    // at this point m_stack should be empty
    if (!m_state->isEmpty())
        m_state->throwParseInvariant("found extra nodes on the parse stack");

    scopeManager->popSpan();
}

void
lyric_parser::internal::ModuleArchetype::exitSymbolIdentifier(ModuleParser::SymbolIdentifierContext *ctx)
{
    auto id = ctx->Identifier()->getText();
    m_state->pushSymbol(id);
}

/*
 * import ops
 */

void lyric_parser::internal::ModuleArchetype::enterNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx)
{
    return ModuleSymbolOps::enterNamespaceStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNamespaceSpec(ModuleParser::NamespaceSpecContext *ctx)
{
    return ModuleSymbolOps::exitNamespaceSpec(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx)
{
    return ModuleSymbolOps::exitNamespaceStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUsingPath(ModuleParser::UsingPathContext *ctx)
{
    return ModuleSymbolOps::exitUsingPath(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterUsingFromStatement(ModuleParser::UsingFromStatementContext *ctx)
{
    return ModuleSymbolOps::enterUsingFromStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUsingFromStatement(ModuleParser::UsingFromStatementContext *ctx)
{
    return ModuleSymbolOps::exitUsingFromStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterUsingLocalStatement(ModuleParser::UsingLocalStatementContext *ctx)
{
    return ModuleSymbolOps::enterUsingLocalStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUsingLocalStatement(ModuleParser::UsingLocalStatementContext *ctx)
{
    return ModuleSymbolOps::exitUsingLocalStatement(ctx);
}
void lyric_parser::internal::ModuleArchetype::exitImportRef(ModuleParser::ImportRefContext *ctx)
{
    return ModuleSymbolOps::exitImportRef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitImportModuleStatement(ModuleParser::ImportModuleStatementContext *ctx)
{
    return ModuleSymbolOps::exitImportModuleStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitImportAllStatement(ModuleParser::ImportAllStatementContext *ctx)
{
    return ModuleSymbolOps::exitImportAllStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx)
{
    return ModuleSymbolOps::enterImportSymbolsStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx)
{
    return ModuleSymbolOps::exitImportSymbolsStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitExportRef(ModuleParser::ExportRefContext *ctx)
{
    return ModuleSymbolOps::exitExportRef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitExportModuleStatement(ModuleParser::ExportModuleStatementContext *ctx)
{
    return ModuleSymbolOps::exitExportModuleStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitExportAllStatement(ModuleParser::ExportAllStatementContext *ctx)
{
    return ModuleSymbolOps::exitExportAllStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterExportSymbolsStatement(ModuleParser::ExportSymbolsStatementContext *ctx)
{
    return ModuleSymbolOps::enterExportSymbolsStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitExportSymbolsStatement(ModuleParser::ExportSymbolsStatementContext *ctx)
{
    return ModuleSymbolOps::exitExportSymbolsStatement(ctx);
}

/*
 * constant ops
 */
void lyric_parser::internal::ModuleArchetype::exitTrueLiteral(ModuleParser::TrueLiteralContext *ctx)
{
    return ModuleConstantOps::exitTrueLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitFalseLiteral(ModuleParser::FalseLiteralContext *ctx)
{
    return ModuleConstantOps::exitFalseLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUndefLiteral(ModuleParser::UndefLiteralContext *ctx)
{
    return ModuleConstantOps::exitUndefLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNilLiteral(ModuleParser::NilLiteralContext *ctx)
{
    return ModuleConstantOps::exitNilLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDecimalInteger(ModuleParser::DecimalIntegerContext *ctx)
{
    return ModuleConstantOps::exitDecimalInteger(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitHexInteger(ModuleParser::HexIntegerContext *ctx)
{
    return ModuleConstantOps::exitHexInteger(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitOctalInteger(ModuleParser::OctalIntegerContext *ctx)
{
    return ModuleConstantOps::exitOctalInteger(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDecimalFixedFloat(ModuleParser::DecimalFixedFloatContext *ctx)
{
    return ModuleConstantOps::exitDecimalFixedFloat(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDecimalScientificFloat(ModuleParser::DecimalScientificFloatContext *ctx)
{
    return ModuleConstantOps::exitDecimalScientificFloat(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitHexFloat(ModuleParser::HexFloatContext *ctx)
{
    return ModuleConstantOps::exitHexFloat(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCharLiteral(ModuleParser::CharLiteralContext *ctx)
{
    return ModuleConstantOps::exitCharLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitStringLiteral(ModuleParser::StringLiteralContext *ctx)
{
    return ModuleConstantOps::exitStringLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUrlLiteral(ModuleParser::UrlLiteralContext *ctx)
{
    return ModuleConstantOps::exitUrlLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitSymbolLiteral(ModuleParser::SymbolLiteralContext *ctx)
{
    return ModuleConstantOps::exitSymbolLiteral(ctx);
}

/*
 * logical ops
 */
void lyric_parser::internal::ModuleArchetype::exitBooleanAndExpression(ModuleParser::BooleanAndExpressionContext *ctx)
{
    return ModuleLogicalOps::exitBooleanAndExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitBooleanOrExpression(ModuleParser::BooleanOrExpressionContext *ctx)
{
    return ModuleLogicalOps::exitBooleanOrExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitBooleanNotExpression(ModuleParser::BooleanNotExpressionContext *ctx)
{
    return ModuleLogicalOps::exitBooleanNotExpression(ctx);
}

/*
 * arithmetic ops
 */
void lyric_parser::internal::ModuleArchetype::exitAddExpression(ModuleParser::AddExpressionContext *ctx)
{
    return ModuleArithmeticOps::exitAddExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitSubExpression(ModuleParser::SubExpressionContext *ctx)
{
    return ModuleArithmeticOps::exitSubExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMulExpression(ModuleParser::MulExpressionContext *ctx)
{
    return ModuleArithmeticOps::exitMulExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDivExpression(ModuleParser::DivExpressionContext *ctx)
{
    return ModuleArithmeticOps::exitDivExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNegExpression(ModuleParser::NegExpressionContext *ctx)
{
    return ModuleArithmeticOps::exitNegExpression(ctx);
}

/*
 * compare ops
 */
void lyric_parser::internal::ModuleArchetype::exitIsEqualExpression(ModuleParser::IsEqualExpressionContext *ctx)
{
    return ModuleCompareOps::exitIsEqualExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitIsLessThanExpression(ModuleParser::IsLessThanExpressionContext *ctx)
{
    return ModuleCompareOps::exitIsLessThanExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitIsLessOrEqualExpression(ModuleParser::IsLessOrEqualExpressionContext *ctx)
{
    return ModuleCompareOps::exitIsLessOrEqualExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitIsGreaterThanExpression(ModuleParser::IsGreaterThanExpressionContext *ctx)
{
    return ModuleCompareOps::exitIsGreaterThanExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitIsGreaterOrEqualExpression(ModuleParser::IsGreaterOrEqualExpressionContext *ctx)
{
    return ModuleCompareOps::exitIsGreaterOrEqualExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitIsAExpression(ModuleParser::IsAExpressionContext *ctx)
{
    return ModuleCompareOps::exitIsAExpression(ctx);
}

/*
 * assign ops
 */
void lyric_parser::internal::ModuleArchetype::enterGlobalStatement(ModuleParser::GlobalStatementContext *ctx)
{
    return ModuleAssignOps::enterGlobalStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitGlobalStatement(ModuleParser::GlobalStatementContext *ctx)
{
    return ModuleAssignOps::exitGlobalStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterUntypedVal(ModuleParser::UntypedValContext *ctx)
{
    return ModuleAssignOps::enterUntypedVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUntypedVal(ModuleParser::UntypedValContext *ctx)
{
    return ModuleAssignOps::exitUntypedVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterTypedVal(ModuleParser::TypedValContext *ctx)
{
    return ModuleAssignOps::enterTypedVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitTypedVal(ModuleParser::TypedValContext *ctx)
{
    return ModuleAssignOps::exitTypedVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterUntypedVar(ModuleParser::UntypedVarContext *ctx)
{
    return ModuleAssignOps::enterUntypedVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUntypedVar(ModuleParser::UntypedVarContext *ctx)
{
    return ModuleAssignOps::exitUntypedVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterTypedVar(ModuleParser::TypedVarContext *ctx)
{
    return ModuleAssignOps::enterTypedVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitTypedVar(ModuleParser::TypedVarContext *ctx)
{
    return ModuleAssignOps::exitTypedVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNameAssignment(ModuleParser::NameAssignmentContext *ctx)
{
    return ModuleAssignOps::exitNameAssignment(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMemberAssignment(ModuleParser::MemberAssignmentContext *ctx)
{
    return ModuleAssignOps::exitMemberAssignment(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitSetStatement(ModuleParser::SetStatementContext *ctx)
{
    return ModuleAssignOps::exitSetStatement(ctx);
}

/*
 * control ops
 */
void lyric_parser::internal::ModuleArchetype::exitIfStatement(ModuleParser::IfStatementContext *ctx)
{
    return ModuleControlOps::exitIfStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitIfThenElseExpression(ModuleParser::IfThenElseExpressionContext *ctx)
{
    return ModuleControlOps::exitIfThenElseExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterCondExpression(ModuleParser::CondExpressionContext *ctx)
{
    return ModuleControlOps::enterCondExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCondWhen(ModuleParser::CondWhenContext *ctx)
{
    return ModuleControlOps::exitCondWhen(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCondElse(ModuleParser::CondElseContext *ctx)
{
    return ModuleControlOps::exitCondElse(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterCondIfStatement(ModuleParser::CondIfStatementContext *ctx)
{
    return ModuleControlOps::enterCondIfStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCondIfWhen(ModuleParser::CondIfWhenContext *ctx)
{
    return ModuleControlOps::exitCondIfWhen(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCondIfElse(ModuleParser::CondIfElseContext *ctx)
{
    return ModuleControlOps::exitCondIfElse(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    return ModuleControlOps::enterWhileStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    return ModuleControlOps::exitWhileStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterForStatement(ModuleParser::ForStatementContext *ctx)
{
    return ModuleControlOps::enterForStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitForStatement(ModuleParser::ForStatementContext * ctx)
{
    return ModuleControlOps::exitForStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitReturnStatement(ModuleParser::ReturnStatementContext *ctx)
{
    return ModuleControlOps::exitReturnStatement(ctx);
}

/*
 * match ops
 */
void lyric_parser::internal::ModuleArchetype::enterMatchExpression(ModuleParser::MatchExpressionContext *ctx)
{
    return ModuleMatchOps::enterMatchExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMatchTarget(ModuleParser::MatchTargetContext *ctx)
{
    return ModuleMatchOps::exitMatchTarget(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMatchOnEnum(ModuleParser::MatchOnEnumContext *ctx)
{
    return ModuleMatchOps::exitMatchOnEnum(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMatchOnType(ModuleParser::MatchOnTypeContext *ctx)
{
    return ModuleMatchOps::exitMatchOnType(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMatchOnUnwrap(ModuleParser::MatchOnUnwrapContext *ctx)
{
    return ModuleMatchOps::exitMatchOnUnwrap(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMatchElse(ModuleParser::MatchElseContext *ctx)
{
    return ModuleMatchOps::exitMatchElse(ctx);
}

/*
 * exception ops
 */
void lyric_parser::internal::ModuleArchetype::enterTryStatement(ModuleParser::TryStatementContext *ctx)
{
    return ModuleExceptionOps::enterTryStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitTryTarget(ModuleParser::TryTargetContext *ctx)
{
    return ModuleExceptionOps::exitTryTarget(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCatchOnUnwrap(ModuleParser::CatchOnUnwrapContext *ctx)
{
    return ModuleExceptionOps::exitCatchOnUnwrap(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCatchOnType(ModuleParser::CatchOnTypeContext *ctx)
{
    return ModuleExceptionOps::exitCatchOnType(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCatchElse(ModuleParser::CatchElseContext *ctx)
{
    return ModuleExceptionOps::exitCatchElse(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCatchFinally(ModuleParser::CatchFinallyContext *ctx)
{
    return ModuleExceptionOps::exitCatchFinally(ctx);
}

/*
 * deref ops
 */
void lyric_parser::internal::ModuleArchetype::enterLiteralExpression(ModuleParser::LiteralExpressionContext *ctx)
{
    return ModuleDerefOps::enterLiteralExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterGroupingExpression(ModuleParser::GroupingExpressionContext *ctx)
{
    return ModuleDerefOps::enterGroupingExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterThisExpression(ModuleParser::ThisExpressionContext *ctx)
{
    return ModuleDerefOps::enterThisExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterNameExpression(ModuleParser::NameExpressionContext *ctx)
{
    return ModuleDerefOps::enterNameExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterCallExpression(ModuleParser::CallExpressionContext *ctx)
{
    return ModuleDerefOps::enterCallExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDerefLiteral(ModuleParser::DerefLiteralContext *ctx)
{
    return ModuleDerefOps::exitDerefLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDerefGrouping(ModuleParser::DerefGroupingContext *ctx)
{
    return ModuleDerefOps::exitDerefGrouping(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitThisSpec(ModuleParser::ThisSpecContext *ctx)
{
    return ModuleDerefOps::exitThisSpec(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNameSpec(ModuleParser::NameSpecContext *ctx)
{
    return ModuleDerefOps::exitNameSpec(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCallSpec(ModuleParser::CallSpecContext *ctx)
{
    return ModuleDerefOps::exitCallSpec(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDerefMethod(ModuleParser::DerefMethodContext *ctx)
{
    return ModuleDerefOps::exitDerefMethod(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDerefMember(ModuleParser::DerefMemberContext *ctx)
{
    return ModuleDerefOps::exitDerefMember(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitLiteralExpression(ModuleParser::LiteralExpressionContext *ctx)
{
    return ModuleDerefOps::exitLiteralExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitGroupingExpression(ModuleParser::GroupingExpressionContext *ctx)
{
    return ModuleDerefOps::exitGroupingExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitThisExpression(ModuleParser::ThisExpressionContext *ctx)
{
    return ModuleDerefOps::exitThisExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNameExpression(ModuleParser::NameExpressionContext *ctx)
{
    return ModuleDerefOps::exitNameExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCallExpression(ModuleParser::CallExpressionContext *ctx)
{
    return ModuleDerefOps::exitCallExpression(ctx);
}

/*
 * construct ops
 */
void lyric_parser::internal::ModuleArchetype::exitPairExpression(ModuleParser::PairExpressionContext *ctx)
{
    return ModuleConstructOps::exitPairExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDerefNew(ModuleParser::DerefNewContext *ctx)
{
    return ModuleConstructOps::exitDerefNew(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitLambdaExpression(ModuleParser::LambdaExpressionContext *ctx)
{
    return ModuleConstructOps::exitLambdaExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefaultInitializerTypedNew(ModuleParser::DefaultInitializerTypedNewContext *ctx)
{
    return ModuleConstructOps::exitDefaultInitializerTypedNew(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefaultInitializerNew(ModuleParser::DefaultInitializerNewContext *ctx)
{
    return ModuleConstructOps::exitDefaultInitializerNew(ctx);
}

/*
 * define ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefStatement(ModuleParser::DefStatementContext *ctx)
{
    return ModuleDefineOps::enterDefStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefStatement(ModuleParser::DefStatementContext *ctx)
{
    return ModuleDefineOps::exitDefStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterImplDef(ModuleParser::ImplDefContext *ctx)
{
    return ModuleDefineOps::enterImplDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitImplDef(ModuleParser::ImplDefContext *ctx)
{
    return ModuleDefineOps::exitImplDef(ctx);
}

/*
 * defclass ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefclassStatement(ModuleParser::DefclassStatementContext *ctx)
{
    return ModuleDefclassOps::enterDefclassStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitClassSuper(ModuleParser::ClassSuperContext *ctx)
{
    return ModuleDefclassOps::exitClassSuper(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterClassInit(ModuleParser::ClassInitContext *ctx)
{
    return ModuleDefclassOps::enterClassInit(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitClassInit(ModuleParser::ClassInitContext *ctx)
{
    return ModuleDefclassOps::exitClassInit(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterClassVal(ModuleParser::ClassValContext *ctx)
{
    return ModuleDefclassOps::enterClassVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitClassVal(ModuleParser::ClassValContext *ctx)
{
    return ModuleDefclassOps::exitClassVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterClassVar(ModuleParser::ClassVarContext *ctx)
{
    return ModuleDefclassOps::enterClassVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitClassVar(ModuleParser::ClassVarContext *ctx)
{
    return ModuleDefclassOps::exitClassVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterClassDef(ModuleParser::ClassDefContext *ctx)
{
    return ModuleDefclassOps::enterClassDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitClassDef(ModuleParser::ClassDefContext *ctx)
{
    return ModuleDefclassOps::exitClassDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterClassImpl(ModuleParser::ClassImplContext *ctx)
{
    return ModuleDefclassOps::enterClassImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitClassImpl(ModuleParser::ClassImplContext *ctx)
{
    return ModuleDefclassOps::exitClassImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefclassStatement(ModuleParser::DefclassStatementContext *ctx)
{
    return ModuleDefclassOps::exitDefclassStatement(ctx);
}

/*
 * defconcept ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx)
{
    return ModuleDefconceptOps::enterDefconceptStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterConceptDecl(ModuleParser::ConceptDeclContext *ctx)
{
    return ModuleDefconceptOps::enterConceptDecl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitConceptDecl(ModuleParser::ConceptDeclContext *ctx)
{
    return ModuleDefconceptOps::exitConceptDecl(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    return ModuleDefconceptOps::enterConceptImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    return ModuleDefconceptOps::exitConceptImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx)
{
    return ModuleDefconceptOps::exitDefconceptStatement(ctx);
}

/*
 * defenum ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefenumStatement(ModuleParser::DefenumStatementContext *ctx)
{
    return ModuleDefenumOps::enterDefenumStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterEnumInit(ModuleParser::EnumInitContext *ctx)
{
    return ModuleDefenumOps::enterEnumInit(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitEnumInit(ModuleParser::EnumInitContext *ctx)
{
    return ModuleDefenumOps::exitEnumInit(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterEnumVal(ModuleParser::EnumValContext *ctx)
{
    return ModuleDefenumOps::enterEnumVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitEnumVal(ModuleParser::EnumValContext *ctx)
{
    return ModuleDefenumOps::exitEnumVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterEnumDef(ModuleParser::EnumDefContext *ctx)
{
    return ModuleDefenumOps::enterEnumDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitEnumDef(ModuleParser::EnumDefContext *ctx)
{
    return ModuleDefenumOps::exitEnumDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterEnumCase(ModuleParser::EnumCaseContext *ctx)
{
    return ModuleDefenumOps::enterEnumCase(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitEnumCase(ModuleParser::EnumCaseContext *ctx)
{
    return ModuleDefenumOps::exitEnumCase(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterEnumImpl(ModuleParser::EnumImplContext *ctx)
{
    return ModuleDefenumOps::enterEnumImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitEnumImpl(ModuleParser::EnumImplContext *ctx)
{
    return ModuleDefenumOps::exitEnumImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefenumStatement(ModuleParser::DefenumStatementContext *ctx)
{
    return ModuleDefenumOps::exitDefenumStatement(ctx);
}

/*
 * definstance ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx)
{
    return ModuleDefinstanceOps::enterDefinstanceStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterInstanceVal(ModuleParser::InstanceValContext *ctx)
{
    return ModuleDefinstanceOps::enterInstanceVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitInstanceVal(ModuleParser::InstanceValContext *ctx)
{
    return ModuleDefinstanceOps::exitInstanceVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterInstanceVar(ModuleParser::InstanceVarContext *ctx)
{
    return ModuleDefinstanceOps::enterInstanceVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitInstanceVar(ModuleParser::InstanceVarContext *ctx)
{
    return ModuleDefinstanceOps::exitInstanceVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterInstanceDef(ModuleParser::InstanceDefContext *ctx)
{
    return ModuleDefinstanceOps::enterInstanceDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitInstanceDef(ModuleParser::InstanceDefContext *ctx)
{
    return ModuleDefinstanceOps::exitInstanceDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    return ModuleDefinstanceOps::enterInstanceImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    return ModuleDefinstanceOps::exitInstanceImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx)
{
    return ModuleDefinstanceOps::exitDefinstanceStatement(ctx);
}

/*
 * defstruct ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefstructStatement(ModuleParser::DefstructStatementContext *ctx)
{
    return ModuleDefstructOps::enterDefstructStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitStructSuper(ModuleParser::StructSuperContext *ctx)
{
    return ModuleDefstructOps::exitStructSuper(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterStructInit(ModuleParser::StructInitContext *ctx)
{
    return ModuleDefstructOps::enterStructInit(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitStructInit(ModuleParser::StructInitContext *ctx)
{
    return ModuleDefstructOps::exitStructInit(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterStructVal(ModuleParser::StructValContext *ctx)
{
    return ModuleDefstructOps::enterStructVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitStructVal(ModuleParser::StructValContext *ctx)
{
    return ModuleDefstructOps::exitStructVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterStructDef(ModuleParser::StructDefContext *ctx)
{
    return ModuleDefstructOps::enterStructDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitStructDef(ModuleParser::StructDefContext *ctx)
{
    return ModuleDefstructOps::exitStructDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterStructImpl(ModuleParser::StructImplContext *ctx)
{
    return ModuleDefstructOps::enterStructImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitStructImpl(ModuleParser::StructImplContext *ctx)
{
    return ModuleDefstructOps::exitStructImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefstructStatement(ModuleParser::DefstructStatementContext *ctx)
{
    return ModuleDefstructOps::exitDefstructStatement(ctx);
}

/*
 * defalias ops
 */
void lyric_parser::internal::ModuleArchetype::exitDefaliasStatement(ModuleParser::DefaliasStatementContext *ctx)
{
    return ModuleDefineOps::exitDefaliasStatement(ctx);
}

/*
 * parameter ops
 */
void lyric_parser::internal::ModuleArchetype::enterParamSpec(ModuleParser::ParamSpecContext *ctx)
{
    return ModuleParameterOps::enterParamSpec(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitPositionalParam(ModuleParser::PositionalParamContext *ctx)
{
    return ModuleParameterOps::exitPositionalParam(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNamedParam(ModuleParser::NamedParamContext *ctx)
{
    return ModuleParameterOps::exitNamedParam(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitRenamedParam(ModuleParser::RenamedParamContext *ctx)
{
    return ModuleParameterOps::exitRenamedParam(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNamedCtx(ModuleParser::NamedCtxContext *ctx)
{
    return ModuleParameterOps::exitNamedCtx(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitRenamedCtx(ModuleParser::RenamedCtxContext *ctx)
{
    return ModuleParameterOps::exitRenamedCtx(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitRest(ModuleParser::RestContext *ctx)
{
    return ModuleParameterOps::exitRest(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitParamSpec(ModuleParser::ParamSpecContext *ctx)
{
    return ModuleParameterOps::exitParamSpec(ctx);
}

/*
 * macro ops
 */

void lyric_parser::internal::ModuleArchetype::enterMacro(ModuleParser::MacroContext *ctx)
{
    return ModuleMacroOps::enterMacro(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterAnnotationList(ModuleParser::AnnotationListContext *ctx)
{
    return ModuleMacroOps::enterAnnotationList(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMacroCall(ModuleParser::MacroCallContext *ctx)
{
    return ModuleMacroOps::exitMacroCall(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitAnnotationList(ModuleParser::AnnotationListContext *ctx)
{
    return ModuleMacroOps::exitAnnotationList(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMacro(ModuleParser::MacroContext *ctx)
{
    return ModuleMacroOps::exitMacro(ctx);
}
