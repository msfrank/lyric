#ifndef LYRIC_OPTIMIZER_INSTANCE_H
#define LYRIC_OPTIMIZER_INSTANCE_H

#include <memory>

namespace lyric_optimizer {

    namespace internal {
        struct InstancePriv;
    }

    class Instance {
    public:
        Instance();
        Instance(const Instance &other);

        bool isValid() const;

        std::string toString() const;

    private:
        std::shared_ptr<internal::InstancePriv> m_instance;

        explicit Instance(std::shared_ptr<internal::InstancePriv> instance);

        friend class Variable;
    };
}

#endif // LYRIC_OPTIMIZER_INSTANCE_H
