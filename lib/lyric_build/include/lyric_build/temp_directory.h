#ifndef LYRIC_BUILD_TEMP_DIRECTORY_H
#define LYRIC_BUILD_TEMP_DIRECTORY_H
#include <filesystem>

#include "build_types.h"

namespace lyric_build {

    class TempDirectory {
    public:
        TempDirectory(
            const std::filesystem::path &tempRoot,
            const BuildGeneration &buildGen,
            const std::string &taskHash);

        std::filesystem::path getRoot() const;

        tempo_utils::Status initialize();

        tempo_utils::Result<std::filesystem::path> makeDirectory(const tempo_utils::UrlPath &path);
        tempo_utils::Result<std::filesystem::path> putContent(
            const tempo_utils::UrlPath &path,
            std::shared_ptr<const tempo_utils::ImmutableBytes> content);
        tempo_utils::Result<std::shared_ptr<const tempo_utils::ImmutableBytes>> getContent(tempo_utils::UrlPath &path);

        tempo_utils::Status cleanup();

    private:
        std::filesystem::path m_tempRoot;
        std::string m_genAndHash;
        std::filesystem::path m_baseDirectory;
    };
}

#endif // LYRIC_BUILD_TEMP_DIRECTORY_H
