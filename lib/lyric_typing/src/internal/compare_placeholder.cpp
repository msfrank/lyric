
#include <lyric_assembler/type_cache.h>
#include <lyric_typing/internal/compare_placeholder.h>
#include <lyric_typing/typing_result.h>

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::internal::compare_placeholder_to_placeholder(
    const lyric_common::TypeDef &toPlaceholder,
    const lyric_common::TypeDef &fromPlaceholder,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toPlaceholder.getType() == lyric_common::TypeDefType::Placeholder);
    TU_ASSERT (fromPlaceholder.getType() == lyric_common::TypeDefType::Placeholder);

    if (fromPlaceholder.getPlaceholderIndex() != toPlaceholder.getPlaceholderIndex())
        return lyric_runtime::TypeComparison::DISJOINT;

    return fromPlaceholder.getPlaceholderTemplateUrl() == toPlaceholder.getPlaceholderTemplateUrl()?
        lyric_runtime::TypeComparison::EQUAL : lyric_runtime::TypeComparison::DISJOINT;
}

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_typing::internal::compare_placeholder(
    const lyric_common::TypeDef &toPlaceholder,
    const lyric_common::TypeDef &fromType,
    lyric_assembler::ObjectState *state)
{
    TU_ASSERT (toPlaceholder.getType() == lyric_common::TypeDefType::Placeholder);
    switch (fromType.getType()) {
        case lyric_common::TypeDefType::Placeholder:
            return compare_placeholder_to_placeholder(toPlaceholder, fromType, state);
        default:
            return lyric_runtime::TypeComparison::DISJOINT;
    }
}
