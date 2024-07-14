#ifndef LYRIC_COMMON_COMMON_TYPES_H
#define LYRIC_COMMON_COMMON_TYPES_H

#define LYRIC_COMMON_SOURCE_FILE_SUFFIX                 "ly"
#define LYRIC_COMMON_SOURCE_FILE_DOT_SUFFIX             ".ly"
#define LYRIC_COMMON_INTERMEZZO_FILE_SUFFIX             "lyi"
#define LYRIC_COMMON_INTERMEZZO_FILE_DOT_SUFFIX         ".lyi"
#define LYRIC_COMMON_INTERMEZZO_CONTENT_TYPE            "application/x.zuri.intermezzo"
#define LYRIC_COMMON_OBJECT_FILE_SUFFIX                 "lyo"
#define LYRIC_COMMON_OBJECT_FILE_DOT_SUFFIX             ".lyo"
#define LYRIC_COMMON_OBJECT_CONTENT_TYPE                "application/x.zuri.object"
#define LYRIC_COMMON_PACKAGE_FILE_SUFFIX                "zpk"
#define LYRIC_COMMON_PACKAGE_FILE_DOT_SUFFIX            ".zpk"
#define LYRIC_COMMON_PACKAGE_CONTENT_TYPE               "application/x.zuri.package"

namespace lyric_common {

    constexpr const char *kSourceFileSuffix = LYRIC_COMMON_SOURCE_FILE_SUFFIX;
    constexpr const char *kSourceFileDotSuffix = LYRIC_COMMON_SOURCE_FILE_DOT_SUFFIX;

    constexpr const char *kIntermezzoFileSuffix = LYRIC_COMMON_INTERMEZZO_FILE_SUFFIX;
    constexpr const char *kIntermezzoFileDotSuffix = LYRIC_COMMON_INTERMEZZO_FILE_DOT_SUFFIX;
    constexpr const char *kIntermezzoContentType = LYRIC_COMMON_INTERMEZZO_CONTENT_TYPE;

    constexpr const char *kObjectFileSuffix = LYRIC_COMMON_OBJECT_FILE_SUFFIX;
    constexpr const char *kObjectFileDotSuffix = LYRIC_COMMON_OBJECT_FILE_DOT_SUFFIX;
    constexpr const char *kObjectContentType = LYRIC_COMMON_OBJECT_CONTENT_TYPE;

    constexpr const char *kPackageFileSuffix = LYRIC_COMMON_PACKAGE_FILE_SUFFIX;
    constexpr const char *kPackageFileDotSuffix = LYRIC_COMMON_PACKAGE_FILE_DOT_SUFFIX;
    constexpr const char *kPackageContentType = LYRIC_COMMON_PACKAGE_CONTENT_TYPE;

}

#endif // LYRIC_COMMON_COMMON_TYPES_H