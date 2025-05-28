#ifndef LYRIC_BUILD_ARTIFACT_CACHE_H
#define LYRIC_BUILD_ARTIFACT_CACHE_H
#include "abstract_cache.h"

namespace lyric_build {

    class ArtifactCache {
    public:
        explicit ArtifactCache(std::shared_ptr<AbstractCache> cache);

        tempo_utils::Status createFileEntry();

    private:
        std::shared_ptr<AbstractCache> m_cache;
    };
}

#endif // LYRIC_BUILD_ARTIFACT_CACHE_H
