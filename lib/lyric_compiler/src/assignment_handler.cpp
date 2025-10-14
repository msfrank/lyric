
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/assignment_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/form_handler.h>
#include <lyric_compiler/operator_utils.h>
#include <lyric_compiler/resolve_utils.h>
#include <lyric_compiler/target_handler.h>
#include <lyric_parser/ast_attrs.h>

#include "lyric_compiler/new_handler.h"

lyric_compiler::AssignmentHandler::AssignmentHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

inline tempo_utils::Status
compile_inplace_operator(
    lyric_schema::LyricAstId astId,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
{
    switch (astId) {
        case lyric_schema::LyricAstId::InplaceAdd:
            astId = lyric_schema::LyricAstId::Add;
            break;
        case lyric_schema::LyricAstId::InplaceSub:
            astId = lyric_schema::LyricAstId::Sub;
            break;
        case lyric_schema::LyricAstId::InplaceMul:
            astId = lyric_schema::LyricAstId::Mul;
            break;
        case lyric_schema::LyricAstId::InplaceDiv:
            astId = lyric_schema::LyricAstId::Div;
            break;
        default:
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kCompilerInvariant, "invalid inplace operator");
    }

    return lyric_compiler::compile_binary_operator(astId, block, fragment, driver);
}

tempo_utils::Status
lyric_compiler::AssignmentHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before AssignmentHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();

    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());
    m_assignment.astId = resource->getId();

    m_assignment.resolveTarget = m_fragment->makeFragment();
    auto target = std::make_unique<AssignmentTarget>(&m_assignment, block, driver);
    ctx.appendChoice(std::move(target));

    m_assignment.evaluateExpression = m_fragment->makeFragment();
    auto expression = std::make_unique<AssignmentExpression>(&m_assignment, block, driver);
    ctx.appendChoice(std::move(expression));

    return {};
}

tempo_utils::Status
lyric_compiler::AssignmentHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after AssignmentHandler@" << this;

    auto *block = BaseGrouping::getBlock();
    auto *driver = getDriver();
    auto *symbolCache = driver->getSymbolCache();
    auto *typeSystem = driver->getTypeSystem();

    // pop expression type off the top of the results stack
    m_assignment.expressionType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    // apply receiver resolution if assigning to a member
    if (m_assignment.resolveTarget) {
        TU_RETURN_IF_NOT_OK (m_fragment->appendFragment(std::move(m_assignment.resolveTarget)));
    }

    lyric_common::TypeDef rvalueType;

    if (m_assignment.astId == lyric_schema::LyricAstId::Set) {
        // append the evaluation expression
        TU_RETURN_IF_NOT_OK (m_fragment->appendFragment(std::move(m_assignment.evaluateExpression)));
        // the type of the rhs of the assignment is expressionType
        rvalueType = m_assignment.expressionType;
    } else {
        // if we are modifying a member then we need to duplicate the receiver because the next load consumes it
        if (m_assignment.target.receiverType.isValid()) {
            TU_RETURN_IF_NOT_OK (m_fragment->dupValue());
        }
        // load the target (lhs) onto the stack
        TU_RETURN_IF_NOT_OK (m_fragment->loadRef(m_assignment.target.targetRef));
        TU_RETURN_IF_NOT_OK (driver->pushResult(m_assignment.target.targetRef.typeDef));
        // load the expression (rhs) onto the stack
        TU_RETURN_IF_NOT_OK (m_fragment->appendFragment(std::move(m_assignment.evaluateExpression)));
        TU_RETURN_IF_NOT_OK (driver->pushResult(m_assignment.expressionType));
        // perform the inplace operation
        TU_RETURN_IF_NOT_OK (compile_inplace_operator(m_assignment.astId, m_fragment, block, driver));
        // the type of the rhs of the assignment is the result type from the in-place operation
        rvalueType = driver->peekResult();
        TU_RETURN_IF_NOT_OK (driver->popResult());
    }

    bool isAssignable;

    // check that the rhs is assignable to the target type
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(m_assignment.target.targetRef.typeDef, rvalueType));
    if (!isAssignable)
        return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
            "target does not match rvalue type {}", rvalueType.toString());

    // check if we are in a constructor
    auto definition = block->getDefinition();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(definition));
    auto *definitionCall = cast_symbol_to_call(symbol);
    bool initialStore = definitionCall->isCtor();

    // store expression result in target
    TU_RETURN_IF_NOT_OK (m_fragment->storeRef(m_assignment.target.targetRef, initialStore));

    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::AssignmentTarget::AssignmentTarget(
    Assignment *assignment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_assignment(assignment)
{
    TU_ASSERT (m_assignment != nullptr);
}

tempo_utils::Status
lyric_compiler::AssignmentTarget::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto *block = getBlock();
    auto *driver = getDriver();

    auto astId = resource->getId();
    switch (astId) {

        case lyric_schema::LyricAstId::Target: {
            auto target = std::make_unique<TargetHandler>(
                &m_assignment->target, m_assignment->resolveTarget.get(), block, driver);
            ctx.setGrouping(std::move(target));
            return {};
        }

        case lyric_schema::LyricAstId::Name: {
            return resolve_name(node, block, m_assignment->target.targetRef, driver);
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid assignment target node");
    }
}

lyric_compiler::AssignmentExpression::AssignmentExpression(
    Assignment *assignment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_assignment(assignment)
{
    TU_ASSERT (m_assignment != nullptr);
}

tempo_utils::Status
lyric_compiler::AssignmentExpression::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto *block = getBlock();
    auto *driver = getDriver();

    auto astId = resource->getId();
    if (astId == lyric_schema::LyricAstId::New) {
        auto typeHint = m_assignment->target.targetRef.typeDef;
        auto expression = std::make_unique<NewHandler>(
            typeHint, /* isSideEffect= */ false, m_assignment->evaluateExpression.get(), block, driver);
        ctx.setGrouping(std::move(expression));
    } else {
        auto expression = std::make_unique<FormChoice>(
            FormType::Expression, m_assignment->evaluateExpression.get(), block, driver);
        ctx.setChoice(std::move(expression));
    }

    return {};
}
