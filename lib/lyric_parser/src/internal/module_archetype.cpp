#include <unicode/ustring.h>

#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/parser_types.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

#include <ModuleParser.h>
#include <absl/strings/substitute.h>

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
#include <tempo_tracing/tracing_schema.h>

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

lyric_parser::internal::ModuleArchetype::ModuleArchetype(
    ArchetypeState *state,
    std::shared_ptr<tempo_tracing::TraceSpan> span)
    : ModuleArchetype(state)
{
    TU_ASSERT (span != nullptr);
    m_span = std::move(span);
}

void
lyric_parser::internal::ModuleArchetype::logErrorOrThrow(
    size_t lineNr,
    size_t columnNr,
    const std::string &message)
{
    // if this is the first error seen then set status
    if (m_status.isOk()) {
        auto fullMessage = absl::Substitute(
            "syntax error at $0:$1: $2", lineNr, columnNr, message);
        m_status = ParseStatus::forCondition(
            ParseCondition::kSyntaxError, fullMessage);
    }

    if (m_span != nullptr) {
        m_span->putTag(tempo_tracing::kOpentracingError, true);
        auto log = m_span->appendLog(absl::Now(), tempo_tracing::LogSeverity::kError);
        log->putField(tempo_tracing::kTempoTracingLineNumber, (tu_uint64) lineNr);
        log->putField(tempo_tracing::kTempoTracingColumnNumber, (tu_uint64) columnNr);
        log->putField(tempo_tracing::kOpentracingMessage, message);
    } else {
        throw tempo_utils::StatusException(m_status);
    }
}

bool
lyric_parser::internal::ModuleArchetype::hasError() const
{
    return m_status.notOk();
}

/**
 * Macro to return immediately from rule if we have detected an error.
 */
#define IGNORE_RULE_IF_HAS_ERROR   do { if (hasError()) return; } while (0);

void
lyric_parser::internal::ModuleArchetype::enterRoot(ModuleParser::RootContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->makeSpan();
}

void
lyric_parser::internal::ModuleArchetype::enterBlock(ModuleParser::BlockContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    auto location = get_token_location(ctx->getStart());
    auto *blockNode = m_state->appendNodeOrThrow(lyric_schema::kLyricAstBlockClass, location);
    m_state->pushNode(blockNode);
}

void
lyric_parser::internal::ModuleArchetype::exitForm(ModuleParser::FormContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR

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
    IGNORE_RULE_IF_HAS_ERROR

    auto *scopeManager = m_state->scopeManager();
    auto span = scopeManager->peekSpan();

    // if ancestor node is not a kBlock, then report internal violation
    if (m_state->isEmpty())
        m_state->throwIncompleteModule(get_token_location(ctx->getStop()));
    auto *rootNode = m_state->peekNode();
    m_state->checkNodeOrThrow(rootNode, lyric_schema::kLyricAstBlockClass);

    // pop the kBlock off the stack
    m_state->popNode();

    // set the root node
    m_state->setRoot(rootNode);

    // at this point m_stack should be empty
    if (!m_state->isEmpty())
        m_state->throwParseInvariant("found extra nodes on the parse stack");

    scopeManager->popSpan();
}

void
lyric_parser::internal::ModuleArchetype::exitSymbolIdentifier(ModuleParser::SymbolIdentifierContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    auto id = ctx->Identifier()->getText();
    m_state->pushSymbol(id);
}

/*
 * import ops
 */

void lyric_parser::internal::ModuleArchetype::enterNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleSymbolOps::enterNamespaceStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNamespaceSpec(ModuleParser::NamespaceSpecContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleSymbolOps::exitNamespaceSpec(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleSymbolOps::exitNamespaceStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterUsingStatement(ModuleParser::UsingStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleSymbolOps::enterUsingStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUsingRef(ModuleParser::UsingRefContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleSymbolOps::exitUsingRef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUsingType(ModuleParser::UsingTypeContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleSymbolOps::exitUsingType(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUsingStatement(ModuleParser::UsingStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleSymbolOps::exitUsingStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitImportRef(ModuleParser::ImportRefContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleSymbolOps::exitImportRef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitImportModuleStatement(ModuleParser::ImportModuleStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleSymbolOps::exitImportModuleStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitImportAllStatement(ModuleParser::ImportAllStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleSymbolOps::exitImportAllStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleSymbolOps::enterImportSymbolsStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleSymbolOps::exitImportSymbolsStatement(ctx);
}

/*
 * constant ops
 */
void lyric_parser::internal::ModuleArchetype::exitTrueLiteral(ModuleParser::TrueLiteralContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstantOps::exitTrueLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitFalseLiteral(ModuleParser::FalseLiteralContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstantOps::exitFalseLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUndefLiteral(ModuleParser::UndefLiteralContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstantOps::exitUndefLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNilLiteral(ModuleParser::NilLiteralContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstantOps::exitNilLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDecimalInteger(ModuleParser::DecimalIntegerContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstantOps::exitDecimalInteger(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitHexInteger(ModuleParser::HexIntegerContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstantOps::exitHexInteger(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitOctalInteger(ModuleParser::OctalIntegerContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstantOps::exitOctalInteger(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDecimalFixedFloat(ModuleParser::DecimalFixedFloatContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstantOps::exitDecimalFixedFloat(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDecimalScientificFloat(ModuleParser::DecimalScientificFloatContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstantOps::exitDecimalScientificFloat(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitHexFloat(ModuleParser::HexFloatContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstantOps::exitHexFloat(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCharLiteral(ModuleParser::CharLiteralContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstantOps::exitCharLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitStringLiteral(ModuleParser::StringLiteralContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstantOps::exitStringLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUrlLiteral(ModuleParser::UrlLiteralContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstantOps::exitUrlLiteral(ctx);
}

/*
 * logical ops
 */
void lyric_parser::internal::ModuleArchetype::exitBooleanAndExpression(ModuleParser::BooleanAndExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleLogicalOps::exitBooleanAndExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitBooleanOrExpression(ModuleParser::BooleanOrExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleLogicalOps::exitBooleanOrExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitBooleanNotExpression(ModuleParser::BooleanNotExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleLogicalOps::exitBooleanNotExpression(ctx);
}

/*
 * arithmetic ops
 */
void lyric_parser::internal::ModuleArchetype::exitAddExpression(ModuleParser::AddExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleArithmeticOps::exitAddExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitSubExpression(ModuleParser::SubExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleArithmeticOps::exitSubExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMulExpression(ModuleParser::MulExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleArithmeticOps::exitMulExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDivExpression(ModuleParser::DivExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleArithmeticOps::exitDivExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNegExpression(ModuleParser::NegExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleArithmeticOps::exitNegExpression(ctx);
}

/*
 * compare ops
 */
void lyric_parser::internal::ModuleArchetype::exitIsEqualExpression(ModuleParser::IsEqualExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleCompareOps::exitIsEqualExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitIsLessThanExpression(ModuleParser::IsLessThanExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleCompareOps::exitIsLessThanExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitIsLessOrEqualExpression(ModuleParser::IsLessOrEqualExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleCompareOps::exitIsLessOrEqualExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitIsGreaterThanExpression(ModuleParser::IsGreaterThanExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleCompareOps::exitIsGreaterThanExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitIsGreaterOrEqualExpression(ModuleParser::IsGreaterOrEqualExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleCompareOps::exitIsGreaterOrEqualExpression(ctx);
}

/*
 * assign ops
 */
void lyric_parser::internal::ModuleArchetype::enterGlobalStatement(ModuleParser::GlobalStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleAssignOps::enterGlobalStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitGlobalStatement(ModuleParser::GlobalStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleAssignOps::exitGlobalStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterUntypedVal(ModuleParser::UntypedValContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleAssignOps::enterUntypedVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUntypedVal(ModuleParser::UntypedValContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleAssignOps::exitUntypedVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterTypedVal(ModuleParser::TypedValContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleAssignOps::enterTypedVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitTypedVal(ModuleParser::TypedValContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleAssignOps::exitTypedVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterUntypedVar(ModuleParser::UntypedVarContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleAssignOps::enterUntypedVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitUntypedVar(ModuleParser::UntypedVarContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleAssignOps::exitUntypedVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterTypedVar(ModuleParser::TypedVarContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleAssignOps::enterTypedVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitTypedVar(ModuleParser::TypedVarContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleAssignOps::exitTypedVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNameAssignment(ModuleParser::NameAssignmentContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleAssignOps::exitNameAssignment(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMemberAssignment(ModuleParser::MemberAssignmentContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleAssignOps::exitMemberAssignment(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitSetStatement(ModuleParser::SetStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleAssignOps::exitSetStatement(ctx);
}

/*
 * control ops
 */
void lyric_parser::internal::ModuleArchetype::exitIfStatement(ModuleParser::IfStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleControlOps::exitIfStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitIfThenElseExpression(ModuleParser::IfThenElseExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleControlOps::exitIfThenElseExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterCondExpression(ModuleParser::CondExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleControlOps::enterCondExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCondWhen(ModuleParser::CondWhenContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleControlOps::exitCondWhen(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCondElse(ModuleParser::CondElseContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleControlOps::exitCondElse(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterCondIfStatement(ModuleParser::CondIfStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleControlOps::enterCondIfStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCondIfWhen(ModuleParser::CondIfWhenContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleControlOps::exitCondIfWhen(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCondIfElse(ModuleParser::CondIfElseContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleControlOps::exitCondIfElse(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleControlOps::enterWhileStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleControlOps::exitWhileStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterForStatement(ModuleParser::ForStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleControlOps::enterForStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitForStatement(ModuleParser::ForStatementContext * ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleControlOps::exitForStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitReturnStatement(ModuleParser::ReturnStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleControlOps::exitReturnStatement(ctx);
}

/*
 * match ops
 */
void lyric_parser::internal::ModuleArchetype::enterMatchExpression(ModuleParser::MatchExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMatchOps::enterMatchExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMatchTarget(ModuleParser::MatchTargetContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMatchOps::exitMatchTarget(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMatchOnEnum(ModuleParser::MatchOnEnumContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMatchOps::exitMatchOnEnum(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMatchOnType(ModuleParser::MatchOnTypeContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMatchOps::exitMatchOnType(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMatchOnUnwrap(ModuleParser::MatchOnUnwrapContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMatchOps::exitMatchOnUnwrap(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMatchElse(ModuleParser::MatchElseContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMatchOps::exitMatchElse(ctx);
}

/*
 * exception ops
 */
void lyric_parser::internal::ModuleArchetype::enterTryStatement(ModuleParser::TryStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleExceptionOps::enterTryStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitTryTarget(ModuleParser::TryTargetContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleExceptionOps::exitTryTarget(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCatchOnUnwrap(ModuleParser::CatchOnUnwrapContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleExceptionOps::exitCatchOnUnwrap(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCatchOnType(ModuleParser::CatchOnTypeContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleExceptionOps::exitCatchOnType(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCatchElse(ModuleParser::CatchElseContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleExceptionOps::exitCatchElse(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCatchFinally(ModuleParser::CatchFinallyContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleExceptionOps::exitCatchFinally(ctx);
}

/*
 * deref ops
 */
void lyric_parser::internal::ModuleArchetype::enterLiteralExpression(ModuleParser::LiteralExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::enterLiteralExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterGroupingExpression(ModuleParser::GroupingExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::enterGroupingExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterThisExpression(ModuleParser::ThisExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::enterThisExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterNameExpression(ModuleParser::NameExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::enterNameExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterCallExpression(ModuleParser::CallExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::enterCallExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDerefLiteral(ModuleParser::DerefLiteralContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitDerefLiteral(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDerefGrouping(ModuleParser::DerefGroupingContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitDerefGrouping(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitThisSpec(ModuleParser::ThisSpecContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitThisSpec(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNameSpec(ModuleParser::NameSpecContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitNameSpec(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCallSpec(ModuleParser::CallSpecContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitCallSpec(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDerefMethod(ModuleParser::DerefMethodContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitDerefMethod(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDerefMember(ModuleParser::DerefMemberContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitDerefMember(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitLiteralExpression(ModuleParser::LiteralExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitLiteralExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitGroupingExpression(ModuleParser::GroupingExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitGroupingExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitThisExpression(ModuleParser::ThisExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitThisExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNameExpression(ModuleParser::NameExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitNameExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitCallExpression(ModuleParser::CallExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitCallExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitSymbolExpression(ModuleParser::SymbolExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitSymbolExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitTypeofExpression(ModuleParser::TypeofExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDerefOps::exitTypeofExpression(ctx);
}

/*
 * construct ops
 */
void lyric_parser::internal::ModuleArchetype::exitPairExpression(ModuleParser::PairExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstructOps::exitPairExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDerefNew(ModuleParser::DerefNewContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstructOps::exitDerefNew(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitLambdaExpression(ModuleParser::LambdaExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstructOps::exitLambdaExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitLambdaFromExpression(ModuleParser::LambdaFromExpressionContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstructOps::exitLambdaFromExpression(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefaultInitializerTypedNew(ModuleParser::DefaultInitializerTypedNewContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstructOps::exitDefaultInitializerTypedNew(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefaultInitializerNew(ModuleParser::DefaultInitializerNewContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleConstructOps::exitDefaultInitializerNew(ctx);
}

/*
 * define ops
 */
void lyric_parser::internal::ModuleArchetype::exitTypenameStatement(ModuleParser::TypenameStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefineOps::exitTypenameStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterDefStatement(ModuleParser::DefStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefineOps::enterDefStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefStatement(ModuleParser::DefStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefineOps::exitDefStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterImplDef(ModuleParser::ImplDefContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefineOps::enterImplDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitImplDef(ModuleParser::ImplDefContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefineOps::exitImplDef(ctx);
}

/*
 * defclass ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefclassStatement(ModuleParser::DefclassStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefclassOps::enterDefclassStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitClassSuper(ModuleParser::ClassSuperContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefclassOps::exitClassSuper(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterClassInit(ModuleParser::ClassInitContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefclassOps::enterClassInit(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitClassInit(ModuleParser::ClassInitContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefclassOps::exitClassInit(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterClassVal(ModuleParser::ClassValContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefclassOps::enterClassVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitClassVal(ModuleParser::ClassValContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefclassOps::exitClassVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterClassVar(ModuleParser::ClassVarContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefclassOps::enterClassVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitClassVar(ModuleParser::ClassVarContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefclassOps::exitClassVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterClassDef(ModuleParser::ClassDefContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefclassOps::enterClassDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitClassDef(ModuleParser::ClassDefContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefclassOps::exitClassDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterClassImpl(ModuleParser::ClassImplContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefclassOps::enterClassImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitClassImpl(ModuleParser::ClassImplContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefclassOps::exitClassImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefclassStatement(ModuleParser::DefclassStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefclassOps::exitDefclassStatement(ctx);
}

/*
 * defconcept ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefconceptOps::enterDefconceptStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterConceptDecl(ModuleParser::ConceptDeclContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefconceptOps::enterConceptDecl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitConceptDecl(ModuleParser::ConceptDeclContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefconceptOps::exitConceptDecl(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefconceptOps::enterConceptImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefconceptOps::exitConceptImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefconceptOps::exitDefconceptStatement(ctx);
}

/*
 * defenum ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefenumStatement(ModuleParser::DefenumStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefenumOps::enterDefenumStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterEnumInit(ModuleParser::EnumInitContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefenumOps::enterEnumInit(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitEnumInit(ModuleParser::EnumInitContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefenumOps::exitEnumInit(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterEnumVal(ModuleParser::EnumValContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefenumOps::enterEnumVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitEnumVal(ModuleParser::EnumValContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefenumOps::exitEnumVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterEnumDef(ModuleParser::EnumDefContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefenumOps::enterEnumDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitEnumDef(ModuleParser::EnumDefContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefenumOps::exitEnumDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterEnumCase(ModuleParser::EnumCaseContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefenumOps::enterEnumCase(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitEnumCase(ModuleParser::EnumCaseContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefenumOps::exitEnumCase(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterEnumImpl(ModuleParser::EnumImplContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefenumOps::enterEnumImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitEnumImpl(ModuleParser::EnumImplContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefenumOps::exitEnumImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefenumStatement(ModuleParser::DefenumStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefenumOps::exitDefenumStatement(ctx);
}

/*
 * definstance ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefinstanceOps::enterDefinstanceStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterInstanceVal(ModuleParser::InstanceValContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefinstanceOps::enterInstanceVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitInstanceVal(ModuleParser::InstanceValContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefinstanceOps::exitInstanceVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterInstanceVar(ModuleParser::InstanceVarContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefinstanceOps::enterInstanceVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitInstanceVar(ModuleParser::InstanceVarContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefinstanceOps::exitInstanceVar(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterInstanceDef(ModuleParser::InstanceDefContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefinstanceOps::enterInstanceDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitInstanceDef(ModuleParser::InstanceDefContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefinstanceOps::exitInstanceDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefinstanceOps::enterInstanceImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefinstanceOps::exitInstanceImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefinstanceOps::exitDefinstanceStatement(ctx);
}

/*
 * defstruct ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefstructStatement(ModuleParser::DefstructStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefstructOps::enterDefstructStatement(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitStructSuper(ModuleParser::StructSuperContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefstructOps::exitStructSuper(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterStructInit(ModuleParser::StructInitContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefstructOps::enterStructInit(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitStructInit(ModuleParser::StructInitContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefstructOps::exitStructInit(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterStructVal(ModuleParser::StructValContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefstructOps::enterStructVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitStructVal(ModuleParser::StructValContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefstructOps::exitStructVal(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterStructDef(ModuleParser::StructDefContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefstructOps::enterStructDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitStructDef(ModuleParser::StructDefContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefstructOps::exitStructDef(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterStructImpl(ModuleParser::StructImplContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefstructOps::enterStructImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitStructImpl(ModuleParser::StructImplContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefstructOps::exitStructImpl(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefstructStatement(ModuleParser::DefstructStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefstructOps::exitDefstructStatement(ctx);
}

/*
 * defalias ops
 */
void lyric_parser::internal::ModuleArchetype::exitDefaliasStatement(ModuleParser::DefaliasStatementContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleDefineOps::exitDefaliasStatement(ctx);
}

/*
 * parameter ops
 */
void lyric_parser::internal::ModuleArchetype::enterParamSpec(ModuleParser::ParamSpecContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleParameterOps::enterParamSpec(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitPositionalParam(ModuleParser::PositionalParamContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleParameterOps::exitPositionalParam(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNamedParam(ModuleParser::NamedParamContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleParameterOps::exitNamedParam(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitRenamedParam(ModuleParser::RenamedParamContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleParameterOps::exitRenamedParam(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitNamedCtx(ModuleParser::NamedCtxContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleParameterOps::exitNamedCtx(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitRenamedCtx(ModuleParser::RenamedCtxContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleParameterOps::exitRenamedCtx(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitRest(ModuleParser::RestContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleParameterOps::exitRest(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitParamSpec(ModuleParser::ParamSpecContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleParameterOps::exitParamSpec(ctx);
}

/*
 * macro ops
 */

void lyric_parser::internal::ModuleArchetype::exitMacroArgs(ModuleParser::MacroArgsContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMacroOps::exitMacroArgs(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterMacroCall(ModuleParser::MacroCallContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMacroOps::enterMacroCall(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMacroCall(ModuleParser::MacroCallContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMacroOps::exitMacroCall(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMacroOps::enterMacroAnnotation(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMacroOps::exitMacroAnnotation(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterPragmaMacro(ModuleParser::PragmaMacroContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMacroOps::enterPragmaMacro(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitPragmaMacro(ModuleParser::PragmaMacroContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMacroOps::exitPragmaMacro(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMacroOps::enterDefinitionMacro(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMacroOps::exitDefinitionMacro(ctx);
}

void lyric_parser::internal::ModuleArchetype::enterBlockMacro(ModuleParser::BlockMacroContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMacroOps::enterBlockMacro(ctx);
}

void lyric_parser::internal::ModuleArchetype::exitBlockMacro(ModuleParser::BlockMacroContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    return ModuleMacroOps::exitBlockMacro(ctx);
}

tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::internal::ModuleArchetype::toArchetype() const
{
    TU_RETURN_IF_NOT_OK (m_status);
    return m_state->toArchetype();
}
