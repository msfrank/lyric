
#include <lyric_typing/compare_assignable.h>
#include <lyric_typing/internal/check_placeholder.h>
#include <lyric_typing/typing_result.h>

tempo_utils::Status
lyric_typing::internal::check_placeholder(
    const lyric_object::TemplateParameter &tp,
    const lyric_common::TypeDef &arg,
    lyric_assembler::ObjectState *objectState)
{
    lyric_runtime::TypeComparison comparison;
    TU_ASSIGN_OR_RETURN (comparison, compare_assignable(tp.typeDef, arg, objectState));

    switch (tp.bound) {
        case lyric_object::BoundType::Extends:
            if (comparison == lyric_runtime::TypeComparison::EQUAL
              || comparison == lyric_runtime::TypeComparison::EXTENDS)
                return {};
            break;
        case lyric_object::BoundType::Super:
            if (comparison == lyric_runtime::TypeComparison::EQUAL
              || comparison == lyric_runtime::TypeComparison::SUPER)
                return {};
            break;
        default:
            break;
    }

    return TypingStatus::forCondition(TypingCondition::kIncompatibleType,
        "argument type {} is not substitutable for constraint {}",
        arg.toString(), tp.typeDef.toString());
}
