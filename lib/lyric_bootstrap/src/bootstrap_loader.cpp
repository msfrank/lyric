
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>

#include <lyric_bootstrap/bootstrap_loader.h>
#include <lyric_bootstrap/bootstrap_plugin.h>
#include <lyric_bootstrap/bootstrap_result.h>
#include <lyric_common/common_types.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/library_loader.h>
#include <tempo_utils/log_stream.h>

#include <lyric_bootstrap/generated/prelude_object.h>

#include "native_prelude.h"

struct ModuleInfo {
    const char *name;
    const tu_uint8 *object_data;
    tu_uint32 object_size;
    const lyric_runtime::NativeInterface *iface;
};

static std::array<ModuleInfo,1> modules = {{
    { "/prelude", lyric_bootstrap::prelude::object::data, lyric_bootstrap::prelude::object::size, &kPreludeInterface},
}};

lyric_bootstrap::BootstrapLoader::BootstrapLoader()
{
    TU_RAISE_IF_NOT_OK (loadModules());
}

tempo_utils::Status
lyric_bootstrap::BootstrapLoader::loadModules()
{
    for (const auto &modinfo : modules) {
        auto path = tempo_utils::UrlPath::fromString(modinfo.name);
        std::span objectBytes(modinfo.object_data, modinfo.object_size);

        // verify that object contents is a valid object
        if (!lyric_object::LyricObject::verify(objectBytes))
            return BootstrapStatus::forCondition(BootstrapCondition::kBootstrapInvariant,
                "failed to verify bootstrap object {}", path.toString());

        lyric_object::LyricObject object(objectBytes);
        auto plugin = std::make_shared<BootstrapPlugin>(modinfo.iface);
        m_modules[path] = std::pair(object, plugin);

        TU_LOG_V << "loaded bootstrap module " << path;
    }
    return {};
}

std::pair<lyric_object::LyricObject,std::shared_ptr<lyric_bootstrap::BootstrapPlugin>>
lyric_bootstrap::BootstrapLoader::findModule(const lyric_common::ModuleLocation &location) const
{
    if (location.getScheme() != "dev.zuri.bootstrap")
        return {};
    auto entry = m_modules.find(location.getPath());
    if (entry != m_modules.cend())
        return entry->second;
    return {};
}

tempo_utils::Result<bool>
lyric_bootstrap::BootstrapLoader::hasModule(const lyric_common::ModuleLocation &location) const
{
    return findModule(location).first.isValid();
}

tempo_utils::Result<Option<lyric_object::LyricObject>>
lyric_bootstrap::BootstrapLoader::loadModule(const lyric_common::ModuleLocation &location)
{
    auto module = findModule(location);
    if (module.first.isValid())
        return Option(module.first);
    return Option<lyric_object::LyricObject>{};
}

tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>>
lyric_bootstrap::BootstrapLoader::loadPlugin(
    const lyric_common::ModuleLocation &location,
    const lyric_object::PluginSpecifier &specifier)
{
    auto module = findModule(location);
    if (module.second != nullptr)
        return Option(std::static_pointer_cast<const lyric_runtime::AbstractPlugin>(module.second));
    return Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>{};
}
