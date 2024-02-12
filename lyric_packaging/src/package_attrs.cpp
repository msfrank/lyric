
#include <lyric_packaging/package_attrs.h>
#include <lyric_schema/packaging_schema.h>

const tempo_utils::Int64Attr lyric_packaging::kLyricPackagingCreateTime(
    &lyric_schema::kLyricPackagingCreateTimeProperty);

const tempo_utils::Int64Attr lyric_packaging::kLyricPackagingExpiryTime(
    &lyric_schema::kLyricPackagingExpiryTimeProperty);

const tempo_utils::StringAttr lyric_packaging::kLyricPackagingContentType(
    &lyric_schema::kLyricPackagingContentTypeProperty);

const lyric_common::AssemblyLocationAttr lyric_packaging::kLyricPackagingMainLocation(
    &lyric_schema::kLyricPackagingMainLocationProperty);