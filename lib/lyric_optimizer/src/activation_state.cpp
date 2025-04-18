
#include <lyric_optimizer/activation_state.h>
#include <lyric_optimizer/internal/cfg_data.h>

lyric_optimizer::ActivationState::ActivationState()
{
}

lyric_optimizer::ActivationState::ActivationState(std::weak_ptr<internal::GraphPriv> graph)
    : m_graph(graph)
{
    auto g = m_graph.lock();
    TU_ASSERT (g != nullptr);

    for (auto &priv : g->arguments) {
        Variable argument(priv, g);
        Instance instance;
        TU_ASSIGN_OR_RAISE (instance, argument.makeInstance());
        m_arguments.push_back(instance);
    }
    for (auto &priv : g->locals) {
        Variable local(priv, g);
        Instance instance;
        TU_ASSIGN_OR_RAISE (instance, local.makeInstance());
        m_locals.push_back(instance);
    }
    for (auto &priv : g->lexicals) {
        Variable lexical(priv, g);
        Instance instance;
        TU_ASSIGN_OR_RAISE (instance, lexical.makeInstance());
        m_lexicals.push_back(instance);
    }
}

lyric_optimizer::ActivationState::ActivationState(const ActivationState &other)
    : m_graph(other.m_graph),
      m_arguments(other.m_arguments),
      m_locals(other.m_locals),
      m_lexicals(other.m_lexicals)
{
}

bool
lyric_optimizer::ActivationState::isValid() const
{
    return !m_graph.expired();
}

tempo_utils::Result<lyric_optimizer::Instance>
lyric_optimizer::ActivationState::resolveArgument(lyric_assembler::ArgumentVariable *argumentVariable)
{
    TU_ASSERT (argumentVariable != nullptr);
    auto offset = argumentVariable->getOffset().getOffset();
    if (m_arguments.size() <= offset)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing argument {}", argumentVariable->getSymbolUrl().toString());
    auto &instance = m_arguments.at(offset);
    return instance;
}

tempo_utils::Result<lyric_optimizer::Instance>
lyric_optimizer::ActivationState::resolveLocal(lyric_assembler::LocalVariable *localVariable)
{
    TU_ASSERT (localVariable != nullptr);
    auto offset = localVariable->getOffset().getOffset();
    if (m_locals.size() <= offset)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing local {}", localVariable->getSymbolUrl().toString());
    auto &instance = m_locals.at(offset);
    return instance;
}

tempo_utils::Result<lyric_optimizer::Instance>
lyric_optimizer::ActivationState::resolveLexical(lyric_assembler::LexicalVariable *lexicalVariable)
{
    TU_ASSERT (lexicalVariable != nullptr);
    auto offset = lexicalVariable->getOffset().getOffset();
    if (m_lexicals.size() <= offset)
        return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
            "missing lexical {}", lexicalVariable->getSymbolUrl().toString());
    auto instance = m_lexicals.at(offset);
    return instance;
}

tempo_utils::Result<lyric_optimizer::Instance>
lyric_optimizer::ActivationState::resolveVariable(const Variable &variable)
{
    auto offset = variable.getOffset();
    switch (variable.getType()) {
        case VariableType::Argument: {
            if (m_arguments.size() <= offset)
                return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
                    "missing argument variable {}", variable.toString());
            return m_arguments.at(offset);
        }
        case VariableType::Local: {
            if (m_locals.size() <= offset)
                return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
                    "missing local variable {}", variable.toString());
            return m_locals.at(offset);
        }
        case VariableType::Lexical: {
            if (m_lexicals.size() <= offset)
                return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
                    "missing lexical variable {}", variable.toString());
            return m_lexicals.at(offset);
        }
        default:
            return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
                "invalid variable {}", variable.toString());
    }
}

tempo_utils::Status
lyric_optimizer::ActivationState::mutateVariable(const Variable &variable, const Instance &instance)
{
    auto offset = variable.getOffset();
    switch (variable.getType()) {
        case VariableType::Argument: {
            if (m_arguments.size() <= offset)
                return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
                    "missing argument variable {}", variable.toString());
            m_arguments[offset] = instance;
            return {};
        }
        case VariableType::Local: {
            if (m_locals.size() <= offset)
                return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
                    "missing local variable {}", variable.toString());
            m_locals[offset] = instance;
            return {};
        }
        case VariableType::Lexical: {
            if (m_lexicals.size() <= offset)
                return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
                    "missing lexical variable {}", variable.toString());
            m_lexicals[offset] = instance;
            return {};
        }
        default:
            return OptimizerStatus::forCondition(OptimizerCondition::kOptimizerInvariant,
                "invalid variable {}", variable.toString());
    }
}