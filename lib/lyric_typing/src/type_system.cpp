
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/parse_assignable.h>
#include <lyric_typing/parse_pack.h>
#include <lyric_typing/resolve_assignable.h>
#include <lyric_typing/resolve_pack.h>
#include <lyric_typing/resolve_template.h>
#include <lyric_typing/type_system.h>
#include <lyric_typing/unify_assignable.h>

lyric_typing::TypeSystem::TypeSystem(lyric_assembler::AssemblyState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::AssemblyState *
lyric_typing::TypeSystem::getState() const
{
    return m_state;
}

tempo_utils::Result<lyric_parser::Assignable>
lyric_typing::TypeSystem::parseAssignable(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker)
{
    return parse_assignable(block, walker, m_state);
}

tempo_utils::Result<lyric_typing::PackSpec>
lyric_typing::TypeSystem::parsePack(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker)
{
    return parse_pack(block, walker, m_state);
}

tempo_utils::Result<lyric_assembler::ParameterPack>
lyric_typing::TypeSystem::resolvePack(
    lyric_assembler::AbstractResolver *resolver,
    const PackSpec &packSpec)
{
    return resolve_pack(packSpec, resolver, m_state);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::TypeSystem::resolveAssignable(
    lyric_assembler::AbstractResolver *resolver,
    const lyric_parser::Assignable &assignable)
{
    return resolve_assignable(assignable, resolver, m_state);
}

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::TypeSystem::compareAssignable(
    const lyric_common::TypeDef &toRef,
    const lyric_common::TypeDef &fromRef)
{
    return compare_assignable(toRef, fromRef, m_state);
}

tempo_utils::Result<bool>
lyric_typing::TypeSystem::isAssignable(
    const lyric_common::TypeDef &toRef,
    const lyric_common::TypeDef &fromRef)
{
    lyric_runtime::TypeComparison cmp;
    TU_ASSIGN_OR_RETURN (cmp, compareAssignable(toRef, fromRef));
    return (cmp == lyric_runtime::TypeComparison::EQUAL || cmp == lyric_runtime::TypeComparison::EXTENDS);
}

tempo_utils::Result<lyric_typing::TemplateSpec>
lyric_typing::TypeSystem::parseTemplate(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker)
{
    return resolve_template(block, walker, m_state);
}

tempo_utils::Result<std::pair<lyric_object::BoundType,lyric_common::TypeDef>>
lyric_typing::TypeSystem::resolveBound(const lyric_common::TypeDef &placeholderType)
{
    return resolve_bound(placeholderType, m_state);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::TypeSystem::unifyAssignable(
    const lyric_common::TypeDef &toRef,
    const lyric_common::TypeDef &fromRef)
{
    return unify_assignable(toRef, fromRef, m_state);
}

tempo_utils::Result<bool>
lyric_typing::TypeSystem::isImplementable(
    const lyric_common::TypeDef &toConcept,
    const lyric_common::TypeDef &fromRef)
{
    return is_implementable(toConcept, fromRef, m_state);
}