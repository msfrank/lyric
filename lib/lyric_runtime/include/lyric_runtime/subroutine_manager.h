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
            std::vector<Operand> &args,
            bool returnsValue,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool callStatic(
            tu_uint32 address,
            std::vector<Operand> &args,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool callStatic(
            const Operand &descriptor,
            std::vector<Operand> &args,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool callVirtual(
            const Operand &receiver,
            tu_uint32 callAddress,
            std::vector<Operand> &args,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool callStub(
            const Operand &receiver,
            tu_uint32 actionAddress,
            std::vector<Operand> &args,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool callConcept(
            const Operand &receiver,
            const Operand &descriptor,
            tu_uint32 address,
            std::vector<Operand> &args,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool callExistential(
            const Operand &receiver,
            const Operand &existentialDescriptor,
            tu_uint32 methodAddress,
            std::vector<Operand> &args,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool initStatic(
            tu_uint32 address,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        bool returnToCaller(
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

    private:
        SegmentManager *m_segmentManager;
    };

    tempo_utils::Status process_arguments(
        const lyric_object::ProcInfo &procInfo,
        const std::vector<Operand> &args,
        tu_uint16 &numRest);

    tempo_utils::Status import_lexicals_into_frame(
        const lyric_object::ProcInfo &procInfo,
        const StackfulCoroutine *currentCoro,
        const BytecodeSegment *segment,
        CallCell &frame);

}

#endif // LYRIC_RUNTIME_SUBROUTINE_MANAGER_H
