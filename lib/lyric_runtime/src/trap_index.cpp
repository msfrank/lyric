
#include <lyric_object/lyric_object.h>
#include <lyric_runtime/interpreter_result.h>
#include <lyric_runtime/trap_index.h>

lyric_runtime::TrapIndex::TrapIndex(std::shared_ptr<const AbstractPlugin> plugin)
    : m_plugin(std::move(plugin)),
      m_iface(nullptr)
{
    TU_ASSERT (m_plugin != nullptr);
}

lyric_runtime::TrapIndex::TrapIndex(const NativeInterface *iface)
    : m_iface(iface)
{
    TU_ASSERT (m_iface != nullptr);
}

inline tempo_utils::Status
insert_trap(
    const lyric_runtime::NativeTrap *trap,
    tu_uint32 index,
    absl::flat_hash_map<std::string,tu_uint32> &traps)
{
    if (trap == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid trap at index {}", index);
    if (trap->name == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid trap at index {}", index);
    std::string_view name(trap->name);
    if (name.empty())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid trap at index {}", index);
    traps[name] = index;
    return {};
}

tempo_utils::Status
lyric_runtime::TrapIndex::initialize()
{
    if (m_plugin != nullptr) {
        for (tu_int32 i = 0; i < m_plugin->numTraps(); i++) {
            TU_RETURN_IF_NOT_OK (insert_trap(m_plugin->getTrap(i), i, m_traps));
        }
    } else {
        for (tu_int32 i = 0; i < m_iface->numTraps(); i++) {
            TU_RETURN_IF_NOT_OK (insert_trap(m_iface->getTrap(i), i, m_traps));
        }
    }
    return {};
}

tu_uint32
lyric_runtime::TrapIndex::lookupTrap(std::string_view name) const
{
    auto entry = m_traps.find(name);
    if (entry != m_traps.cend())
        return entry->second;
    return lyric_object::INVALID_ADDRESS_U32;
}