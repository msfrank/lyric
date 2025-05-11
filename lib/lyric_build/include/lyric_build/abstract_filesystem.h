#ifndef LYRIC_BUILD_ABSTRACT_FILESYSTEM_H
#define LYRIC_BUILD_ABSTRACT_FILESYSTEM_H

#include <tempo_utils/immutable_bytes.h>
#include <tempo_utils/option_template.h>
#include <tempo_utils/result.h>
#include <tempo_utils/url.h>

namespace lyric_build {

    struct Resource {
        std::string id;                     /**< opaque id used to fetch the resource. */
        tu_uint64 lastModifiedMillis;       /**< last modified time, in millis since the epoch. */
        std::string entityTag;              /**< resource content hash. */
        std::filesystem::path localPath;    /**< path to the content on the local filesystem, if applicable. */
    };

    struct ResourceList {
        std::vector<tempo_utils::UrlPath> resources;
        std::string token;
    };

    /**
     * Defines a function returning true if the path argument matches a resource.
     */
    typedef bool (*ResourceMatcherFunc)(const tempo_utils::UrlPath &);

    class AbstractFilesystem {
    public:
        virtual ~AbstractFilesystem() = default;

        /**
         * Returns true if the filesystem contains a resource at the given path, otherwise false.
         *
         * @param path The path to the resource.
         * @return true if the resource exists, otherwise false.
         */
        virtual bool containsResource(const tempo_utils::UrlPath &path) = 0;

        /**
         * Fetch metadata describing the resource at the given path. If the resource does not exist then
         * an empty `Option` is returned, otherwise the `Result` contains a `Status` describing the failure.
         *
         * @param path The path to the resource.
         * @return A `Result` containing an `Option` with the resource metadata if it exists.
         */
        virtual tempo_utils::Result<Option<Resource>> fetchResource(const tempo_utils::UrlPath &path) = 0;

        /**
         * Load the content for the resource at the given path as an immutable sequence of bytes.
         *
         * @param resourceId The resource id (from the `id` field of a `Resource`).
         * @return A `Result` containing the content.
         */
        virtual tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> loadResource(
            std::string_view resourceId) = 0;

        /**
         * Fetch a list of resources which are stored at the given root path and which match the specified
         * `matchFunc`. If the `token` is not empty then the list process picks up where it left off based on
         * the token.
         *
         * @param root
         * @param matcherFunc
         * @param token
         * @return
         */
        virtual tempo_utils::Result<ResourceList> listResources(
            const tempo_utils::UrlPath &root,
            ResourceMatcherFunc matcherFunc,
            const std::string &token) = 0;

        /**
         * Fetch a list of resources which are stored at the given root path and which match the specified
         * `matchFunc`. This method recursively descends into child directories of the root path. If the
         * `token` is not empty then the list process picks up where it left off based on the token.
         *
         * @param root
         * @param matcherFunc
         * @param token
         * @return
         */
        virtual tempo_utils::Result<ResourceList> listResourcesRecursively(
            const tempo_utils::UrlPath &root,
            ResourceMatcherFunc matcherFunc,
            const std::string &token) = 0;
    };
}

#endif // LYRIC_BUILD_ABSTRACT_FILESYSTEM_H
