#ifndef LYRIC_OPTIMIZER_INSTANCE_H
#define LYRIC_OPTIMIZER_INSTANCE_H

#include <memory>

#include "abstract_directive.h"
#include "value.h"

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
        Value getValue() const;
        tempo_utils::Status setValue(const Value &value);

        bool isEquivalentTo(const Instance &other) const;

        std::string toString() const;

        bool operator<(const Instance &other) const;
        bool operator==(const Instance &other) const;
        template <typename H> friend H AbslHashValue(H h, const Instance &instance);

    private:
        std::shared_ptr<internal::InstancePriv> m_instance;

        explicit Instance(std::shared_ptr<internal::InstancePriv> instance);

        friend class ActivationState;
        friend class Variable;
    };

    template <typename H>
    H AbslHashValue(H h, const Instance &instance) {
        return H::combine(std::move(h), instance.getName());
    }
}

#endif // LYRIC_OPTIMIZER_INSTANCE_H
