
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
lyric_optimizer::Instance::getName() const
{
    if (m_instance != nullptr) {
        auto variable = m_instance->variable.lock();
        if (variable != nullptr) {
            return absl::StrCat(variable->name, ":", m_instance->generation);
        }
    }
    return "???";
}

std::string
lyric_optimizer::Instance::getVariableName() const
{
    if (m_instance != nullptr) {
        auto variable = m_instance->variable.lock();
        return variable->name;
    }
    return "???";
}

tu_uint32
lyric_optimizer::Instance::getGeneration() const
{
    if (m_instance != nullptr)
        return m_instance->generation;
    return 0;
}

bool
lyric_optimizer::Instance::hasValue() const
{
    if (m_instance == nullptr)
        return false;
    return !m_instance->values.empty();
}

std::shared_ptr<lyric_optimizer::AbstractDirective>
lyric_optimizer::Instance::getValue() const
{
    if (m_instance == nullptr)
        return {};
    if (m_instance->values.empty())
        return {};
    return m_instance->values.front();;
}

tempo_utils::Status
lyric_optimizer::Instance::updateValue(std::shared_ptr<AbstractDirective> value)
{
    if (m_instance == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid instance");
    m_instance->values.push_front(value);
    return {};
}

std::string
lyric_optimizer::Instance::toString() const
{
    if (m_instance == nullptr)
        return "???";
    auto variable = m_instance->variable.lock();
    if (variable == nullptr)
        return "???";
    auto name = variable->name;
    if (m_instance->values.empty())
        return absl::StrCat(variable->name, ":", m_instance->generation, " = ???");
    auto value = m_instance->values.front();
    return absl::StrCat(
        variable->name, ":", m_instance->generation,
        " = ",
        value->toString());
}

