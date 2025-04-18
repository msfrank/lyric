#ifndef LYRIC_OPTIMIZER_VALUE_H
#define LYRIC_OPTIMIZER_VALUE_H

#include "abstract_directive.h"

namespace lyric_optimizer {

    namespace internal {
        struct ValuePriv;
    }

    class Value {
    public:
        Value();
        explicit Value(std::shared_ptr<AbstractDirective> value);
        Value(const Value &other);

        bool hasValue() const;
        std::shared_ptr<AbstractDirective> getValue() const;
        tempo_utils::Status updateValue(std::shared_ptr<AbstractDirective> value);
        tempo_utils::Status updateValue(const Value &value);

        bool isEquivalentTo(const Value &other) const;

        std::string toString() const;

        bool operator==(const Value &other) const;
        template <typename H> friend H AbslHashValue(H h, const Value &value);

    private:
        std::shared_ptr<internal::ValuePriv> m_value;

        explicit Value(std::shared_ptr<internal::ValuePriv> value);

        friend class ActivationState;
        friend class Instance;

        void HashValue(absl::HashState state) const;
    };

    template <typename H>
    H AbslHashValue(H h, const Value &value) {
        return H::combine(std::move(h), value.hasValue());
    }
}

#endif // LYRIC_OPTIMIZER_VALUE_H
