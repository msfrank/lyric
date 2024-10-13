
#include <lyric_compiler/assignment_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/operator_utils.h>
#include <lyric_compiler/resolve_utils.h>
#include <lyric_parser/ast_attrs.h>

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
    lyric_compiler::BeforeContext &ctx)
{
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
    auto expression = std::make_unique<FormChoice>(
        FormType::Expression, m_assignment.evaluateExpression.get(), block, driver);
    ctx.appendChoice(std::move(expression));

    return {};
}

tempo_utils::Status
lyric_compiler::AssignmentHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    auto *block = BaseGrouping::getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    // pop expression type off the top of the results stack
    m_assignment.expressionType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    // apply receiver resolution if assigning to a member
    if (m_assignment.resolveTarget) {
        TU_RETURN_IF_NOT_OK (m_fragment->appendFragment(std::move(m_assignment.resolveTarget)));
    }

    lyric_common::TypeDef rvalueType;

    // apply the expression
    if (m_assignment.astId == lyric_schema::LyricAstId::Set) {
        TU_RETURN_IF_NOT_OK (m_fragment->appendFragment(std::move(m_assignment.evaluateExpression)));
        rvalueType = m_assignment.expressionType;
    } else {
        // load the target (lhs) onto the stack
        TU_RETURN_IF_NOT_OK (m_fragment->loadRef(m_assignment.targetRef));
        TU_RETURN_IF_NOT_OK (driver->pushResult(m_assignment.targetRef.typeDef));
        // load the expression (rhs) onto the stack
        TU_RETURN_IF_NOT_OK (m_fragment->appendFragment(std::move(m_assignment.evaluateExpression)));
        TU_RETURN_IF_NOT_OK (driver->pushResult(m_assignment.expressionType));
        // perform the inplace operation
        TU_RETURN_IF_NOT_OK (compile_inplace_operator(m_assignment.astId, m_fragment, block, driver));
        rvalueType = driver->peekResult();
        TU_RETURN_IF_NOT_OK (driver->popResult());
    }

    bool isAssignable;

    // check that the rhs is assignable to the target type
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(m_assignment.targetRef.typeDef, rvalueType));
    if (!isAssignable)
        return block->logAndContinue(lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "target does not match rvalue type {}", rvalueType.toString());

    TU_RETURN_IF_NOT_OK (m_fragment->storeRef(m_assignment.targetRef));

    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}

lyric_compiler::AssignmentTarget::AssignmentTarget(
    lyric_compiler::Assignment *assignment,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_assignment(assignment)
{
    TU_ASSERT (m_assignment != nullptr);
}

tempo_utils::Status
lyric_compiler::AssignmentTarget::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto *block = getBlock();
    auto *driver = getDriver();

    auto astId = resource->getId();
    switch (astId) {

        case lyric_schema::LyricAstId::Target: {
            auto target = std::make_unique<AssignmentTargetHandler>(block, m_assignment, driver);
            ctx.setGrouping(std::move(target));
            return {};
        }

        case lyric_schema::LyricAstId::Name: {
            return resolve_name(node, block, m_assignment->targetRef, driver);
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invlid assignment target node");
    }
}

lyric_compiler::AssignmentTargetHandler::AssignmentTargetHandler(
    lyric_assembler::BlockHandle *block,
    Assignment *assignment,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_assignment(assignment)
{
    TU_ASSERT (m_assignment != nullptr);
}

tempo_utils::Status
lyric_compiler::AssignmentTargetHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
{
    auto numChildren = node->numChildren();
    TU_ASSERT (numChildren > 0);

    if (numChildren > 1) {
        auto initial = std::make_unique<InitialTarget>(m_assignment);
        ctx.appendBehavior(std::move(initial));
        if (numChildren > 2) {
            for (int i = 0; i < numChildren - 2; i++) {
                auto member = std::make_unique<ResolveMember>(m_assignment);
                ctx.appendBehavior(std::move(member));
            }
        }
    }
    auto resolve = std::make_unique<ResolveTarget>(m_assignment);
    ctx.appendBehavior(std::move(resolve));

    return {};
}

tempo_utils::Status
lyric_compiler::AssignmentTargetHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    return {};
}

lyric_compiler::InitialTarget::InitialTarget(Assignment *assignment)
    : m_assignment(assignment)
{
    TU_ASSERT (m_assignment != nullptr);
}

tempo_utils::Status lyric_compiler::InitialTarget::enter(
    CompilerScanDriver *driver,
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    EnterContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {

        case lyric_schema::LyricAstId::This: {
            m_assignment->thisReceiver = true;
            TU_RETURN_IF_NOT_OK (deref_this(block, m_assignment->resolveTarget.get(), driver));
            m_assignment->receiverType = driver->peekResult();
            return driver->popResult();
        }

        case lyric_schema::LyricAstId::Name: {
            m_assignment->bindingBlock = block;
            TU_RETURN_IF_NOT_OK (deref_name(
                node, &m_assignment->bindingBlock, m_assignment->resolveTarget.get(), driver));
            m_assignment->receiverType = driver->peekResult();
            return driver->popResult();
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invlid assignment target node");
    }
}

tempo_utils::Status lyric_compiler::InitialTarget::exit(
    CompilerScanDriver *driver,
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    ExitContext &ctx)
{
    return {};
}

lyric_compiler::ResolveMember::ResolveMember(Assignment *assignment)
    : m_assignment(assignment)
{
    TU_ASSERT (m_assignment != nullptr);
}

tempo_utils::Status lyric_compiler::ResolveMember::enter(
    CompilerScanDriver *driver,
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    EnterContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {

        case lyric_schema::LyricAstId::Name: {
            if (!m_assignment->receiverType.isValid())
                return CompilerStatus::forCondition(
                    CompilerCondition::kCompilerInvariant, "missing receiver type");
            TU_RETURN_IF_NOT_OK (deref_member(
                node, m_assignment->bindingBlock, m_assignment->resolveTarget.get(),
                m_assignment->receiverType, m_assignment->thisReceiver, driver));
            m_assignment->receiverType = driver->peekResult();
            return driver->popResult();
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invlid assignment target node");
    }
}

tempo_utils::Status lyric_compiler::ResolveMember::exit(
    CompilerScanDriver *driver,
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    ExitContext &ctx)
{
    return {};
}

lyric_compiler::ResolveTarget::ResolveTarget(Assignment *assignment)
    : m_assignment(assignment)
{
    TU_ASSERT (m_assignment != nullptr);
}

tempo_utils::Status lyric_compiler::ResolveTarget::enter(
    CompilerScanDriver *driver,
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    EnterContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {

        case lyric_schema::LyricAstId::Name: {
            if (m_assignment->receiverType.isValid()) {
                TU_RETURN_IF_NOT_OK (resolve_member(node, block, m_assignment->receiverType,
                    m_assignment->thisReceiver, m_assignment->targetRef, driver));
            } else {
                TU_RETURN_IF_NOT_OK (resolve_name(node, block, m_assignment->targetRef, driver));
            }
            return {};
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invlid assignment target node");
    }
}

tempo_utils::Status lyric_compiler::ResolveTarget::exit(
    CompilerScanDriver *driver,
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    ExitContext &ctx)
{
    return {};
}
