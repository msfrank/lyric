
#include <lyric_assembler/object_plugin.h>

#include "lyric_assembler/assembler_result.h"

lyric_assembler::ObjectPlugin::ObjectPlugin(
    const lyric_common::ModuleLocation &pluginLocation,
    bool isExactLinkage,
    lyric_object::HashType hash)
    : m_location(pluginLocation),
      m_isExactLinkage(isExactLinkage),
      m_hash(hash)
{
    TU_ASSERT (m_location.isValid());
    TU_ASSERT (m_location.isRelative());
}

lyric_common::ModuleLocation
lyric_assembler::ObjectPlugin::getLocation() const
{
    return m_location;
}

bool
lyric_assembler::ObjectPlugin::isExactLinkage() const
{
    return m_isExactLinkage;
}

lyric_object::HashType
lyric_assembler::ObjectPlugin::getHashType() const
{
    return m_hash;
}

tempo_utils::Result<tu_uint32>
lyric_assembler::ObjectPlugin::getOrInsertTrap(std::string_view name)
{
    if (name.empty())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid plugin trap name '{}'", name);
    auto entry = m_trapIndex.find(name);
    if (entry != m_trapIndex.cend())
        return entry->second;
    auto index = m_trapIndex.size();
    m_trapIndex[name] = index;
    return index;
}

absl::flat_hash_map<std::string,tu_uint32>::const_iterator
lyric_assembler::ObjectPlugin::trapsBegin() const
{
    return m_trapIndex.cbegin();
}

absl::flat_hash_map<std::string,tu_uint32>::const_iterator
lyric_assembler::ObjectPlugin::trapsEnd() const
{
    return m_trapIndex.cend();
}

tu_uint32
lyric_assembler::ObjectPlugin::numTraps() const
{
    return m_trapIndex.size();
}