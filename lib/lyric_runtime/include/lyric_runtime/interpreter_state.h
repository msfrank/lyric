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
        /**
         *
         */
        lyric_common::ModuleLocation preludeLocation = {};
        /**
         *
         */
        lyric_common::ModuleLocation mainLocation = {};
        /**
         *
         */
        std::shared_ptr<AbstractLoader> bootstrapLoader = {};
        /**
         *
         */
        std::shared_ptr<AbstractHeap> heap = {};
    };

    class InterpreterState : public std::enable_shared_from_this<InterpreterState> {
    public:
        virtual ~InterpreterState();

        lyric_common::ModuleLocation getMainLocation() const;
        tu_uint64 getLoadEpochMillis() const;
        tempo_utils::StatusCode getStatusCode() const;
        bool isActive() const;

        // subsystems

        virtual StackfulCoroutine *currentCoro() const;
        virtual SegmentManager *segmentManager() const;
        virtual TypeManager *typeManager() const;
        virtual SubroutineManager *subroutineManager() const;
        virtual SystemScheduler *systemScheduler() const;
        virtual PortMultiplexer *portMultiplexer() const;
        virtual HeapManager *heapManager() const;
        virtual uv_loop_t *mainLoop() const;

        tempo_utils::Status load(const lyric_common::ModuleLocation &mainLocation);
        tempo_utils::Status halt(tempo_utils::StatusCode statusCode);

        // heap management

        RefHandle createHandle(const DataCell &ref);

        static tempo_utils::Result<std::shared_ptr<InterpreterState>> create(
            std::shared_ptr<AbstractLoader> loader,
            const InterpreterStateOptions &options = {});

    private:
        // set in constructor
        uv_loop_t *m_loop;
        lyric_common::ModuleLocation m_preludeLocation;
        std::shared_ptr<AbstractLoader> m_loader;
        std::shared_ptr<AbstractHeap> m_heap;

        // set in initialize method
        std::unique_ptr<SegmentManager> m_segmentManager;
        std::unique_ptr<TypeManager> m_typeManager;
        std::unique_ptr<SubroutineManager> m_subroutineManager;
        std::unique_ptr<SystemScheduler> m_systemScheduler;
        std::unique_ptr<PortMultiplexer> m_portMultiplexer;
        std::unique_ptr<HeapManager> m_heapManager;

        // set in load method
        lyric_common::ModuleLocation m_mainLocation;
        tu_uint64 m_loadEpochMillis;
        tempo_utils::StatusCode m_statusCode;
        bool m_active;

        InterpreterState(
            uv_loop_t *loop,
            lyric_common::ModuleLocation preludeLocation,
            std::shared_ptr<AbstractLoader> loader,
            std::shared_ptr<AbstractHeap> heap);

        tempo_utils::Status initialize(const lyric_common::ModuleLocation &mainLocation);

        friend class BytecodeInterpreter;
        friend class RefHandle;
    };
}

#endif // LYRIC_RUNTIME_INTERPRETER_STATE_H
