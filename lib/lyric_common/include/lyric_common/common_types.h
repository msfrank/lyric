#ifndef LYRIC_COMMON_COMMON_TYPES_H
#define LYRIC_COMMON_COMMON_TYPES_H

#define LYRIC_COMMON_SOURCE_FILE_SUFFIX                 "ly"
#define LYRIC_COMMON_SOURCE_FILE_DOT_SUFFIX             ".ly"
#define LYRIC_COMMON_INTERMEZZO_FILE_SUFFIX             "lyi"
#define LYRIC_COMMON_INTERMEZZO_FILE_DOT_SUFFIX         ".lyi"
#define LYRIC_COMMON_INTERMEZZO_CONTENT_TYPE            "application/x.zuri.intermezzo"
#define LYRIC_COMMON_ASSEMBLY_FILE_SUFFIX               "lyo"
#define LYRIC_COMMON_ASSEMBLY_FILE_DOT_SUFFIX           ".lyo"
#define LYRIC_COMMON_ASSEMBLY_CONTENT_TYPE              "application/x.zuri.assembly"
#define LYRIC_COMMON_PACKAGE_FILE_SUFFIX                "zpk"
#define LYRIC_COMMON_PACKAGE_FILE_DOT_SUFFIX            ".zpk"
#define LYRIC_COMMON_PACKAGE_CONTENT_TYPE               "application/x.zuri.package"

namespace lyric_common {

    constexpr const char *kSourceFileSuffix = LYRIC_COMMON_SOURCE_FILE_SUFFIX;
    constexpr const char *kSourceFileDotSuffix = LYRIC_COMMON_SOURCE_FILE_DOT_SUFFIX;

    constexpr const char *kIntermezzoFileSuffix = LYRIC_COMMON_INTERMEZZO_FILE_SUFFIX;
    constexpr const char *kIntermezzoFileDotSuffix = LYRIC_COMMON_INTERMEZZO_FILE_DOT_SUFFIX;
    constexpr const char *kIntermezzoContentType = LYRIC_COMMON_INTERMEZZO_CONTENT_TYPE;

    constexpr const char *kAssemblyFileSuffix = LYRIC_COMMON_ASSEMBLY_FILE_SUFFIX;
    constexpr const char *kAssemblyFileDotSuffix = LYRIC_COMMON_ASSEMBLY_FILE_DOT_SUFFIX;
    constexpr const char *kAssemblyContentType = LYRIC_COMMON_ASSEMBLY_CONTENT_TYPE;

    constexpr const char *kPackageFileSuffix = LYRIC_COMMON_PACKAGE_FILE_SUFFIX;
    constexpr const char *kPackageFileDotSuffix = LYRIC_COMMON_PACKAGE_FILE_DOT_SUFFIX;
    constexpr const char *kPackageContentType = LYRIC_COMMON_PACKAGE_CONTENT_TYPE;

}

#endif // LYRIC_COMMON_COMMON_TYPES_H