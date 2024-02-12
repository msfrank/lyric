#ifndef LYRIC_BUILD_METADATA_MATCHER_H
#define LYRIC_BUILD_METADATA_MATCHER_H

#include <lyric_build/lyric_metadata.h>

namespace lyric_build {

    bool metadata_matches_all_filters(const LyricMetadata &metadata, const LyricMetadata &filters);
};


#endif // LYRIC_BUILD_METADATA_MATCHER_H