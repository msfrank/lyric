
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
    return "";
}

std::string
lyric_optimizer::Instance::getVariableName() const
{
    if (m_instance != nullptr) {
        auto variable = m_instance->variable.lock();
        return variable->name;
    }
    return "";
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
    return m_instance->value.hasValue();
}

lyric_optimizer::Value
lyric_optimizer::Instance::getValue() const
{
    if (m_instance == nullptr)
        return {};
    return m_instance->value;
}

tempo_utils::Status
lyric_optimizer::Instance::setValue(const Value &value)
{
    if (!value.hasValue())
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid value for instance");
    if (m_instance == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid instance");
    if (m_instance->value.hasValue())
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "instance already has a value");
    auto variable = m_instance->variable.lock();
    if (variable == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid instance");
    if (variable->counter == internal::kCounterMax)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "exceeded maximum instances for variable {}", variable->name);
    m_instance->generation = variable->counter++;
    m_instance->value = value;
    variable->instances.push_back(m_instance);
    return {};
}

bool
lyric_optimizer::Instance::isEquivalentTo(const Instance &other) const
{
    auto lhs = getValue();
    auto rhs = other.getValue();
    return lhs.isEquivalentTo(rhs);
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
    if (!m_instance->value.hasValue())
        return absl::StrCat(variable->name, ":", m_instance->generation, " = ???");
    return absl::StrCat(
        variable->name, ":", m_instance->generation,
        " = ",
        m_instance->value.toString());
}

bool
lyric_optimizer::Instance::operator<(const Instance &other) const
{
    return getName() < other.getName();
}

bool
lyric_optimizer::Instance::operator==(const Instance &other) const
{
    return getName() == other.getName();
}
