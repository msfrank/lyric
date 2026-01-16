#ifndef LYRIC_BUILD_LOCAL_FILESYSTEM_H
#define LYRIC_BUILD_LOCAL_FILESYSTEM_H

#include "abstract_filesystem.h"

namespace lyric_build {

    class LocalFilesystem: public AbstractFilesystem {
    public:
        static tempo_utils::Result<std::shared_ptr<LocalFilesystem>> create(
            const std::filesystem::path &baseDirectory,
            bool allowSymlinksOutsideBase = false);

        bool supportsScheme(std::string_view scheme);
        bool supportsScheme(const tempo_utils::Url &url);

        bool containsResource(const tempo_utils::UrlPath &path) override;
        tempo_utils::Result<Option<Resource>> fetchResource(const tempo_utils::UrlPath &path) override;
        tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> loadResource(
            std::string_view resourceId) override;

        tempo_utils::Result<ResourceList> listResources(
            const tempo_utils::UrlPath &root,
            ResourceMatcherFunc matcherFunc,
            const std::string &token) override;
        tempo_utils::Result<ResourceList> listResourcesRecursively(
            const tempo_utils::UrlPath &root,
            ResourceMatcherFunc matcherFunc,
            const std::string &token) override;

    private:
        std::filesystem::path m_baseDirectory;
        bool m_allowSymlinksOutsideBase;

        explicit LocalFilesystem(const std::filesystem::path &baseDirectory, bool allowSymlinksOutsideBase);
    };
}

#endif // LYRIC_BUILD_LOCAL_FILESYSTEM_H
