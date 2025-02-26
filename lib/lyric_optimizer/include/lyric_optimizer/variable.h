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

    class Variable {
    public:
        Variable();
        Variable(const Variable &other);

        bool isValid() const;

        tempo_utils::Result<Instance> pushInstance();

        std::string toString() const;

    private:
        std::shared_ptr<internal::VariablePriv> m_variable;
        std::shared_ptr<internal::GraphPriv> m_graph;

        Variable(
            std::shared_ptr<internal::VariablePriv> variable,
            std::shared_ptr<internal::GraphPriv> graph);

        friend class BasicBlock;
    };
}

#endif // LYRIC_OPTIMIZER_VARIABLE_H
