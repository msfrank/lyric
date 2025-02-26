
#include <lyric_optimizer/internal/cfg_data.h>
#include <lyric_optimizer/optimizer_result.h>
#include <lyric_optimizer/variable.h>

lyric_optimizer::Variable::Variable()
{
}

lyric_optimizer::Variable::Variable(const Variable &other)
    : m_variable(other.m_variable),
      m_graph(other.m_graph)
{
}

lyric_optimizer::Variable::Variable(
    std::shared_ptr<internal::VariablePriv> variable,
    std::shared_ptr<internal::GraphPriv> graph)
    : m_variable(std::move(variable)),
      m_graph(std::move(graph))
{
    TU_ASSERT (m_variable != nullptr);
    TU_ASSERT (m_graph != nullptr);
}

bool
lyric_optimizer::Variable::isValid() const
{
    return m_variable != nullptr;
}

tempo_utils::Result<lyric_optimizer::Instance>
lyric_optimizer::Variable::pushInstance()
{
    if (m_variable == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid variable");
    if (m_variable->counter == std::numeric_limits<tu_uint32>::max())
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "exceeded maximum variable instances");
    auto instance = std::make_shared<internal::InstancePriv>();
    instance->variable = m_variable;
    instance->generation = m_variable->counter++;
    m_variable->instances.push_back(instance);
    return Instance(instance);
}

std::string
lyric_optimizer::Variable::toString() const
{
    if (m_variable != nullptr)
        return m_variable->name;
    return "???";
}
