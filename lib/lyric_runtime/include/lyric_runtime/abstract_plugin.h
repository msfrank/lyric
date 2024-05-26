#ifndef LYRIC_RUNTIME_ABSTRACT_PLUGIN_H
#define LYRIC_RUNTIME_ABSTRACT_PLUGIN_H

#include "native_interface.h"

namespace lyric_runtime {

    class AbstractPlugin {

    public:
        virtual ~AbstractPlugin() = default;

        virtual bool load(BytecodeSegment *segment) const = 0;

        virtual void unload(BytecodeSegment *segment) const = 0;

        virtual NativeFunc getTrap(tu_uint32 index) const = 0;

        virtual tu_uint32 numTraps() const = 0;
    };
}
#endif // LYRIC_RUNTIME_ABSTRACT_PLUGIN_H
