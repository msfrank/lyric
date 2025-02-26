#ifndef LYRIC_OPTIMIZER_PHI_FUNCTION_H
#define LYRIC_OPTIMIZER_PHI_FUNCTION_H

#include <memory>
#include <vector>

#include "instance.h"

namespace lyric_optimizer {

    namespace internal {
        struct PhiPriv;
        struct GraphPriv;
    }

    class PhiFunction {
    public:
        PhiFunction();
        PhiFunction(const PhiFunction &other);

        bool isValid() const;

        Instance getPhiTarget() const;
        std::vector<Instance>::const_iterator phiArgumentsBegin() const;
        std::vector<Instance>::const_iterator phiArgumentsEnd() const;
        int numPhiArguments() const;

        std::string toString() const;

    private:
        std::shared_ptr<internal::PhiPriv> m_phi;

        PhiFunction(std::shared_ptr<internal::PhiPriv> phi);

        friend class BasicBlock;
    };
}

#endif // LYRIC_OPTIMIZER_PHI_FUNCTION_H
