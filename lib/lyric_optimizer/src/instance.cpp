
#include <lyric_optimizer/instance.h>
#include <lyric_optimizer/optimizer_result.h>

#include "lyric_optimizer/internal/cfg_data.h"

lyric_optimizer::Instance::Instance()
{
}

lyric_optimizer::Instance::Instance(std::shared_ptr<internal::InstancePriv> instance)
    : m_instance(std::move(instance))
{
    TU_ASSERT (m_instance != nullptr);
}

lyric_optimizer::Instance::Instance(const Instance &other)
    : m_instance(other.m_instance)
{
}

bool
lyric_optimizer::Instance::isValid() const
{
    return m_instance != nullptr;
}

std::string
lyric_optimizer::Instance::toString() const
{
    if (m_instance != nullptr) {
        auto variable = m_instance->variable.lock();
        if (variable != nullptr) {
            return absl::StrCat(variable->name, ":", m_instance->generation);
        }
    }
    return "???";
}

