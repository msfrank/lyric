#ifndef LYRIC_COMMON_COMMON_TYPES_H
#define LYRIC_COMMON_COMMON_TYPES_H

#define LYRIC_COMMON_SOURCE_FILE_SUFFIX                 "ly"
#define LYRIC_COMMON_SOURCE_FILE_DOT_SUFFIX             ".ly"
#define LYRIC_COMMON_SOURCE_CONTENT_TYPE                "application/vnd.lyric.source"

#define LYRIC_COMMON_INTERMEZZO_FILE_SUFFIX             "lyi"
#define LYRIC_COMMON_INTERMEZZO_FILE_DOT_SUFFIX         ".lyi"
#define LYRIC_COMMON_INTERMEZZO_CONTENT_TYPE            "application/vnd.lyric.intermezzo"

#define LYRIC_COMMON_OBJECT_FILE_SUFFIX                 "lyo"
#define LYRIC_COMMON_OBJECT_FILE_DOT_SUFFIX             ".lyo"
#define LYRIC_COMMON_OBJECT_CONTENT_TYPE                "application/vnd.lyric.object"

#define LYRIC_COMMON_PLUGIN_CONTENT_TYPE                "application/vnd.lyric.plugin"

namespace lyric_common {

    constexpr const char *kSourceFileSuffix = LYRIC_COMMON_SOURCE_FILE_SUFFIX;
    constexpr const char *kSourceFileDotSuffix = LYRIC_COMMON_SOURCE_FILE_DOT_SUFFIX;
    constexpr const char *kSourceContentType = LYRIC_COMMON_SOURCE_CONTENT_TYPE;

    constexpr const char *kIntermezzoFileSuffix = LYRIC_COMMON_INTERMEZZO_FILE_SUFFIX;
    constexpr const char *kIntermezzoFileDotSuffix = LYRIC_COMMON_INTERMEZZO_FILE_DOT_SUFFIX;
    constexpr const char *kIntermezzoContentType = LYRIC_COMMON_INTERMEZZO_CONTENT_TYPE;

    constexpr const char *kObjectFileSuffix = LYRIC_COMMON_OBJECT_FILE_SUFFIX;
    constexpr const char *kObjectFileDotSuffix = LYRIC_COMMON_OBJECT_FILE_DOT_SUFFIX;
    constexpr const char *kObjectContentType = LYRIC_COMMON_OBJECT_CONTENT_TYPE;

    constexpr const char *kPluginContentType = LYRIC_COMMON_PLUGIN_CONTENT_TYPE;
}

#endif // LYRIC_COMMON_COMMON_TYPES_H