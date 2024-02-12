#ifndef LYRIC_PACKAGING_DIRECTORY_LOADER_H
#define LYRIC_PACKAGING_DIRECTORY_LOADER_H

#include <filesystem>

#include <lyric_runtime/abstract_loader.h>

namespace lyric_packaging {

    class DirectoryLoader : public lyric_runtime::AbstractLoader {
    public:
        DirectoryLoader(const std::filesystem::path &directoryPath);

        tempo_utils::Result<bool> hasAssembly(
            const lyric_common::AssemblyLocation &location) const override;
        tempo_utils::Result<Option<lyric_common::AssemblyLocation>> resolveAssembly(
            const lyric_common::AssemblyLocation &location) const override;
        tempo_utils::Result<Option<lyric_object::LyricObject>> loadAssembly(
            const lyric_common::AssemblyLocation &location) override;
        tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
            const lyric_common::AssemblyLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;

    private:
        std::filesystem::path m_directoryPath;

        std::filesystem::path findAssembly(const lyric_common::AssemblyLocation &location) const;
        std::filesystem::path assemblyLocationToFilePath(
            const std::filesystem::path &packagesPath,
            const lyric_common::AssemblyLocation &location) const;
        bool fileInfoIsValid(const std::filesystem::path &path) const;
    };
}

#endif // LYRIC_PACKAGING_DIRECTORY_LOADER_H
