#ifndef LYRIC_RUNTIME_LIBRARY_PLUGIN_H
#define LYRIC_RUNTIME_LIBRARY_PLUGIN_H

#include <lyric_runtime/abstract_plugin.h>
#include <tempo_utils/library_loader.h>

namespace lyric_runtime {

    class LibraryPlugin : public AbstractPlugin {

    public:
        LibraryPlugin(std::shared_ptr<tempo_utils::LibraryLoader> loader, const NativeInterface *iface);

        NativeFunc getTrap(tu_uint32 index) const override;
        tu_uint32 numTraps() const override;

    private:
        std::shared_ptr<tempo_utils::LibraryLoader> m_loader;
        const NativeInterface *m_iface;
    };
}

#endif // LYRIC_RUNTIME_LIBRARY_PLUGIN_H
