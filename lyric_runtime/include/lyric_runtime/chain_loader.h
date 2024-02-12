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

        tempo_utils::Result<bool> hasAssembly(
            const lyric_common::AssemblyLocation &location) const override;
        tempo_utils::Result<Option<lyric_common::AssemblyLocation>> resolveAssembly(
            const lyric_common::AssemblyLocation &location) const override;
        tempo_utils::Result<Option<lyric_object::LyricObject>> loadAssembly(
            const lyric_common::AssemblyLocation &location) override;
        tempo_utils::Result<Option<std::shared_ptr<const AbstractPlugin>>> loadPlugin(
            const lyric_common::AssemblyLocation &location,
            const lyric_object::PluginSpecifier &specifier) override;

    private:
        std::vector<std::shared_ptr<AbstractLoader>> m_chain;
    };
}

#endif // LYRIC_RUNTIME_CHAIN_LOADER_H
