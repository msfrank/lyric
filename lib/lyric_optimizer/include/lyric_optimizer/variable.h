#ifndef LYRIC_OPTIMIZER_VARIABLE_H
#define LYRIC_OPTIMIZER_VARIABLE_H

#include <memory>

#include <tempo_utils/result.h>

#include "instance.h"

namespace lyric_optimizer {

    namespace internal {
        struct VariablePriv;
        struct BasicBlockPriv;
        struct GraphPriv;
    }

    enum class VariableType {
        Invalid,
        Argument,
        Local,
        Lexical,
    };

    class Variable {
    public:
        Variable();
        Variable(const Variable &other);

        bool isValid() const;

        std::string getName() const;
        VariableType getType() const;
        int getOffset() const;

        Instance getInstance(int offset) const;
        tempo_utils::Result<Instance> makeInstance();
        int numInstances() const;

        std::string toString() const;

        bool operator<(const Variable &other) const;
        bool operator==(const Variable &other) const;
        template <typename H> friend H AbslHashValue(H h, const Variable &instance);

    private:
        std::shared_ptr<internal::VariablePriv> m_variable;
        std::shared_ptr<internal::GraphPriv> m_graph;

        Variable(
            std::shared_ptr<internal::VariablePriv> variable,
            std::shared_ptr<internal::GraphPriv> graph);

        friend class ActivationState;
        friend class BasicBlock;
        friend class ControlFlowGraph;
    };

    template <typename H>
    H AbslHashValue(H h, const Variable &variable) {
        return H::combine(std::move(h), variable.getName());
    }
}

#endif // LYRIC_OPTIMIZER_VARIABLE_H
