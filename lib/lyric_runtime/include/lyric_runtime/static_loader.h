#ifndef LYRIC_RUNTIME_STATIC_LOADER_H
#define LYRIC_RUNTIME_STATIC_LOADER_H

#include "abstract_loader.h"

namespace lyric_runtime {

    class StaticLoader : public AbstractLoader {
    public:
        StaticLoader() = default;

        tempo_utils::Result<bool> hasModule(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<lyric_common::ModuleLocation>> resolveModule(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) override;
        tempo_utils::Result<Option<std::shared_ptr<const AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;

        tempo_utils::Status insertModule(
            const lyric_common::ModuleLocation &location,
            const lyric_object::LyricObject &object);

    private:
        absl::flat_hash_map<lyric_common::ModuleLocation,lyric_object::LyricObject> m_objects;
    };
}

#endif // LYRIC_RUNTIME_STATIC_LOADER_H
