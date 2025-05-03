#ifndef LYRIC_RUNTIME_CHAIN_LOADER_H
#define LYRIC_RUNTIME_CHAIN_LOADER_H

#include <vector>

#include "abstract_loader.h"

namespace lyric_runtime {

    class ChainLoader : public AbstractLoader {

    public:
        ChainLoader();
        ChainLoader(const std::vector<std::shared_ptr<AbstractLoader>> &loaderChain);
        ChainLoader(const ChainLoader &other);

        tempo_utils::Result<bool> hasModule(
            const lyric_common::ModuleLocation &location) const override;
        tempo_utils::Result<Option<lyric_object::LyricObject>> loadModule(
            const lyric_common::ModuleLocation &location) override;
        tempo_utils::Result<Option<std::shared_ptr<const AbstractPlugin>>> loadPlugin(
            const lyric_common::ModuleLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;

        std::shared_ptr<AbstractLoader> getLoader(int index) const;
        int numLoaders() const;

    private:
        std::vector<std::shared_ptr<AbstractLoader>> m_chain;
    };
}

#endif // LYRIC_RUNTIME_CHAIN_LOADER_H
