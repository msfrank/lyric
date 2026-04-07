#ifndef LYRIC_ASSEMBLER_OBJECT_PLUGIN_H
#define LYRIC_ASSEMBLER_OBJECT_PLUGIN_H

#include <lyric_common/module_location.h>
#include <tempo_utils/result.h>

#include "lyric_object/lyric_object.h"

namespace lyric_assembler {

    class ObjectPlugin {
    public:
        ObjectPlugin(
            const lyric_common::ModuleLocation &location,
            bool isExactLinkage,
            lyric_object::HashType hash);

        lyric_common::ModuleLocation getLocation() const;
        bool isExactLinkage() const;
        lyric_object::HashType getHashType() const;

        tempo_utils::Result<tu_uint32> getOrInsertTrap(std::string_view name);
        absl::flat_hash_map<std::string,tu_uint32>::const_iterator trapsBegin() const;
        absl::flat_hash_map<std::string,tu_uint32>::const_iterator trapsEnd() const;
        tu_uint32 numTraps() const;

    private:
        lyric_common::ModuleLocation m_location;
        bool m_isExactLinkage;
        lyric_object::HashType m_hash;
        absl::flat_hash_map<std::string,tu_uint32> m_trapIndex;
    };
}

#endif // LYRIC_ASSEMBLER_OBJECT_PLUGIN_H
