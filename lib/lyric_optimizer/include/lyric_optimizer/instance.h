#ifndef LYRIC_OPTIMIZER_INSTANCE_H
#define LYRIC_OPTIMIZER_INSTANCE_H

#include <memory>

#include "abstract_directive.h"

namespace lyric_optimizer {

    namespace internal {
        struct InstancePriv;
    }

    class Instance {
    public:
        Instance();
        Instance(const Instance &other);

        bool isValid() const;

        std::string getName() const;
        std::string getVariableName() const;
        tu_uint32 getGeneration() const;

        bool hasValue() const;
        std::shared_ptr<AbstractDirective> getValue() const;
        tempo_utils::Status updateValue(std::shared_ptr<AbstractDirective> value);

        std::string toString() const;

    private:
        std::shared_ptr<internal::InstancePriv> m_instance;

        explicit Instance(std::shared_ptr<internal::InstancePriv> instance);

        friend class ActivationState;
        friend class Variable;
    };
}

#endif // LYRIC_OPTIMIZER_INSTANCE_H
