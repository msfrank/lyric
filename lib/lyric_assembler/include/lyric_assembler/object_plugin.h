#ifndef LYRIC_ASSEMBLER_OBJECT_PLUGIN_H
#define LYRIC_ASSEMBLER_OBJECT_PLUGIN_H

#include <lyric_runtime/trap_index.h>

#include "lyric_common/module_location.h"

namespace lyric_assembler {

    class ObjectPlugin {
    public:
        ObjectPlugin(
            const lyric_common::ModuleLocation &pluginLocation,
            std::unique_ptr<lyric_runtime::TrapIndex> trapIndex);

        lyric_common::ModuleLocation getLocation() const;
        tu_uint32 lookupTrap(std::string_view name) const;

    private:
        lyric_common::ModuleLocation m_pluginLocation;
        std::unique_ptr<lyric_runtime::TrapIndex> m_trapIndex;
    };
}

#endif // LYRIC_ASSEMBLER_OBJECT_PLUGIN_H
