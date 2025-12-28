#ifndef LYRIC_BOOTSTRAP_BOOTSTRAP_PLUGIN_H
#define LYRIC_BOOTSTRAP_BOOTSTRAP_PLUGIN_H

#include <lyric_runtime/abstract_plugin.h>

namespace lyric_bootstrap {

    class BootstrapPlugin : public lyric_runtime::AbstractPlugin {

    public:
        explicit BootstrapPlugin(const lyric_runtime::NativeInterface *iface);

        bool load(lyric_runtime::BytecodeSegment *segment) const override;
        void unload(lyric_runtime::BytecodeSegment *segment) const override;
        const lyric_runtime::NativeTrap *getTrap(tu_uint32 index) const override;
        tu_uint32 numTraps() const override;

    private:
        const lyric_runtime::NativeInterface *m_iface;
    };
}

#endif // LYRIC_BOOTSTRAP_BOOTSTRAP_PLUGIN_H