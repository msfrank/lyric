#ifndef LYRIC_RUNTIME_INTERPRETER_STATE_H
#define LYRIC_RUNTIME_INTERPRETER_STATE_H

#include <uv.h>

#include <lyric_runtime/abstract_heap.h>
#include <lyric_runtime/abstract_loader.h>
#include <lyric_runtime/call_cell.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/heap_manager.h>
#include <lyric_runtime/port_multiplexer.h>
#include <lyric_runtime/ref_handle.h>
#include <lyric_runtime/segment_manager.h>
#include <lyric_runtime/stackful_coroutine.h>
#include <lyric_runtime/subroutine_manager.h>
#include <lyric_runtime/system_scheduler.h>

#include "type_manager.h"

namespace lyric_runtime {

    struct InterpreterStateOptions {
        std::shared_ptr<AbstractLoader> loader;
        lyric_common::AssemblyLocation preludeLocation = {};
        std::shared_ptr<AbstractLoader> bootstrapLoader = {};
        std::shared_ptr<AbstractHeap> heap = {};
    };

    class InterpreterState : public std::enable_shared_from_this<InterpreterState> {
    public:
        static tempo_utils::Result<std::shared_ptr<InterpreterState>> create(
            const InterpreterStateOptions &options,
            const lyric_common::AssemblyLocation &mainLocation = {});
        virtual ~InterpreterState();

        // subsystems

        virtual StackfulCoroutine *currentCoro() const;
        virtual SegmentManager *segmentManager() const;
        virtual TypeManager *typeManager() const;
        virtual SubroutineManager *subroutineManager() const;
        virtual SystemScheduler *systemScheduler() const;
        virtual PortMultiplexer *portMultiplexer() const;
        virtual HeapManager *heapManager() const;
        virtual uv_loop_t *mainLoop() const;

        tempo_utils::Status reload(const lyric_common::AssemblyLocation &mainLocation);

        // heap management

        RefHandle createHandle(const DataCell &ref);

    private:
        uv_loop_t *m_loop;
        std::shared_ptr<AbstractHeap> m_heap;
        SegmentManager *m_segmentManager;
        TypeManager *m_typeManager;
        SubroutineManager *m_subroutineManager;
        SystemScheduler *m_systemScheduler;
        PortMultiplexer *m_portMultiplexer;
        HeapManager *m_heapManager;

        InterpreterState(
            uv_loop_t *loop,
            std::shared_ptr<AbstractHeap> heap,
            SegmentManager *segmentManager,
            TypeManager *typeManager,
            SubroutineManager *subroutineManager,
            SystemScheduler *systemScheduler,
            PortMultiplexer *portMultiplexer,
            HeapManager *heapManager);

        static tempo_utils::Result<std::shared_ptr<InterpreterState>> createInterpreterState(
            const lyric_common::AssemblyLocation &location,
            uv_loop_t *m_loop,
            std::shared_ptr<AbstractHeap> heap,
            SegmentManager *segmentManager,
            SubroutineManager *subroutineManager,
            SystemScheduler *systemScheduler,
            PortMultiplexer *portMultiplexer,
            HeapManager *heapManager);

        friend class RefHandle;
    };
}

#endif // LYRIC_RUNTIME_INTERPRETER_STATE_H
