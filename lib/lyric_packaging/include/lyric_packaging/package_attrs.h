#ifndef LYRIC_PACKAGING_PACKAGE_ATTRS_H
#define LYRIC_PACKAGING_PACKAGE_ATTRS_H

#include <lyric_common/common_serde.h>
#include <lyric_schema/packaging_schema.h>
#include <tempo_utils/attr.h>

namespace lyric_packaging {

    extern const tempo_utils::Int64Attr kLyricPackagingCreateTime;
    extern const tempo_utils::Int64Attr kLyricPackagingExpiryTime;
    extern const tempo_utils::StringAttr kLyricPackagingContentType;
    extern const lyric_common::ModuleLocationAttr kLyricPackagingMainLocation;
}

#endif // LYRIC_PACKAGING_PACKAGE_ATTRS_H