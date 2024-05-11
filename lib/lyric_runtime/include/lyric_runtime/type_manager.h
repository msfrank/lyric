#ifndef LYRIC_RUNTIME_TYPE_MANAGER_H
#define LYRIC_RUNTIME_TYPE_MANAGER_H

#include <tempo_utils/result.h>

#include "data_cell.h"
#include "runtime_types.h"
#include "segment_manager.h"

namespace lyric_runtime {

    class TypeManager {
    public:
        TypeManager(std::vector<DataCell> &&intrinsiccache, SegmentManager *segmentManager);
        virtual ~TypeManager() = default;

        tempo_utils::Result<DataCell> typeOf(const DataCell &value);
        tempo_utils::Result<TypeComparison> compareTypes(const DataCell &lhs,const DataCell &rhs);

    private:
        std::vector<DataCell> m_intrinsiccache;
        SegmentManager *m_segmentManager;
    };
}

#endif // LYRIC_RUNTIME_TYPE_MANAGER_H
