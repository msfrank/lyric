
#include <lyric_optimizer/internal/cfg_data.h>
#include <lyric_optimizer/optimizer_result.h>
#include <lyric_optimizer/value.h>

lyric_optimizer::Value::Value()
    : m_value(std::make_shared<internal::ValuePriv>())
{
}

lyric_optimizer::Value::Value(std::shared_ptr<AbstractDirective> value)
    : m_value(std::make_shared<internal::ValuePriv>())
{
    TU_ASSERT (value != nullptr);
    m_value->values.push_front(value);
}

lyric_optimizer::Value::Value(std::shared_ptr<internal::ValuePriv> instance)
    : m_value(std::move(instance))
{
    TU_ASSERT (m_value != nullptr);
}

lyric_optimizer::Value::Value(const Value &other)
    : m_value(other.m_value)
{
}

bool
lyric_optimizer::Value::hasValue() const
{
    return !m_value->values.empty();
}

std::shared_ptr<lyric_optimizer::AbstractDirective>
lyric_optimizer::Value::getValue() const
{
    if (m_value->values.empty())
        return {};
    return m_value->values.front();;
}

tempo_utils::Status
lyric_optimizer::Value::updateValue(std::shared_ptr<AbstractDirective> value)
{
    if (value == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid value");
    m_value->values.push_front(value);
    return {};
}

tempo_utils::Status
lyric_optimizer::Value::updateValue(const Value &value)
{
    m_value = value.m_value;
    return {};
}

bool
lyric_optimizer::Value::isEquivalentTo(const Value &other) const
{
    auto lhs = getValue();
    auto rhs = other.getValue();
    if (lhs != nullptr) {
        if (rhs == nullptr)
            return false;
        return lhs->isEquivalentTo(rhs);
    }
    return rhs == nullptr;
}

std::string
lyric_optimizer::Value::toString() const
{
    if (m_value->values.empty())
        return "???";
    auto value = m_value->values.front();
    return value->toString();
}

bool
lyric_optimizer::Value::operator==(const Value &other) const
{
    return isEquivalentTo(other);
}
