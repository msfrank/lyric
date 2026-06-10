#ifndef LYRIC_RUNTIME_TYPE_MANAGER_H
#define LYRIC_RUNTIME_TYPE_MANAGER_H

#include <tempo_utils/result.h>

#include "runtime_types.h"
#include "segment_manager.h"

namespace lyric_runtime {

    class TypeManager {
    public:
        TypeManager(std::vector<Operand> &&intrinsiccache, SegmentManager *segmentManager);
        virtual ~TypeManager() = default;

        tempo_utils::Result<Operand> typeOf(const Operand &value);
        tempo_utils::Result<TypeComparison> compareTypes(const Operand &lhs,const Operand &rhs);

    private:
        std::vector<Operand> m_intrinsiccache;
        SegmentManager *m_segmentManager;
    };
}

#endif // LYRIC_RUNTIME_TYPE_MANAGER_H
