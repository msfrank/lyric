
#include <lyric_runtime/trap_index.h>

#include "lyric_object/lyric_object.h"
#include "lyric_runtime/interpreter_result.h"

lyric_runtime::TrapIndex::TrapIndex(std::shared_ptr<const AbstractPlugin> plugin)
    : m_plugin(std::move(plugin))
{
    TU_ASSERT (m_plugin != nullptr);

}

tempo_utils::Status
lyric_runtime::TrapIndex::initialize()
{
    for (tu_int32 i = 0; i < m_plugin->numTraps(); i++) {
        auto *trap = m_plugin->getTrap(i);
        if (trap == nullptr)
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "invalid trap at index {}", i);
        if (trap->name == nullptr)
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "invalid trap at index {}", i);
        std::string_view name(trap->name);
        if (name.empty())
            return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                "invalid trap at index {}", i);
        m_traps[name] = i;
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