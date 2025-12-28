#ifndef LYRIC_BOOTSTRAP_BOOTSTRAP_LOADER_H
#define LYRIC_BOOTSTRAP_BOOTSTRAP_LOADER_H

#include <filesystem>

#include <lyric_runtime/abstract_loader.h>

#include "bootstrap_plugin.h"

namespace lyric_bootstrap {

    class BootstrapLoader : public lyric_runtime::AbstractLoader {
    public:
        BootstrapLoader();

        tempo_utils::Result<bool> hasModule(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) override;
        tempo_utils::Result<Option<std::shared_ptr<const lyric_runtime::AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;

    private:
        absl::flat_hash_map<
            tempo_utils::UrlPath,
            std::pair<lyric_object::LyricObject,std::shared_ptr<BootstrapPlugin>>
        > m_modules;

        tempo_utils::Status loadModules();
        std::pair<lyric_object::LyricObject,std::shared_ptr<BootstrapPlugin>> findModule(
            const lyric_common::ModuleLocation &location) const;
    };
}

#endif // LYRIC_BOOTSTRAP_BOOTSTRAP_LOADER_H
