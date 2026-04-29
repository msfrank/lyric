
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/type_handle.h>
#include <lyric_typing/typing_result.h>
#include <lyric_typing/unify_assignable.h>

static tempo_utils::Result<lyric_common::TypeDef>
unify_concrete_and_concrete(
    const lyric_common::TypeDef &ref1,
    const lyric_common::TypeDef &ref2,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (ref1.getType() == lyric_common::TypeDefType::Concrete);
    TU_ASSERT (ref2.getType() == lyric_common::TypeDefType::Concrete);
    auto *typeCache = state->typeCache();

    lyric_assembler::TypeSignature sig1;
    TU_ASSIGN_OR_RETURN (sig1, typeCache->resolveSignature(ref1.getConcreteUrl()));
    auto vec1 = sig1.getSignature();
    if (vec1.size() == 0)
        return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kTypeError,
            "type {} has invalid type signature", ref1.toString());

    lyric_assembler::TypeSignature sig2;
    TU_ASSIGN_OR_RETURN (sig2, typeCache->resolveSignature(ref2.getConcreteUrl()));
    auto vec2 = sig2.getSignature();
    if (vec2.size() == 0)
        return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kTypeError,
            "type {} has invalid type signature", ref2.toString());

    int minsize = std::min(vec1.size(), vec2.size());
    if (vec1[0] != vec2[0])
        return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kTypeError,
            "type {} cannot be unified with type {}", ref1.toString(), ref2.toString());

    // find the address of the most specific common ancestor type
    int i = 1;
    while (i < minsize) {
        if (vec1[i] != vec2[i])
            break;
        i++;
    }

    auto *typeHandle = vec1[i - 1];
    return typeHandle->getTypeDef();
}

static tempo_utils::Result<lyric_common::TypeDef>
unify_concrete(
    const lyric_common::TypeDef &ref1,
    const lyric_common::TypeDef &ref2,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (ref1.getType() == lyric_common::TypeDefType::Concrete);
    switch (ref2.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return unify_concrete_and_concrete(ref1, ref2, state);
        default:
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kTypeError,
                "type {} cannot be unified with type {}", ref1.toString(), ref2.toString());
    }
}

static tempo_utils::Result<lyric_common::TypeDef>
unify_placeholder(
    const lyric_common::TypeDef &ref1,
    const lyric_common::TypeDef &ref2,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (ref1.getType() == lyric_common::TypeDefType::Placeholder);
    switch (ref2.getType()) {
        case lyric_common::TypeDefType::Placeholder: {
            if (ref1 == ref2)
                return ref1;
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kTypeError,
                "type {} cannot be unified with type {}", ref1.toString(), ref2.toString());
        }
        default:
            return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kTypeError,
                "type {} cannot be unified with type {}", ref1.toString(), ref2.toString());
    }
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_typing::unify_assignable(
    const lyric_common::TypeDef &ref1,
    const lyric_common::TypeDef &ref2,
    lyric_assembler::ObjectState *state)
{
    /*
     *
     *              || to Concrete  || to Placeholder || to INTERSECTION || to UNION ||
     *              ----------------------------------------------------------------
     *     Concrete || yes             no                yes                yes
     *  Placeholder || no              yes               no                 no
     * INTERSECTION || no              no                yes                no
     *        UNION || no              no                no                 yes
     */

    switch (ref1.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return unify_concrete(ref1, ref2, state);
        case lyric_common::TypeDefType::Placeholder:
            return unify_placeholder(ref1, ref2, state);
        default:
            return TypingStatus::forCondition(TypingCondition::kTypeError,
                "type {} cannot be unified with type {}", ref1.toString(), ref2.toString());
    }
}