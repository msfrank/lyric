
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

    auto resolveSignature1Result = state->typeCache()->resolveSignature(ref1.getConcreteUrl());
    if (resolveSignature1Result.isStatus())
        return resolveSignature1Result.getStatus();
    auto sig1 = resolveSignature1Result.getResult().getSignature();
    if (sig1.size() == 0)
        return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kTypeError,
            "type {} has invalid type signature", ref1.toString());

    auto resolveSignature2Result = state->typeCache()->resolveSignature(ref2.getConcreteUrl());
    if (resolveSignature2Result.isStatus())
        return resolveSignature2Result.getStatus();
    auto sig2 = resolveSignature2Result.getResult().getSignature();
    if (sig2.size() == 0)
        return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kTypeError,
            "type {} has invalid type signature", ref2.toString());

    int minsize = std::min(sig1.size(), sig2.size());
    if (sig1[0] != sig2[0])
        return lyric_typing::TypingStatus::forCondition(lyric_typing::TypingCondition::kTypeError,
            "type {} cannot be unified with type {}", ref1.toString(), ref2.toString());

    // find the address of the most specific common ancestor type
    int i = 1;
    while (i < minsize) {
        if (sig1[i] != sig2[i])
            break;
        i++;
    }

    auto *typeHandle = sig1[i - 1];
    return typeHandle->getTypeDef();

//    auto address = sig1[i - 1];
//
//    // resolve type address to definition symbol url
//    if (lyric_object::IS_NEAR(address.getAddress())) {
//        auto *typeHandle = state->typeCache()->getType(address);
//        TU_ASSERT (typeHandle != nullptr);
//        return typeHandle->getTypeDef();
//    }
//
//    auto commonType = state->importCache()->getLinkUrl(address.getAddress() & 0x7FFFFFFF);
//    auto unifiedType = lyric_common::TypeDef::forConcrete(
//        lyric_common::SymbolUrl(commonType.getModuleLocation(),
//        lyric_common::SymbolPath(commonType.getSymbolPath().getPath())));
//
//    return unifiedType;
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