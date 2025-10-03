#ifndef LYRIC_RUNTIME_SUBROUTINE_MANAGER_H
#define LYRIC_RUNTIME_SUBROUTINE_MANAGER_H

#include <lyric_runtime/segment_manager.h>
#include <lyric_runtime/stackful_coroutine.h>

namespace lyric_runtime {

    class SubroutineManager {
    public:
        explicit SubroutineManager(SegmentManager *segmentManager);
        virtual ~SubroutineManager() = default;

        bool callProc(
            tu_uint32 callIndex,
            BytecodeSegment *segment,
            tu_uint32 procOffset,
            std::vector<DataCell> &args,
            bool returnsValue,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool callStatic(
            tu_uint32 address,
            std::vector<DataCell> &args,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool callStatic(
            const DataCell &descriptor,
            std::vector<DataCell> &args,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool callVirtual(
            const DataCell &receiver,
            tu_uint32 address,
            std::vector<DataCell> &args,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool callConcept(
            const DataCell &receiver,
            const DataCell &descriptor,
            tu_uint32 address,
            std::vector<DataCell> &args,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool callExistential(
            const DataCell &receiver,
            const DataCell &existentialDescriptor,
            tu_uint32 methodAddress,
            std::vector<DataCell> &args,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool initStatic(
            tu_uint32 address,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool returnToCaller(
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool raiseException(
            const DataCell &exc,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

    private:
        SegmentManager *m_segmentManager;
    };
}

#endif // LYRIC_RUNTIME_SUBROUTINE_MANAGER_H
