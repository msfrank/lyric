#ifndef LYRIC_PACKAGING_PACKAGE_LOADER_H
#define LYRIC_PACKAGING_PACKAGE_LOADER_H

#include <filesystem>

#include <absl/container/flat_hash_map.h>

#include <lyric_runtime/abstract_loader.h>

namespace lyric_packaging {

    class PackageLoader : public lyric_runtime::AbstractLoader {

    public:
        PackageLoader();
        PackageLoader(const std::vector<std::filesystem::path> &packagesPathList);
        PackageLoader(
            const std::vector<std::filesystem::path> &packagesPathList,
            const absl::flat_hash_map<std::string, std::string> &packageMap);
        PackageLoader(const PackageLoader &other);

        tempo_utils::Result<bool> hasModule(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<lyric_common::ModuleLocation>> resolveModule(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) override;
        tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;

        std::vector<std::filesystem::path> getPackagesPathList() const;
        absl::flat_hash_map<std::string, std::string> getPackageMap() const;

    private:
        std::vector<std::filesystem::path> m_packagesPathList;
        absl::flat_hash_map<std::string, std::string> m_packageMap;

        tempo_utils::Result<std::filesystem::path> findModule(
            const lyric_common::ModuleLocation &location) const;
        tempo_utils::Result<std::filesystem::path> packageLocationToFilePath(
            const std::filesystem::path &directoryPath,
            const lyric_common::ModuleLocation &location) const;
        tempo_utils::Result<std::filesystem::path> moduleLocationToFilePath(
            const std::filesystem::path &directoryPath,
            const lyric_common::ModuleLocation &location) const;
    };

}

#endif // LYRIC_PACKAGING_PACKAGE_LOADER_H
