#ifndef LYRIC_RUNTIME_TRAP_INDEX_H
#define LYRIC_RUNTIME_TRAP_INDEX_H

#include <absl/container/flat_hash_map.h>

#include "abstract_plugin.h"

namespace lyric_runtime {

    class TrapIndex {
    public:
        TrapIndex(std::shared_ptr<const AbstractPlugin> plugin);

        tempo_utils::Status initialize();

        tu_uint32 lookupTrap(std::string_view name) const;

    private:
        std::shared_ptr<const AbstractPlugin> m_plugin;
        absl::flat_hash_map<std::string, tu_uint32> m_traps;
    };
}

#endif // LYRIC_RUNTIME_TRAP_INDEX_H
