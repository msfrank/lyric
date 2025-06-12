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
#include <lyric_parser/internal/module_defclass_ops.h>
#include <lyric_parser/internal/module_defconcept_ops.h>
#include <lyric_parser/internal/module_defenum_ops.h>
#include <lyric_parser/internal/module_define_ops.h>
#include <lyric_parser/internal/module_definstance_ops.h>
#include <lyric_parser/internal/module_defstruct_ops.h>
#include <lyric_parser/internal/module_deref_ops.h>
#include <lyric_parser/internal/module_exception_ops.h>
#include <lyric_parser/internal/module_logical_ops.h>
#include <lyric_parser/internal/module_macro_ops.h>
#include <lyric_parser/internal/module_match_ops.h>
#include <lyric_parser/internal/module_parameter_ops.h>
#include <lyric_parser/internal/module_symbol_ops.h>
#include <lyric_parser/internal/parser_utils.h>
#include <lyric_parser/internal/semantic_exception.h>
#include <tempo_tracing/enter_scope.h>
#include <tempo_tracing/exit_scope.h>
#include <tempo_tracing/tracing_schema.h>

lyric_parser::internal::ModuleArchetype::ModuleArchetype(
    ArchetypeState *state,
    std::shared_ptr<tempo_tracing::TraceContext> context)
    : m_state(state),
      m_context(std::move(context))
{
    TU_ASSERT (m_state != nullptr);
    TU_ASSERT (m_context != nullptr);
}

lyric_parser::ArchetypeState *
lyric_parser::internal::ModuleArchetype::getState() const
{
    return m_state;
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

    tempo_tracing::CurrentScope scope;
    scope.putTag(tempo_tracing::kOpentracingError, true);
    auto log = scope.appendLog(absl::Now(), tempo_tracing::LogSeverity::kError);
    log->putField(tempo_tracing::kTempoTracingLineNumber, (tu_uint64) lineNr);
    log->putField(tempo_tracing::kTempoTracingColumnNumber, (tu_uint64) columnNr);
    log->putField(tempo_tracing::kOpentracingMessage, message);
}

bool
lyric_parser::internal::ModuleArchetype::hasError() const
{
    return m_status.notOk();
}

#define LOG_ERROR_ON_EXCEPTION(ctx, stmt)               \
    try {                                               \
        stmt;                                           \
    } catch (SemanticException &ex) {                   \
        logErrorOrThrow(                                \
            ex.getLineNr(),                             \
            ex.getColumnNr(),                           \
            ex.getMessage());                           \
    } catch (std::exception &ex) {                      \
        auto *token = ctx->getStart();                  \
        std::string message((const char *) ex.what());  \
        logErrorOrThrow(                                \
            token->getLine(),                           \
            token->getCharPositionInLine(),             \
            message);                                   \
        auto eptr = std::make_exception_ptr(ex);        \
        std::rethrow_exception(eptr);                   \
    }

/**
 * Macro to return immediately from rule if we have detected an error.
 */
#define IGNORE_RULE_IF_HAS_ERROR   do { if (hasError()) return; } while (0);

void
lyric_parser::internal::ModuleArchetype::enterRoot(ModuleParser::RootContext *ctx)
{
    tempo_tracing::EnterScope scope("lyric_parser::internal::ModuleArchetype::enterRoot");
}

void
lyric_parser::internal::ModuleArchetype::enterBlock(ModuleParser::BlockContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR
    auto location = get_token_location(ctx->getStart());
    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, m_state->appendNode(lyric_schema::kLyricAstBlockClass, location));
    TU_RAISE_IF_NOT_OK (m_state->pushNode(blockNode));
}

void
lyric_parser::internal::ModuleArchetype::exitForm(ModuleParser::FormContext *ctx)
{
    IGNORE_RULE_IF_HAS_ERROR

    ArchetypeNode *formNode;
    TU_ASSIGN_OR_RAISE (formNode, m_state->popNode());

    ArchetypeNode *blockNode;
    TU_ASSIGN_OR_RAISE (blockNode, m_state->peekNode(lyric_schema::kLyricAstBlockClass));

    // otherwise append form to the block
    TU_RAISE_IF_NOT_OK (blockNode->appendChild(formNode));
}

void
lyric_parser::internal::ModuleArchetype::exitRoot(ModuleParser::RootContext *ctx)
{
    tempo_tracing::ExitScope scope;

    IGNORE_RULE_IF_HAS_ERROR

    ArchetypeNode *rootNode;
    TU_ASSIGN_OR_RAISE (rootNode, m_state->popNode(lyric_schema::kLyricAstBlockClass));

    // set the root node
    m_state->setRoot(rootNode);

    // at this point node stack should be empty
    if (!m_state->isEmpty())
        throw tempo_utils::StatusException(
            ParseStatus::forCondition(ParseCondition::kParseInvariant,
                "found extra nodes on the parse stack"));
}

void
lyric_parser::internal::ModuleArchetype::exitSymbolIdentifier(ModuleParser::SymbolIdentifierContext *ctx)
{
    // always parse symbol identifier regardless of error status
    auto id = ctx->Identifier()->getText();
    m_state->pushSymbol(id);
}

/*
 * import ops
 */

void lyric_parser::internal::ModuleArchetype::enterNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx)
{
    ModuleSymbolOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterNamespaceStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitNamespaceSpec(ModuleParser::NamespaceSpecContext *ctx)
{
    ModuleSymbolOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitNamespaceSpec(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitNamespaceStatement(ModuleParser::NamespaceStatementContext *ctx)
{
    ModuleSymbolOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitNamespaceStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterUsingStatement(ModuleParser::UsingStatementContext *ctx)
{
    ModuleSymbolOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterUsingStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitUsingRef(ModuleParser::UsingRefContext *ctx)
{
    ModuleSymbolOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitUsingRef(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitUsingType(ModuleParser::UsingTypeContext *ctx)
{
    ModuleSymbolOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitUsingType(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitUsingStatement(ModuleParser::UsingStatementContext *ctx)
{
    ModuleSymbolOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitUsingStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitImportRef(ModuleParser::ImportRefContext *ctx)
{
    ModuleSymbolOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitImportRef(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitImportModuleStatement(ModuleParser::ImportModuleStatementContext *ctx)
{
    ModuleSymbolOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitImportModuleStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitImportAllStatement(ModuleParser::ImportAllStatementContext *ctx)
{
    ModuleSymbolOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitImportAllStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx)
{
    ModuleSymbolOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterImportSymbolsStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitImportSymbolsStatement(ModuleParser::ImportSymbolsStatementContext *ctx)
{
    ModuleSymbolOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitImportSymbolsStatement(ctx));
}

/*
 * constant ops
 */
void lyric_parser::internal::ModuleArchetype::exitTrueLiteral(ModuleParser::TrueLiteralContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseTrueLiteral(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitFalseLiteral(ModuleParser::FalseLiteralContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseFalseLiteral(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitUndefLiteral(ModuleParser::UndefLiteralContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseUndefLiteral(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitNilLiteral(ModuleParser::NilLiteralContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseNilLiteral(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDecimalInteger(ModuleParser::DecimalIntegerContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseDecimalInteger(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitHexInteger(ModuleParser::HexIntegerContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseHexInteger(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitOctalInteger(ModuleParser::OctalIntegerContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseOctalInteger(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDecimalFixedFloat(ModuleParser::DecimalFixedFloatContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseDecimalFixedFloat(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDecimalScientificFloat(ModuleParser::DecimalScientificFloatContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseDecimalScientificFloat(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitHexFloat(ModuleParser::HexFloatContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseHexFloat(ctx));
}
void lyric_parser::internal::ModuleArchetype::exitInvalidNumber(ModuleParser::InvalidNumberContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseInvalidNumber(ctx));
}


void lyric_parser::internal::ModuleArchetype::exitCharLiteral(ModuleParser::CharLiteralContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseCharLiteral(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitStringLiteral(ModuleParser::StringLiteralContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseStringLiteral(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitUrlLiteral(ModuleParser::UrlLiteralContext *ctx)
{
    ModuleConstantOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseUrlLiteral(ctx));
}

/*
 * logical ops
 */
void lyric_parser::internal::ModuleArchetype::exitBooleanAndExpression(ModuleParser::BooleanAndExpressionContext *ctx)
{
    ModuleLogicalOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitBooleanAndExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitBooleanOrExpression(ModuleParser::BooleanOrExpressionContext *ctx)
{
    ModuleLogicalOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitBooleanOrExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitBooleanNotExpression(ModuleParser::BooleanNotExpressionContext *ctx)
{
    ModuleLogicalOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitBooleanNotExpression(ctx));
}

/*
 * arithmetic ops
 */
void lyric_parser::internal::ModuleArchetype::exitAddExpression(ModuleParser::AddExpressionContext *ctx)
{
    ModuleArithmeticOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseAddExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitSubExpression(ModuleParser::SubExpressionContext *ctx)
{
    ModuleArithmeticOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseSubExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitMulExpression(ModuleParser::MulExpressionContext *ctx)
{
    ModuleArithmeticOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseMulExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDivExpression(ModuleParser::DivExpressionContext *ctx)
{
    ModuleArithmeticOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseDivExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitNegExpression(ModuleParser::NegExpressionContext *ctx)
{
    ModuleArithmeticOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseNegExpression(ctx));
}

/*
 * compare ops
 */
void lyric_parser::internal::ModuleArchetype::exitIsEqualExpression(ModuleParser::IsEqualExpressionContext *ctx)
{
    ModuleCompareOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseIsEqualExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitIsLessThanExpression(ModuleParser::IsLessThanExpressionContext *ctx)
{
    ModuleCompareOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseIsLessThanExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitIsLessOrEqualExpression(ModuleParser::IsLessOrEqualExpressionContext *ctx)
{
    ModuleCompareOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseIsLessOrEqualExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitIsGreaterThanExpression(ModuleParser::IsGreaterThanExpressionContext *ctx)
{
    ModuleCompareOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseIsGreaterThanExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitIsGreaterOrEqualExpression(ModuleParser::IsGreaterOrEqualExpressionContext *ctx)
{
    ModuleCompareOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseIsGreaterOrEqualExpression(ctx));
}

/*
 * assign ops
 */
void lyric_parser::internal::ModuleArchetype::enterGlobalStatement(ModuleParser::GlobalStatementContext *ctx)
{
    ModuleAssignOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterGlobalStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitGlobalStatement(ModuleParser::GlobalStatementContext *ctx)
{
    ModuleAssignOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitGlobalStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterUntypedVal(ModuleParser::UntypedValContext *ctx)
{
    ModuleAssignOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterUntypedVal(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitUntypedVal(ModuleParser::UntypedValContext *ctx)
{
    ModuleAssignOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitUntypedVal(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterTypedVal(ModuleParser::TypedValContext *ctx)
{
    ModuleAssignOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterTypedVal(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitTypedVal(ModuleParser::TypedValContext *ctx)
{
    ModuleAssignOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitTypedVal(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterUntypedVar(ModuleParser::UntypedVarContext *ctx)
{
    ModuleAssignOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterUntypedVar(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitUntypedVar(ModuleParser::UntypedVarContext *ctx)
{
    ModuleAssignOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitUntypedVar(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterTypedVar(ModuleParser::TypedVarContext *ctx)
{
    ModuleAssignOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterTypedVar(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitTypedVar(ModuleParser::TypedVarContext *ctx)
{
    ModuleAssignOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitTypedVar(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitNameAssignment(ModuleParser::NameAssignmentContext *ctx)
{
    ModuleAssignOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseNameAssignment(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitMemberAssignment(ModuleParser::MemberAssignmentContext *ctx)
{
    ModuleAssignOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseMemberAssignment(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitSetStatement(ModuleParser::SetStatementContext *ctx)
{
    ModuleAssignOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseSetStatement(ctx));
}

/*
 * control ops
 */
void lyric_parser::internal::ModuleArchetype::exitIfStatement(ModuleParser::IfStatementContext *ctx)
{
    ModuleControlOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitIfStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitIfThenElseExpression(ModuleParser::IfThenElseExpressionContext *ctx)
{
    ModuleControlOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitIfThenElseExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterCondExpression(ModuleParser::CondExpressionContext *ctx)
{
    ModuleControlOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterCondExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitCondWhen(ModuleParser::CondWhenContext *ctx)
{
    ModuleControlOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitCondWhen(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitCondElse(ModuleParser::CondElseContext *ctx)
{
    ModuleControlOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitCondElse(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterCondIfStatement(ModuleParser::CondIfStatementContext *ctx)
{
    ModuleControlOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterCondIfStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitCondIfWhen(ModuleParser::CondIfWhenContext *ctx)
{
    ModuleControlOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitCondIfWhen(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitCondIfElse(ModuleParser::CondIfElseContext *ctx)
{
    ModuleControlOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitCondIfElse(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    ModuleControlOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterWhileStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitWhileStatement(ModuleParser::WhileStatementContext *ctx)
{
    ModuleControlOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitWhileStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterForStatement(ModuleParser::ForStatementContext *ctx)
{
    ModuleControlOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterForStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitForStatement(ModuleParser::ForStatementContext * ctx)
{
    ModuleControlOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitForStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitReturnStatement(ModuleParser::ReturnStatementContext *ctx)
{
    ModuleControlOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitReturnStatement(ctx));
}

/*
 * match ops
 */
void lyric_parser::internal::ModuleArchetype::enterMatchExpression(ModuleParser::MatchExpressionContext *ctx)
{
    ModuleMatchOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterMatchExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitMatchTarget(ModuleParser::MatchTargetContext *ctx)
{
    ModuleMatchOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitMatchTarget(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitMatchOnEnum(ModuleParser::MatchOnEnumContext *ctx)
{
    ModuleMatchOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitMatchOnEnum(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitMatchOnType(ModuleParser::MatchOnTypeContext *ctx)
{
    ModuleMatchOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitMatchOnType(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitMatchOnUnwrap(ModuleParser::MatchOnUnwrapContext *ctx)
{
    ModuleMatchOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitMatchOnUnwrap(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitMatchElse(ModuleParser::MatchElseContext *ctx)
{
    ModuleMatchOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitMatchElse(ctx));
}

/*
 * exception ops
 */
void lyric_parser::internal::ModuleArchetype::enterTryStatement(ModuleParser::TryStatementContext *ctx)
{
    ModuleExceptionOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterTryStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitTryTarget(ModuleParser::TryTargetContext *ctx)
{
    ModuleExceptionOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitTryTarget(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitCatchOnUnwrap(ModuleParser::CatchOnUnwrapContext *ctx)
{
    ModuleExceptionOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitCatchOnUnwrap(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitCatchOnType(ModuleParser::CatchOnTypeContext *ctx)
{
    ModuleExceptionOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitCatchOnType(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitCatchElse(ModuleParser::CatchElseContext *ctx)
{
    ModuleExceptionOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitCatchElse(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitCatchFinally(ModuleParser::CatchFinallyContext *ctx)
{
    ModuleExceptionOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitCatchFinally(ctx));
}

/*
 * deref ops
 */
void lyric_parser::internal::ModuleArchetype::enterLiteralExpression(ModuleParser::LiteralExpressionContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterLiteralExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterGroupingExpression(ModuleParser::GroupingExpressionContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterGroupingExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterThisExpression(ModuleParser::ThisExpressionContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterThisExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterNameExpression(ModuleParser::NameExpressionContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterNameExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterCallExpression(ModuleParser::CallExpressionContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterCallExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDerefLiteral(ModuleParser::DerefLiteralContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitDerefLiteral(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDerefGrouping(ModuleParser::DerefGroupingContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitDerefGrouping(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitThisSpec(ModuleParser::ThisSpecContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitThisSpec(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitNameSpec(ModuleParser::NameSpecContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitNameSpec(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitCallSpec(ModuleParser::CallSpecContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitCallSpec(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDerefMethod(ModuleParser::DerefMethodContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitDerefMethod(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDerefMember(ModuleParser::DerefMemberContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitDerefMember(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitLiteralExpression(ModuleParser::LiteralExpressionContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitLiteralExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitGroupingExpression(ModuleParser::GroupingExpressionContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitGroupingExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitThisExpression(ModuleParser::ThisExpressionContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitThisExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitNameExpression(ModuleParser::NameExpressionContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitNameExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitCallExpression(ModuleParser::CallExpressionContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitCallExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitSymbolExpression(ModuleParser::SymbolExpressionContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitSymbolExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitTypeofExpression(ModuleParser::TypeofExpressionContext *ctx)
{
    ModuleDerefOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitTypeofExpression(ctx));
}

/*
 * construct ops
 */
void lyric_parser::internal::ModuleArchetype::exitPairExpression(ModuleParser::PairExpressionContext *ctx)
{
    ModuleConstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parsePairExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDerefNew(ModuleParser::DerefNewContext *ctx)
{
    ModuleConstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseDerefNew(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitLambdaExpression(ModuleParser::LambdaExpressionContext *ctx)
{
    ModuleConstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseLambdaExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitLambdaFromExpression(ModuleParser::LambdaFromExpressionContext *ctx)
{
    ModuleConstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseLambdaFromExpression(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDefaultInitializerTypedNew(ModuleParser::DefaultInitializerTypedNewContext *ctx)
{
    ModuleConstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseDefaultInitializerTypedNew(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDefaultInitializerNew(ModuleParser::DefaultInitializerNewContext *ctx)
{
    ModuleConstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.parseDefaultInitializerNew(ctx));
}

/*
 * define ops
 */
void lyric_parser::internal::ModuleArchetype::exitTypenameStatement(ModuleParser::TypenameStatementContext *ctx)
{
    ModuleDefineOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitTypenameStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterDefStatement(ModuleParser::DefStatementContext *ctx)
{
    ModuleDefineOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterDefStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDefStatement(ModuleParser::DefStatementContext *ctx)
{
    ModuleDefineOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitDefStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterImplDef(ModuleParser::ImplDefContext *ctx)
{
    ModuleDefineOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterImplDef(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitImplDef(ModuleParser::ImplDefContext *ctx)
{
    ModuleDefineOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitImplDef(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDefaliasStatement(ModuleParser::DefaliasStatementContext *ctx)
{
    ModuleDefineOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitDefaliasStatement(ctx));
}

/*
 * defclass ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefclassStatement(ModuleParser::DefclassStatementContext *ctx)
{
    ModuleDefclassOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterDefclassStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitClassSuper(ModuleParser::ClassSuperContext *ctx)
{
    ModuleDefclassOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitClassSuper(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterClassInit(ModuleParser::ClassInitContext *ctx)
{
    ModuleDefclassOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterClassInit(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitClassInit(ModuleParser::ClassInitContext *ctx)
{
    ModuleDefclassOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitClassInit(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterClassVal(ModuleParser::ClassValContext *ctx)
{
    ModuleDefclassOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterClassVal(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitClassVal(ModuleParser::ClassValContext *ctx)
{
    ModuleDefclassOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitClassVal(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterClassVar(ModuleParser::ClassVarContext *ctx)
{
    ModuleDefclassOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterClassVar(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitClassVar(ModuleParser::ClassVarContext *ctx)
{
    ModuleDefclassOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitClassVar(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterClassDef(ModuleParser::ClassDefContext *ctx)
{
    ModuleDefclassOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterClassDef(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitClassDef(ModuleParser::ClassDefContext *ctx)
{
    ModuleDefclassOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitClassDef(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterClassImpl(ModuleParser::ClassImplContext *ctx)
{
    ModuleDefclassOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterClassImpl(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitClassImpl(ModuleParser::ClassImplContext *ctx)
{
    ModuleDefclassOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitClassImpl(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDefclassStatement(ModuleParser::DefclassStatementContext *ctx)
{
    ModuleDefclassOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitDefclassStatement(ctx));
}

/*
 * defconcept ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx)
{
    ModuleDefconceptOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterDefconceptStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterConceptDecl(ModuleParser::ConceptDeclContext *ctx)
{
    ModuleDefconceptOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterConceptDecl(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitConceptDecl(ModuleParser::ConceptDeclContext *ctx)
{
    ModuleDefconceptOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitConceptDecl(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    ModuleDefconceptOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterConceptImpl(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitConceptImpl(ModuleParser::ConceptImplContext *ctx)
{
    ModuleDefconceptOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitConceptImpl(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDefconceptStatement(ModuleParser::DefconceptStatementContext *ctx)
{
    ModuleDefconceptOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitDefconceptStatement(ctx));
}

/*
 * defenum ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefenumStatement(ModuleParser::DefenumStatementContext *ctx)
{
    ModuleDefenumOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterDefenumStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterEnumInit(ModuleParser::EnumInitContext *ctx)
{
    ModuleDefenumOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterEnumInit(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitEnumInit(ModuleParser::EnumInitContext *ctx)
{
    ModuleDefenumOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitEnumInit(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterEnumVal(ModuleParser::EnumValContext *ctx)
{
    ModuleDefenumOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterEnumVal(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitEnumVal(ModuleParser::EnumValContext *ctx)
{
    ModuleDefenumOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitEnumVal(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterEnumDef(ModuleParser::EnumDefContext *ctx)
{
    ModuleDefenumOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterEnumDef(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitEnumDef(ModuleParser::EnumDefContext *ctx)
{
    ModuleDefenumOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitEnumDef(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterEnumCase(ModuleParser::EnumCaseContext *ctx)
{
    ModuleDefenumOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterEnumCase(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitEnumCase(ModuleParser::EnumCaseContext *ctx)
{
    ModuleDefenumOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitEnumCase(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterEnumImpl(ModuleParser::EnumImplContext *ctx)
{
    ModuleDefenumOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterEnumImpl(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitEnumImpl(ModuleParser::EnumImplContext *ctx)
{
    ModuleDefenumOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitEnumImpl(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDefenumStatement(ModuleParser::DefenumStatementContext *ctx)
{
    ModuleDefenumOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitDefenumStatement(ctx));
}

/*
 * definstance ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx)
{
    ModuleDefinstanceOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterDefinstanceStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterInstanceVal(ModuleParser::InstanceValContext *ctx)
{
    ModuleDefinstanceOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterInstanceVal(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitInstanceVal(ModuleParser::InstanceValContext *ctx)
{
    ModuleDefinstanceOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitInstanceVal(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterInstanceVar(ModuleParser::InstanceVarContext *ctx)
{
    ModuleDefinstanceOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterInstanceVar(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitInstanceVar(ModuleParser::InstanceVarContext *ctx)
{
    ModuleDefinstanceOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitInstanceVar(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterInstanceDef(ModuleParser::InstanceDefContext *ctx)
{
    ModuleDefinstanceOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterInstanceDef(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitInstanceDef(ModuleParser::InstanceDefContext *ctx)
{
    ModuleDefinstanceOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitInstanceDef(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    ModuleDefinstanceOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterInstanceImpl(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitInstanceImpl(ModuleParser::InstanceImplContext *ctx)
{
    ModuleDefinstanceOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitInstanceImpl(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDefinstanceStatement(ModuleParser::DefinstanceStatementContext *ctx)
{
    ModuleDefinstanceOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitDefinstanceStatement(ctx));
}

/*
 * defstruct ops
 */
void lyric_parser::internal::ModuleArchetype::enterDefstructStatement(ModuleParser::DefstructStatementContext *ctx)
{
    ModuleDefstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterDefstructStatement(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitStructSuper(ModuleParser::StructSuperContext *ctx)
{
    ModuleDefstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitStructSuper(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterStructInit(ModuleParser::StructInitContext *ctx)
{
    ModuleDefstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterStructInit(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitStructInit(ModuleParser::StructInitContext *ctx)
{
    ModuleDefstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitStructInit(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterStructVal(ModuleParser::StructValContext *ctx)
{
    ModuleDefstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterStructVal(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitStructVal(ModuleParser::StructValContext *ctx)
{
    ModuleDefstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitStructVal(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterStructDef(ModuleParser::StructDefContext *ctx)
{
    ModuleDefstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterStructDef(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitStructDef(ModuleParser::StructDefContext *ctx)
{
    ModuleDefstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitStructDef(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterStructImpl(ModuleParser::StructImplContext *ctx)
{
    ModuleDefstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterStructImpl(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitStructImpl(ModuleParser::StructImplContext *ctx)
{
    ModuleDefstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitStructImpl(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDefstructStatement(ModuleParser::DefstructStatementContext *ctx)
{
    ModuleDefstructOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitDefstructStatement(ctx));
}

/*
 * parameter ops
 */
void lyric_parser::internal::ModuleArchetype::enterParamSpec(ModuleParser::ParamSpecContext *ctx)
{
    ModuleParameterOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterParamSpec(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitPositionalParam(ModuleParser::PositionalParamContext *ctx)
{
    ModuleParameterOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitPositionalParam(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitNamedParam(ModuleParser::NamedParamContext *ctx)
{
    ModuleParameterOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitNamedParam(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitRenamedParam(ModuleParser::RenamedParamContext *ctx)
{
    ModuleParameterOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitRenamedParam(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitNamedCtx(ModuleParser::NamedCtxContext *ctx)
{
    ModuleParameterOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitNamedCtx(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitRenamedCtx(ModuleParser::RenamedCtxContext *ctx)
{
    ModuleParameterOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitRenamedCtx(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitRest(ModuleParser::RestContext *ctx)
{
    ModuleParameterOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitRest(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitParamSpec(ModuleParser::ParamSpecContext *ctx)
{
    ModuleParameterOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitParamSpec(ctx));
}

/*
 * macro ops
 */

void lyric_parser::internal::ModuleArchetype::exitMacroArgs(ModuleParser::MacroArgsContext *ctx)
{
    ModuleMacroOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitMacroArgs(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterMacroCall(ModuleParser::MacroCallContext *ctx)
{
    ModuleMacroOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterMacroCall(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitMacroCall(ModuleParser::MacroCallContext *ctx)
{
    ModuleMacroOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitMacroCall(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx)
{
    ModuleMacroOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterMacroAnnotation(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitMacroAnnotation(ModuleParser::MacroAnnotationContext *ctx)
{
    ModuleMacroOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitMacroAnnotation(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterPragmaMacro(ModuleParser::PragmaMacroContext *ctx)
{
    ModuleMacroOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterPragmaMacro(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitPragmaMacro(ModuleParser::PragmaMacroContext *ctx)
{
    ModuleMacroOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitPragmaMacro(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx)
{
    ModuleMacroOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterDefinitionMacro(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitDefinitionMacro(ModuleParser::DefinitionMacroContext *ctx)
{
    ModuleMacroOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitDefinitionMacro(ctx));
}

void lyric_parser::internal::ModuleArchetype::enterBlockMacro(ModuleParser::BlockMacroContext *ctx)
{
    ModuleMacroOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.enterBlockMacro(ctx));
}

void lyric_parser::internal::ModuleArchetype::exitBlockMacro(ModuleParser::BlockMacroContext *ctx)
{
    ModuleMacroOps ops(this);
    LOG_ERROR_ON_EXCEPTION (ctx, ops.exitBlockMacro(ctx));
}

tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::internal::ModuleArchetype::toArchetype() const
{
    TU_RETURN_IF_NOT_OK (m_status);
    return m_state->toArchetype();
}
