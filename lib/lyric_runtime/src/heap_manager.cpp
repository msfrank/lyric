
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytes_ref.h>
#include <lyric_runtime/heap_manager.h>
#include <lyric_runtime/system_scheduler.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_runtime/url_ref.h>
#include <tempo_utils/big_endian.h>

#include "lyric_runtime/namespace_ref.h"
#include "lyric_runtime/protocol_ref.h"
#include "lyric_runtime/rest_ref.h"
#include "lyric_runtime/status_ref.h"

lyric_runtime::HeapManager::HeapManager(
    PreludeTables preludeTables,
    SegmentManager *segmentManager,
    SystemScheduler *systemScheduler,
    std::shared_ptr<AbstractHeap> heap)
    : m_preludeTables(std::move(preludeTables)),
      m_segmentManager(segmentManager),
      m_systemScheduler(systemScheduler),
      m_heap(std::move(heap))
{
    TU_ASSERT (m_heap != nullptr);
}

lyric_runtime::DataCell
lyric_runtime::HeapManager::allocateString(std::string_view string)
{
    auto *instance = new StringRef(m_preludeTables.StringTable, string.data(), string.size());
    m_heap->insertInstance(instance);
    return DataCell::forString(instance);
}

tempo_utils::Status
lyric_runtime::HeapManager::loadLiteralStringOntoStack(tu_uint32 address)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);
    auto *sp = currentCoro->peekSP();
    TU_ASSERT (sp != nullptr);

    LiteralCell literal;
    tempo_utils::Status status;
    literal = m_segmentManager->resolveLiteral(sp, address, status);
    TU_RETURN_IF_NOT_OK (status);

    auto *instance = new StringRef(m_preludeTables.StringTable, literal);
    m_heap->insertInstance(instance);
    auto cell = DataCell::forString(instance);
    currentCoro->pushData(cell);

    return status;
}

tempo_utils::Status
lyric_runtime::HeapManager::loadStringOntoStack(std::string_view string)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);

    auto *instance = new StringRef(m_preludeTables.StringTable, string.data(), string.size());
    m_heap->insertInstance(instance);
    auto cell = DataCell::forString(instance);
    currentCoro->pushData(cell);

    return {};
}

lyric_runtime::DataCell
lyric_runtime::HeapManager::allocateUrl(const tempo_utils::Url &url)
{
    auto *instance = new UrlRef(m_preludeTables.UrlTable, url);
    m_heap->insertInstance(instance);
    return DataCell::forUrl(instance);
}

tempo_utils::Status
lyric_runtime::HeapManager::loadLiteralUrlOntoStack(tu_uint32 address)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);
    auto *sp = currentCoro->peekSP();
    TU_ASSERT (sp != nullptr);

    LiteralCell literal;
    tempo_utils::Status status;
    literal = m_segmentManager->resolveLiteral(sp, address, status);
    TU_RETURN_IF_NOT_OK (status);

    auto *instance = new UrlRef(m_preludeTables.UrlTable, literal);
    m_heap->insertInstance(instance);
    auto cell = DataCell::forUrl(instance);
    currentCoro->pushData(cell);

    return status;
}

tempo_utils::Status
lyric_runtime::HeapManager::loadUrlOntoStack(const tempo_utils::Url &url)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);

    auto *instance = new UrlRef(m_preludeTables.UrlTable, url);
    m_heap->insertInstance(instance);
    auto cell = DataCell::forUrl(instance);
    currentCoro->pushData(cell);

    return {};
}

lyric_runtime::DataCell
lyric_runtime::HeapManager::allocateBytes(std::span<const tu_uint8> bytes)
{
    auto *instance = new BytesRef(m_preludeTables.BytesTable, bytes.data(), bytes.size());
    m_heap->insertInstance(instance);
    return DataCell::forBytes(instance);
}

tempo_utils::Status
lyric_runtime::HeapManager::loadLiteralBytesOntoStack(tu_uint32 address)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);
    auto *sp = currentCoro->peekSP();
    TU_ASSERT (sp != nullptr);

    LiteralCell literal;
    tempo_utils::Status status;
    literal = m_segmentManager->resolveLiteral(sp, address, status);
    TU_RETURN_IF_NOT_OK (status);

    auto *instance = new BytesRef(m_preludeTables.BytesTable, literal);
    m_heap->insertInstance(instance);
    auto cell = DataCell::forBytes(instance);
    currentCoro->pushData(cell);

    return status;
}

tempo_utils::Status
lyric_runtime::HeapManager::loadBytesOntoStack(std::span<const tu_uint8> bytes)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);

    auto *instance = new BytesRef(m_preludeTables.BytesTable, bytes.data(), bytes.size());
    m_heap->insertInstance(instance);
    auto cell = DataCell::forBytes(instance);
    currentCoro->pushData(cell);

    return {};
}

inline const lyric_runtime::VirtualTable *
status_code_to_vtable(tempo_utils::StatusCode statusCode, const lyric_runtime::PreludeTables &preludeTables)
{
    switch (statusCode) {
        case tempo_utils::StatusCode::kCancelled:
            return preludeTables.CancelledTable;
        case tempo_utils::StatusCode::kInvalidArgument:
            return preludeTables.InvalidArgumentTable;
        case tempo_utils::StatusCode::kDeadlineExceeded:
            return preludeTables.DeadlineExceededTable;
        case tempo_utils::StatusCode::kNotFound:
            return preludeTables.NotFoundTable;
        case tempo_utils::StatusCode::kAlreadyExists:
            return preludeTables.AlreadyExistsTable;
        case tempo_utils::StatusCode::kPermissionDenied:
            return preludeTables.PermissionDeniedTable;
        case tempo_utils::StatusCode::kUnauthenticated:
            return preludeTables.UnauthenticatedTable;
        case tempo_utils::StatusCode::kResourceExhausted:
            return preludeTables.ResourceExhaustedTable;
        case tempo_utils::StatusCode::kFailedPrecondition:
            return preludeTables.FailedPreconditionTable;
        case tempo_utils::StatusCode::kAborted:
            return preludeTables.AbortedTable;
        case tempo_utils::StatusCode::kUnavailable:
            return preludeTables.UnavailableTable;
        case tempo_utils::StatusCode::kOutOfRange:
            return preludeTables.OutOfRangeTable;
        case tempo_utils::StatusCode::kUnimplemented:
            return preludeTables.UnimplementedTable;
        case tempo_utils::StatusCode::kInternal:
            return preludeTables.InternalTable;
        case tempo_utils::StatusCode::kUnknown:
            return preludeTables.UnknownTable;
        default:
            return nullptr;
    }
}

lyric_runtime::DataCell
lyric_runtime::HeapManager::allocateStatus(const VirtualTable *vtable)
{
    auto *instance = new StatusRef(vtable);
    m_heap->insertInstance(instance);
    return DataCell::forStatus(instance);
}

lyric_runtime::DataCell
lyric_runtime::HeapManager::allocateStatus(tempo_utils::StatusCode statusCode, std::string_view message)
{
    const VirtualTable *vtable = status_code_to_vtable(statusCode, m_preludeTables);
    auto str = allocateString(message);
    auto *instance = new StatusRef(vtable, statusCode, str.data.str);
    m_heap->insertInstance(instance);
    return DataCell::forStatus(instance);
}

tempo_utils::Status
lyric_runtime::HeapManager::loadStatusOntoStack(tempo_utils::StatusCode statusCode, std::string_view message)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);

    const VirtualTable *vtable = status_code_to_vtable(statusCode, m_preludeTables);
    auto str = allocateString(message);
    auto *instance = new StatusRef(vtable, statusCode, str.data.str);
    m_heap->insertInstance(instance);
    auto cell = DataCell::forStatus(instance);
    currentCoro->pushData(cell);

    return {};
}

lyric_runtime::DataCell
lyric_runtime::HeapManager::allocateRest(const CallCell &frame)
{
    std::vector<DataCell> restArgs;
    for (int i = 0; i < frame.numRest(); i++) {
        restArgs.push_back(frame.getRest(i));
    }
    auto *instance = new RestRef(m_preludeTables.RestTable, std::move(restArgs));
    m_heap->insertInstance(instance);
    return DataCell::forRest(instance);
}

tempo_utils::Status
lyric_runtime::HeapManager::loadRestOntoStack(const CallCell &frame)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);

    auto cell = allocateRest(frame);
    currentCoro->pushData(cell);

    return {};
}

lyric_runtime::DataCell
lyric_runtime::HeapManager::allocateProtocol(const DataCell &descriptor)
{
    TU_ASSERT (descriptor.type == DataCellType::DESCRIPTOR);
    TU_ASSERT (descriptor.data.descriptor->getLinkageSection() == lyric_object::LinkageSection::Protocol);
    auto *segment = descriptor.data.descriptor->getSegment();
    auto object = segment->getObject();
    auto walker = object.getProtocol(descriptor.data.descriptor->getDescriptorIndex());
    auto port = walker.getPort();
    auto comm = walker.getCommunication();
    auto *protocolType = segment->lookupType(walker.getProtocolType().getDescriptorOffset());
    auto type = DataCell::forType(protocolType);

    auto *instance = new ProtocolRef(m_preludeTables.ProtocolTable, descriptor, type, port, comm);
    m_heap->insertInstance(instance);
    return DataCell::forProtocol(instance);
}

tempo_utils::Status
lyric_runtime::HeapManager::loadProtocolOntoStack(const DataCell &descriptor)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);

    auto cell = allocateProtocol(descriptor);
    currentCoro->pushData(cell);

    return {};
}

lyric_runtime::DataCell
lyric_runtime::HeapManager::allocateNamespace(const DataCell &descriptor)
{
    TU_ASSERT (descriptor.type == DataCellType::DESCRIPTOR);
    TU_ASSERT (descriptor.data.descriptor->getLinkageSection() == lyric_object::LinkageSection::Namespace);
    auto *segment = descriptor.data.descriptor->getSegment();
    auto object = segment->getObject();
    auto walker = object.getNamespace(descriptor.data.descriptor->getDescriptorIndex());
    auto *namespaceType = segment->lookupType(walker.getNamespaceType().getDescriptorOffset());
    auto type = DataCell::forType(namespaceType);

    auto *instance = new NamespaceRef(m_preludeTables.NamespaceTable, descriptor, type);
    m_heap->insertInstance(instance);
    return DataCell::forNamespace(instance);
}

tempo_utils::Status
lyric_runtime::HeapManager::loadNamespaceOntoStack(const DataCell &descriptor)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);

    auto cell = allocateNamespace(descriptor);
    currentCoro->pushData(cell);

    return {};
}

static void
set_reachable_for_task(lyric_runtime::Task *task)
{
    if (task == nullptr)
        return;

    auto *coro = task->stackfulCoroutine();

    // walk the data stack and mark all reachable instances
    for (auto iterator = coro->dataBegin(); iterator != coro->dataEnd(); iterator++) {
        if (iterator->type == lyric_runtime::DataCellType::REF) {
            auto *instance = iterator->data.ref;
            if (instance) {
                instance->setReachable();
            }
        }
    }
}

tempo_utils::Status
lyric_runtime::HeapManager::collectGarbage() {

    // clear reachable flag for all heap allocated instances
    m_heap->clearReachable();

    set_reachable_for_task(m_systemScheduler->currentTask());

    for (Task *task = m_systemScheduler->firstWaitingTask(); task != nullptr; task = task->nextTask()) {
        set_reachable_for_task(task);
    }

    for (Task *task = m_systemScheduler->firstReadyTask(); task != nullptr; task = task->nextTask()) {
        set_reachable_for_task(task);
    }

    Waiter *waiterHead = m_systemScheduler->firstWaiter();
    if (waiterHead != nullptr) {
        Waiter *waiter = waiterHead;
        Waiter *next = waiter->nextWaiter();
        do {
            if (waiter->hasPromise()) {
                auto promise = waiter->getPromise();
                promise->setReachable();
            }
            waiter = next;
            next = waiter->nextWaiter();
        } while (next != nullptr && next != waiterHead);
    }

    // scan the heap and delete all unreachable instances
    m_heap->deleteUnreachable();

    return {};
}
