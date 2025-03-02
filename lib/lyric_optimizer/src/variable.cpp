
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

lyric_optimizer::VariableType
lyric_optimizer::Variable::getType() const
{
    if (m_variable == nullptr)
        return VariableType::Invalid;
    switch (m_variable->type) {
        case lyric_assembler::SymbolType::ARGUMENT:
            return VariableType::Argument;
        case lyric_assembler::SymbolType::LOCAL:
            return VariableType::Local;
        case lyric_assembler::SymbolType::LEXICAL:
            return VariableType::Lexical;
        default:
            return VariableType::Invalid;
    }
}

int
lyric_optimizer::Variable::getOffset() const
{
    if (m_variable == nullptr)
        return -1;
    return m_variable->offset;
}

tempo_utils::Result<lyric_optimizer::Instance>
lyric_optimizer::Variable::currentInstance() const
{
    if (m_variable == nullptr)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid variable");
    if (m_variable->instances.empty())
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "invalid variable");
    auto instance = m_variable->instances.back();
    return Instance(instance);
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
