#ifndef LYRIC_RUNTIME_HEAP_MANAGER_H
#define LYRIC_RUNTIME_HEAP_MANAGER_H

#include <lyric_runtime/abstract_heap.h>
#include <lyric_runtime/segment_manager.h>
#include <lyric_runtime/system_scheduler.h>

namespace lyric_runtime {

    struct PreludeTables {
        const ExistentialTable *BytesTable = nullptr;
        const ExistentialTable *RestTable = nullptr;
        const ExistentialTable *StringTable = nullptr;
        const ExistentialTable *UrlTable = nullptr;
        const VirtualTable *CancelledTable = nullptr;
        const VirtualTable *InvalidArgumentTable = nullptr;
        const VirtualTable *DeadlineExceededTable = nullptr;
        const VirtualTable *NotFoundTable = nullptr;
        const VirtualTable *AlreadyExistsTable = nullptr;
        const VirtualTable *PermissionDeniedTable = nullptr;
        const VirtualTable *UnauthenticatedTable = nullptr;
        const VirtualTable *ResourceExhaustedTable = nullptr;
        const VirtualTable *FailedPreconditionTable = nullptr;
        const VirtualTable *AbortedTable = nullptr;
        const VirtualTable *UnavailableTable = nullptr;
        const VirtualTable *OutOfRangeTable = nullptr;
        const VirtualTable *UnimplementedTable = nullptr;
        const VirtualTable *InternalTable = nullptr;
        const VirtualTable *UnknownTable = nullptr;
    };

    class HeapManager {
    public:
        HeapManager(
            PreludeTables preludeTables,
            SegmentManager *segmentManager,
            SystemScheduler *systemScheduler,
            std::shared_ptr<AbstractHeap> heap);
        virtual ~HeapManager() = default;

        virtual DataCell allocateString(std::string_view string);
        virtual tempo_utils::Status loadLiteralStringOntoStack(tu_uint32 address);
        virtual tempo_utils::Status loadStringOntoStack(std::string_view string);

        virtual DataCell allocateUrl(const tempo_utils::Url &url);
        virtual tempo_utils::Status loadLiteralUrlOntoStack(tu_uint32 address);
        virtual tempo_utils::Status loadUrlOntoStack(const tempo_utils::Url &url);

        virtual DataCell allocateBytes(std::span<const tu_uint8> bytes);
        virtual tempo_utils::Status loadLiteralBytesOntoStack(tu_uint32 address);
        virtual tempo_utils::Status loadBytesOntoStack(std::span<const tu_uint8> bytes);

        virtual DataCell allocateStatus(const VirtualTable *vtable);
        virtual DataCell allocateStatus(
            tempo_utils::StatusCode statusCode,
            std::string_view message = {});
        virtual tempo_utils::Status loadStatusOntoStack(
            tempo_utils::StatusCode statusCode,
            std::string_view message = {});

        virtual DataCell allocateRest(const CallCell &frame);
        virtual tempo_utils::Status loadRestOntoStack(const CallCell &frame);

        // virtual NativeFunc prepareNew(tu_uint8 newType, tu_uint32 address, tempo_utils::Status &status);
        // virtual bool constructNew(std::vector<DataCell> &args, tempo_utils::Status &status);
        virtual tempo_utils::Status collectGarbage();

    private:
        PreludeTables m_preludeTables;
        SegmentManager *m_segmentManager;
        SystemScheduler *m_systemScheduler;
        std::shared_ptr<AbstractHeap> m_heap;

    public:
        /**
         * Allocate a new instance of RefType on the heap. This overload invokes a 0-argument RefType constructor.
         *
         * @tparam RefType
         * @param vtable
         * @return
         */
        template <class RefType>
        DataCell allocateRef(const VirtualTable *vtable)
        {
            auto *instance = new RefType(vtable);
            m_heap->insertInstance(instance);
            return DataCell::forRef(instance);
        }

        /**
         * Allocate a new instance of RefType on the heap. This overload invokes a 1-argument RefType constructor.
         *
         * @tparam RefType
         * @tparam Arg0
         * @param vtable
         * @param arg0
         * @return
         */
        template <class RefType, class Arg0>
        DataCell allocateRef(const VirtualTable *vtable, Arg0&& arg0)
        {
            auto *instance = new RefType(vtable, std::forward<Arg0>(arg0));
            m_heap->insertInstance(instance);
            return DataCell::forRef(instance);
        }

        /**
         * Allocate a new instance of RefType on the heap. This overload invokes a 2-argument RefType constructor.
         *
         * \tparam RefType
         * \tparam Arg0
         * \tparam Arg1
         * \param vtable
         * \param arg0
         * \param arg1
         * \return
         */
        template <class RefType, class Arg0, class Arg1>
        DataCell allocateRef(
            const VirtualTable *vtable,
            Arg0&& arg0,
            Arg1&& arg1)
        {
            auto *instance = new RefType(vtable, std::forward<Arg0>(arg0), std::forward<Arg1>(arg1));
            m_heap->insertInstance(instance);
            return DataCell::forRef(instance);
        }

        /**
         * Allocate a new instance of RefType on the heap. This overload invokes a 3-argument RefType constructor.
         *
         * \tparam RefType
         * \tparam Arg0
         * \tparam Arg1
         * \tparam Arg2
         * \param vtable
         * \param arg0
         * \param arg1
         * \param arg2
         * \return
         */
        template <class RefType, class Arg0, class Arg1, class Arg2>
        DataCell allocateRef(
            const VirtualTable *vtable,
            Arg0&& arg0,
            Arg1&& arg1,
            Arg2&& arg2)
        {
            auto *instance = new RefType(vtable,
                std::forward<Arg0>(arg0),
                std::forward<Arg1>(arg1),
                std::forward<Arg2>(arg2));
            m_heap->insertInstance(instance);
            return DataCell::forRef(instance);
        }

        /**
         * Allocate a new instance of RefType on the heap. This overload invokes a 4-argument RefType constructor.
         *
         * \tparam RefType
         * \tparam Arg0
         * \tparam Arg1
         * \tparam Arg2
         * \tparam Arg3
         * \param vtable
         * \param arg0
         * \param arg1
         * \param arg2
         * \param arg3
         * \return
         */
        template <class RefType, class Arg0, class Arg1, class Arg2, class Arg3>
        DataCell allocateRef(
            const VirtualTable *vtable,
            Arg0&& arg0,
            Arg1&& arg1,
            Arg2&& arg2,
            Arg3&& arg3)
        {
            auto *instance = new RefType(vtable,
                std::forward<Arg0>(arg0),
                std::forward<Arg1>(arg1),
                std::forward<Arg2>(arg2),
                std::forward<Arg3>(arg3));
            m_heap->insertInstance(instance);
            return DataCell::forRef(instance);
        }

        /**
         * Allocate a new instance of RefType on the heap. This overload invokes a 5-argument RefType constructor.
         *
         * \tparam RefType
         * \tparam Arg0
         * \tparam Arg1
         * \tparam Arg2
         * \tparam Arg3
         * \tparam Arg4
         * \param vtable
         * \param arg0
         * \param arg1
         * \param arg2
         * \param arg3
         * \param arg4
         * \return
         */
        template <class RefType, class Arg0, class Arg1, class Arg2, class Arg3, class Arg4>
        DataCell allocateRef(
            const VirtualTable *vtable,
            Arg0&& arg0,
            Arg1&& arg1,
            Arg2&& arg2,
            Arg3&& arg3,
            Arg4&& arg4)
        {
            auto *instance = new RefType(vtable,
                std::forward<Arg0>(arg0),
                std::forward<Arg1>(arg1),
                std::forward<Arg2>(arg2),
                std::forward<Arg3>(arg3),
                std::forward<Arg4>(arg4));
            m_heap->insertInstance(instance);
            return DataCell::forRef(instance);
        }
    };
}

#endif // LYRIC_RUNTIME_HEAP_MANAGER_H
