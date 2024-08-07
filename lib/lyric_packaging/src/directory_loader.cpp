
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>

#include <lyric_common/common_types.h>
#include <lyric_packaging/directory_loader.h>
#include <lyric_packaging/package_result.h>
#include <lyric_runtime/library_plugin.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/library_loader.h>
#include <tempo_utils/log_stream.h>

lyric_packaging::DirectoryLoader::DirectoryLoader(const std::filesystem::path &directoryPath)
    : m_directoryPath(directoryPath)
{
}

std::filesystem::path
lyric_packaging::DirectoryLoader::assemblyLocationToFilePath(
    const std::filesystem::path &directoryPath,
    const lyric_common::AssemblyLocation &location) const
{
    if (!directoryPath.is_absolute() || !is_directory(directoryPath))
        return {};
    if (!location.isValid())
        return {};
    if (!location.getScheme().empty() || location.getAuthority().isValid())
        return {};

    // build the location path
    auto modulePath = location.getPath().toFilesystemPath(directoryPath);
    modulePath.replace_extension(lyric_common::kObjectFileSuffix);

    auto modulePathString = modulePath.string();
    auto directoryPathString = directoryPath.string();

    // validate that the absolute path does not escape the package path
    if (!absl::StartsWith(modulePathString, directoryPathString))
        return {};

    // special case: if packagesPathString does not end with a '/', then
    // modulePathString must start with a '/'
    if (directoryPathString.back() != '/' && modulePathString.front() != '/')
        return {};

    return modulePath;
}

bool
lyric_packaging::DirectoryLoader::fileInfoIsValid(const std::filesystem::path &path) const
{
    if (!std::filesystem::exists(path))
        return false;
    if (!std::filesystem::is_regular_file(path))
        return false;

    return true;
}

std::filesystem::path
lyric_packaging::DirectoryLoader::findAssembly(const lyric_common::AssemblyLocation &location) const
{
    auto absolutePath = assemblyLocationToFilePath(m_directoryPath, location);
    if (!absolutePath.empty()) {
        if (fileInfoIsValid(absolutePath))
            return absolutePath;
    }
    return {};
}

tempo_utils::Result<bool>
lyric_packaging::DirectoryLoader::hasAssembly(const lyric_common::AssemblyLocation &location) const
{
    return !findAssembly(location).empty();
}

tempo_utils::Result<Option<lyric_common::AssemblyLocation>>
lyric_packaging::DirectoryLoader::resolveAssembly(const lyric_common::AssemblyLocation &location) const
{
    return Option<lyric_common::AssemblyLocation>(location);
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
lyric_packaging::DirectoryLoader::loadAssembly(const lyric_common::AssemblyLocation &location)
{
    auto absolutePath = findAssembly(location);
    if (absolutePath.empty())
        return Option<lyric_object::LyricObject>();

    tempo_utils::FileReader reader(absolutePath.string());
    if (!reader.isValid())
        return reader.getStatus();
    auto bytes = reader.getBytes();

    // verify that file contents is a valid assembly
    if (!lyric_object::LyricObject::verify(std::span<const tu_uint8>(bytes->getData(), bytes->getSize())))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant, "failed to verify assembly");

    // return platform-specific LyricAssembly
    TU_LOG_INFO << "loaded assembly at " << absolutePath;
    return Option(lyric_object::LyricObject(bytes));
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
lyric_packaging::DirectoryLoader::loadPlugin(
    const lyric_common::AssemblyLocation &location,
    const lyric_object::PluginSpecifier &specifier)
{
    auto absolutePath = findAssembly(location);
    if (absolutePath.empty())
        return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>();

    auto platformId = absl::StrCat(ASSEMBLY_PLUGIN_SYSTEM_NAME, "-", ASSEMBLY_PLUGIN_ARCHITECTURE);
    auto pluginName = absl::StrCat(absolutePath.stem().string(), ".", platformId, ASSEMBLY_PLUGIN_SUFFIX);
    absolutePath.replace_filename(pluginName);

    // attempt to load the plugin
    auto loader = std::make_shared<tempo_utils::LibraryLoader>(absolutePath, "native_init");
    if (!loader->isValid()) {
        auto status = loader->getStatus();
        // if plugin is not found then return empty option instead of status
        if (status.getStatusCode() == tempo_utils::StatusCode::kNotFound)
            return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>();
        return status;
    }

    // cast raw pointer to native_init function pointer
    auto native_init = (lyric_runtime::NativeInitFunc) loader->symbolPointer();
    if (native_init == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "failed to retrieve native_init symbol from plugin {}", absolutePath.string());

    // retrieve the plugin interface
    auto *iface = native_init();
    if (iface == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "failed to retrieve interface for plugin {}", absolutePath.string());

    TU_LOG_INFO << "loaded plugin " << absolutePath;
    auto plugin = std::make_shared<const lyric_runtime::LibraryPlugin>(loader, iface);
    return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>(plugin);
}
