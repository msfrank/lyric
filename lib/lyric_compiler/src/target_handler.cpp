
#include <lyric_assembler/call_symbol.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/operator_utils.h>
#include <lyric_compiler/resolve_utils.h>
#include <lyric_compiler/target_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::TargetHandler::TargetHandler(
    Target *target,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_target(target),
      m_fragment(fragment)
{
    TU_ASSERT (m_target != nullptr);
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::TargetHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    if (!node->isClass(lyric_schema::kLyricAstTargetClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected Target node");

    auto numChildren = node->numChildren();
    TU_ASSERT (numChildren > 0);

    if (numChildren > 1) {
        auto initial = std::make_unique<InitialTarget>(m_target, m_fragment);
        ctx.appendBehavior(std::move(initial));
        if (numChildren > 2) {
            for (int i = 0; i < numChildren - 2; i++) {
                auto member = std::make_unique<ResolveMember>(m_target, m_fragment);
                ctx.appendBehavior(std::move(member));
            }
        }
    }
    auto resolve = std::make_unique<ResolveTarget>(m_target);
    ctx.appendBehavior(std::move(resolve));

    return {};
}

tempo_utils::Status
lyric_compiler::TargetHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    return {};
}

lyric_compiler::InitialTarget::InitialTarget(Target *target, lyric_assembler::CodeFragment *fragment)
    : m_target(target),
      m_fragment(fragment)
{
    TU_ASSERT (m_target != nullptr);
    TU_ASSERT (m_fragment != nullptr);
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
            m_target->thisReceiver = true;
            TU_RETURN_IF_NOT_OK (deref_this(m_target->targetRef, block, m_fragment, driver));
            m_target->receiverType = driver->peekResult();
            return driver->popResult();
        }

        case lyric_schema::LyricAstId::Name: {
            m_target->bindingBlock = block;
            TU_RETURN_IF_NOT_OK (deref_name(
                node, m_target->targetRef, &m_target->bindingBlock, m_fragment, driver));
            m_target->receiverType = driver->peekResult();
            return driver->popResult();
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invlid target target node");
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

lyric_compiler::ResolveMember::ResolveMember(Target *target, lyric_assembler::CodeFragment *fragment)
    : m_target(target),
      m_fragment(fragment)
{
    TU_ASSERT (m_target != nullptr);
    TU_ASSERT (m_fragment != nullptr);
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
            if (!m_target->receiverType.isValid())
                return CompilerStatus::forCondition(
                    CompilerCondition::kCompilerInvariant, "missing receiver type");
            TU_RETURN_IF_NOT_OK (deref_member(node, m_target->targetRef, m_target->receiverType,
                m_target->thisReceiver, m_target->bindingBlock, m_fragment, driver));
            m_target->receiverType = driver->peekResult();
            return driver->popResult();
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invlid target target node");
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

lyric_compiler::ResolveTarget::ResolveTarget(Target *target)
    : m_target(target)
{
    TU_ASSERT (m_target != nullptr);
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
            if (m_target->receiverType.isValid()) {
                TU_RETURN_IF_NOT_OK (resolve_member(node, block, m_target->receiverType,
                    m_target->thisReceiver, m_target->targetRef, driver));
            } else {
                TU_RETURN_IF_NOT_OK (resolve_name(node, block, m_target->targetRef, driver));
            }
            return {};
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid target target node");
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
