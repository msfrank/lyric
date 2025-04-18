#ifndef LYRIC_OPTIMIZER_ACTIVATION_STATE_H
#define LYRIC_OPTIMIZER_ACTIVATION_STATE_H

#include <vector>

#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/lexical_variable.h>
#include <lyric_assembler/local_variable.h>

#include "instance.h"
#include "optimizer_directives.h"
#include "value.h"

namespace lyric_optimizer {

    // forward declarations
    namespace internal {
        struct GraphPriv;
    }

    class ActivationState {
    public:
        ActivationState();
        explicit ActivationState(std::weak_ptr<internal::GraphPriv> graph);
        ActivationState(const ActivationState &other);

        bool isValid() const;

        tempo_utils::Result<Instance> resolveArgument(
            lyric_assembler::ArgumentVariable *argumentVariable);
        tempo_utils::Result<Instance> resolveLocal(
            lyric_assembler::LocalVariable *localVariable);
        tempo_utils::Result<Instance> resolveLexical(
            lyric_assembler::LexicalVariable *lexicalVariable);
        tempo_utils::Result<Instance> resolveVariable(
            const Variable &variable);

        tempo_utils::Status mutateVariable(const Variable &variable, const Instance &instance);

    private:
        std::weak_ptr<internal::GraphPriv> m_graph;
        std::vector<Instance> m_arguments;
        std::vector<Instance> m_locals;
        std::vector<Instance> m_lexicals;
    };
}

#endif // LYRIC_OPTIMIZER_ACTIVATION_STATE_H