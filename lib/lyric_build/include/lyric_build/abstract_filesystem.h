#ifndef LYRIC_BUILD_ABSTRACT_FILESYSTEM_H
#define LYRIC_BUILD_ABSTRACT_FILESYSTEM_H

#include <tempo_utils/immutable_bytes.h>
#include <tempo_utils/option_template.h>
#include <tempo_utils/result.h>
#include <tempo_utils/url.h>

namespace lyric_build {

    struct Resource {
        std::string id;
        tu_uint64 lastModifiedMillis;
        std::string entityTag;
    };

    struct ResourceList {
        std::vector<tempo_utils::Url> resources;
        std::string token;
    };

    typedef bool (*ResourceMatcherFunc)(const std::filesystem::path &);

    class AbstractFilesystem {
    public:
        virtual ~AbstractFilesystem() = default;

        /**
         *
         * @param url
         * @return
         */
        virtual Option<bool> containsResource(const tempo_utils::Url &url) = 0;

        /**
         *
         * @param url
         * @return
         */
        virtual tempo_utils::Result<Option<Resource>> fetchResource(const tempo_utils::Url &url) = 0;

        /**
         *
         * @param resourceId
         * @return
         */
        virtual tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> loadResource(
            std::string_view resourceId) = 0;

        virtual tempo_utils::Result<ResourceList> listResources(
            const tempo_utils::Url &resourceRoot,
            ResourceMatcherFunc matcherFunc,
            const std::string &token) = 0;

        virtual tempo_utils::Result<ResourceList> listResourcesRecursively(
            const tempo_utils::Url &resourceRoot,
            ResourceMatcherFunc matcherFunc,
            const std::string &token) = 0;
    };
}

#endif // LYRIC_BUILD_ABSTRACT_FILESYSTEM_H
