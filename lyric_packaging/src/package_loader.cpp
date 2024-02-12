
#include <filesystem>

#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>

#include <lyric_common/common_types.h>
#include <lyric_packaging/package_attrs.h>
#include <lyric_packaging/package_loader.h>
#include <lyric_packaging/package_reader.h>
#include <lyric_packaging/package_specifier.h>
#include <lyric_packaging/package_types.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/library_loader.h>
#include "lyric_runtime/library_plugin.h"

lyric_packaging::PackageLoader::PackageLoader()
{
}

lyric_packaging::PackageLoader::PackageLoader(const std::vector<std::filesystem::path> &packagesPathList)
    : m_packagesPathList(packagesPathList)
{
}

lyric_packaging::PackageLoader::PackageLoader(
    const std::vector<std::filesystem::path> &packagesPathList,
    const absl::flat_hash_map<std::string,std::string> &packageMap)
    : m_packagesPathList(packagesPathList), m_packageMap(packageMap)
{
}

lyric_packaging::PackageLoader::PackageLoader(const PackageLoader &other)
    : m_packagesPathList(other.m_packagesPathList),
      m_packageMap(other.m_packageMap)
{
}

tempo_utils::Result<std::filesystem::path>
lyric_packaging::PackageLoader::packageLocationToFilePath(
    const std::filesystem::path &packagesPath,
    const lyric_common::AssemblyLocation &location) const
{
    // build the package specifier
    auto authority = location.getAuthority();
    auto packageSpecifier = PackageSpecifier::fromAuthority(location.getAuthority());
    if (!packageSpecifier.isValid())
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant);

    // build the module path
    auto packagePath = packageSpecifier.toFilesystemPath(packagesPath);
    auto packagePathString = packagePath.generic_string();
    auto packagesPathString = packagesPath.generic_string();

    // validate that the absolute path does not escape the package path
    if (!absl::StartsWith(packagePathString, packagesPathString))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant);

    // special case: if packagesPathString does not end with a '/', then
    // modulePathString must start with a '/'
    if (packagesPathString.back() != '/' && packagePathString.front() != '/')
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant);

    return packagePath;
}

tempo_utils::Result<std::filesystem::path>
lyric_packaging::PackageLoader::assemblyLocationToFilePath(
    const std::filesystem::path &packagesPath,
    const lyric_common::AssemblyLocation &location) const
{
    // build the package specifier
    auto authority = location.getAuthority();
    auto packageSpecifier = PackageSpecifier::fromAuthority(authority);
    if (!packageSpecifier.isValid())
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant);

    // build the location path
    std::filesystem::path locationPath("", std::filesystem::path::auto_format);
    auto parts = location.getPath();
    for (int i = 0; i < parts.numParts(); i++) {
        locationPath.append(parts.partView(i));
    }
    locationPath.replace_extension(lyric_common::kAssemblyFileSuffix);

    // build the module path
    auto modulePath = packagesPath / packageSpecifier.toString() / locationPath;

    auto modulePathString = modulePath.string();
    auto packagesPathString = packagesPath.string();

    // validate that the absolute path does not escape the package path
    if (!absl::StartsWith(modulePathString, packagesPathString))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant);

    // special case: if packagesPathString does not end with a '/', then
    // modulePathString must start with a '/'
    if (packagesPathString.back() != '/' && modulePathString.front() != '/')
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant);

    return modulePath;
}

tempo_utils::Result<std::filesystem::path>
lyric_packaging::PackageLoader::findAssembly(const lyric_common::AssemblyLocation &location) const
{
    if (location.getScheme() == "file") {
        // a file: location cannot have an authority
        // FIXME: the authority check fails due to incorrect host detection
        //if (!location.getAuthority().isEmpty())
        //    return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
        //        "invalid location for file: scheme");
        auto path = location.getPath().toFilesystemPath(std::filesystem::current_path().root_path());
        if (path.extension() == lyric_common::kPackageFileDotSuffix) {
            if (exists(path) && is_regular_file(path))
                return path;
        }
        // if file does not exist or does not end in the package suffix, then delegate to the next loader
        return std::filesystem::path{};
    }

    if (location.getScheme() == "dev.zuri.pkg") {

        // a dev.zuri.pkg: location must have a valid authority
        auto authority = location.getAuthority();
        if (!authority.hasHost())
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant);
        if (!authority.hasUsername())
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant);

        for (const auto &packagesPath: m_packagesPathList) {

            if (!packagesPath.is_absolute() || !is_directory(packagesPath))
                continue;

            auto toFilePathResult = packageLocationToFilePath(packagesPath, location);
            if (toFilePathResult.isStatus())
                return toFilePathResult.getStatus();
            auto absolutePath = toFilePathResult.getResult();

            if (!absolutePath.empty()) {
                if (exists(absolutePath) && is_regular_file(absolutePath))
                    return absolutePath;
            }

            toFilePathResult = assemblyLocationToFilePath(packagesPath, location);
            if (toFilePathResult.isStatus())
                return toFilePathResult.getStatus();
            absolutePath = toFilePathResult.getResult();

            if (!absolutePath.empty()) {
                if (exists(absolutePath) && is_regular_file(absolutePath))
                    return absolutePath;
            }
        }

        // if package does not exist in any of the package paths, then delegate to the next loader
        return std::filesystem::path{};
    }

    return std::filesystem::path{};
}

tempo_utils::Result<bool>
lyric_packaging::PackageLoader::hasAssembly(const lyric_common::AssemblyLocation &location) const
{
    auto findAssemblyResult = findAssembly(location);
    if (findAssemblyResult.isStatus())
        return findAssemblyResult.getStatus();
    return !findAssemblyResult.getResult().empty();
}

tempo_utils::Result<Option<lyric_common::AssemblyLocation>>
lyric_packaging::PackageLoader::resolveAssembly(const lyric_common::AssemblyLocation &location) const
{
    auto authority = location.getAuthority();

    // if scheme is file or dev.zuri.pkg then location is already fully qualified
    if (location.getScheme() == "file")
        return Option(location);
    if (location.getScheme() == "dev.zuri.pkg")
        return Option(location);

    // if location has any other scheme then we can't qualify the location
    if (location.hasScheme())
        return Option<lyric_common::AssemblyLocation>();

    // if there is no authority then we can't qualify the location
    if (!authority.isValid())
        return Option<lyric_common::AssemblyLocation>();

    auto authorityString = authority.toString();
    if (!m_packageMap.contains(authorityString))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "missing package specifier for '{}'", authorityString);
    auto origin = absl::StrCat("dev.zuri.pkg://", m_packageMap.at(authorityString));
    auto path = location.getPath().toString();
    return Option(lyric_common::AssemblyLocation(origin, path));
}

static lyric_packaging::EntryPath
assembly_location_path_to_entry_path(const tempo_utils::UrlPath &locationPath)
{
    auto locationInit = locationPath.getInit();
    auto assemblyPath = lyric_packaging::EntryPath::fromString("lib");
    for (int i = 0; i < locationInit.numParts(); i++) {
        assemblyPath = assemblyPath.traverse(locationInit.partView(i));
    }

    std::filesystem::path filename(locationPath.lastView());
    filename.replace_extension(lyric_common::kAssemblyFileSuffix);
    return assemblyPath.traverse(filename.string());
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
lyric_packaging::PackageLoader::loadAssembly(const lyric_common::AssemblyLocation &location)
{
    auto findAssemblyResult = findAssembly(location);
    if (findAssemblyResult.isStatus())
        return findAssemblyResult.getStatus();
    auto absolutePath = findAssemblyResult.getResult();

    if (absolutePath.empty())
        return Option<lyric_object::LyricObject>();

    std::shared_ptr<const tempo_utils::ImmutableBytes> bytes;

    if (location.getScheme() == "file") {
        // load the assembly from the package
        auto createReaderResult = PackageReader::open(absolutePath);
        if (createReaderResult.isStatus())
            return createReaderResult.getStatus();
        auto reader = createReaderResult.getResult();
        TU_ASSERT (reader->isValid());

        auto manifest = reader->getManifest().getManifest();
        auto root = manifest.getRoot();
        lyric_common::AssemblyLocation packageLocation;
        auto status = root.parseAttr(kLyricPackagingMainLocation, packageLocation);
        if (status.notOk())
            return status;
        auto locationPath = packageLocation.getPath();

        // convert the location path into an entry path
        auto assemblyPath = assembly_location_path_to_entry_path(locationPath);

        auto slice = reader->getFileContents(assemblyPath);
        if (slice.isEmpty())
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                "package {} missing file contents for {}",
                absolutePath.string(), assemblyPath.toString());
        bytes = slice.toImmutableBytes();
    }
    else if (absolutePath.extension() == lyric_common::kPackageFileDotSuffix) {
        // load the assembly from the package
        auto createReaderResult = PackageReader::open(absolutePath);
        if (createReaderResult.isStatus())
            return createReaderResult.getStatus();
        auto reader = createReaderResult.getResult();
        TU_ASSERT (reader->isValid());

        // if the location contains an assembly path, then use it; otherwise look for
        // the main location in the package manifest
        auto locationPath = location.getPath();
        if (locationPath.isEmpty()) {
            auto manifest = reader->getManifest().getManifest();
            auto root = manifest.getRoot();
            lyric_common::AssemblyLocation packageLocation;
            auto status = root.parseAttr(kLyricPackagingMainLocation, packageLocation);
            if (status.notOk())
                return status;
            locationPath = packageLocation.getPath();
        }
        if (locationPath.isEmpty())
            return Option<lyric_object::LyricObject>();

        // convert the location path into an entry path
        auto assemblyPath = assembly_location_path_to_entry_path(locationPath);

        auto slice = reader->getFileContents(assemblyPath);
        if (slice.isEmpty())
            return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
                "package {} missing file contents for {}",
                absolutePath.string(), assemblyPath.toString());
        bytes = slice.toImmutableBytes();
    }
    else if (absolutePath.extension() == std::string_view(lyric_common::kAssemblyFileDotSuffix)) {
        // load the assembly from the filesystem
        tempo_utils::FileReader reader(absolutePath.string());
        if (!reader.isValid())
            return reader.getStatus();
        bytes = reader.getBytes();
    }
    else {
        // encountered an unknown or invalid file type
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "unknown or invalid file {}", absolutePath.string());
    }

    if (bytes == nullptr)
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "empty or invalid file {}", absolutePath.string());

    // verify that file contents is a valid assembly
    if (!lyric_object::LyricObject::verify(std::span<const tu_uint8>(bytes->getData(), bytes->getSize())))
        return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
            "verification failed for entry {}", absolutePath.string());

    TU_LOG_V << "loaded " << location << " from " << absolutePath;
    return Option(lyric_object::LyricObject(bytes));
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
lyric_packaging::PackageLoader::loadPlugin(
    const lyric_common::AssemblyLocation &location,
    const lyric_object::PluginSpecifier &specifier)
{
    auto findAssemblyResult = findAssembly(location);
    if (findAssemblyResult.isStatus())
        return findAssemblyResult.getStatus();
    auto absolutePath = findAssemblyResult.getResult();

    if (absolutePath.empty())
        return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>();

    std::shared_ptr<const std::string> bytes;

    if (location.getScheme() == "file") {
        //return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
        //    "failed to load plugin {}: scheme is unimplemented", location.toString());
        return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>();
    }
    else if (absolutePath.extension() == lyric_common::kPackageFileDotSuffix) {
        //return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
        //    "failed to load plugin {}: scheme is unimplemented", location.toString());
        return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>();
    }
    else if (absolutePath.extension() == std::string_view(lyric_common::kAssemblyFileDotSuffix)) {

        auto platformId = absl::StrCat(ASSEMBLY_PLUGIN_SYSTEM_NAME, "-", ASSEMBLY_PLUGIN_ARCHITECTURE);
        auto pluginName = absl::StrCat(absolutePath.stem().string(), ".", platformId, ASSEMBLY_PLUGIN_SUFFIX);
        absolutePath.replace_filename(pluginName);

        // attempt to load the plugin
        auto loader = std::make_shared<tempo_utils::LibraryLoader>(absolutePath, "native_init");
        if (!loader->isValid())
            return loader->getStatus();

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

    // encountered an unknown or invalid file type
    return PackageStatus::forCondition(PackageCondition::kPackageInvariant,
        "unknown or invalid file {}", absolutePath.string());
}

std::vector<std::filesystem::path>
lyric_packaging::PackageLoader::getPackagesPathList() const
{
    return m_packagesPathList;
}

absl::flat_hash_map<std::string, std::string>
lyric_packaging::PackageLoader::getPackageMap() const
{
    return m_packageMap;
}