
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytes_ref.h>
#include <lyric_runtime/f64_ref.h>
#include <lyric_runtime/heap_manager.h>
#include <lyric_runtime/i64_ref.h>
#include <lyric_runtime/namespace_ref.h>
#include <lyric_runtime/protocol_ref.h>
#include <lyric_runtime/rest_ref.h>
#include <lyric_runtime/status_ref.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_runtime/system_scheduler.h>
#include <lyric_runtime/u64_ref.h>

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

lyric_runtime::Operand
lyric_runtime::HeapManager::allocateI64(tu_int64 i64)
{
    auto *instance = new I64Ref(m_preludeTables.I64Table, i64);
    m_heap->insertInstance(instance);
    return Operand::fromI64(instance);
}

lyric_runtime::Operand
lyric_runtime::HeapManager::allocateU64(tu_uint64 u64)
{
    auto *instance = new U64Ref(m_preludeTables.U64Table, u64);
    m_heap->insertInstance(instance);
    return Operand::fromU64(instance);
}

lyric_runtime::Operand
lyric_runtime::HeapManager::allocateF64(double f64)
{
    auto *instance = new F64Ref(m_preludeTables.F64Table, f64);
    m_heap->insertInstance(instance);
    return Operand::fromF64(instance);
}

lyric_runtime::Operand
lyric_runtime::HeapManager::allocateString(std::string_view string, bool isPermanent)
{
    auto *instance = new StringRef(m_preludeTables.StringTable, string.data(), string.size());
    if (isPermanent) {
        instance->setPermanent();
    }
    m_heap->insertInstance(instance);
    return Operand::fromString(instance);
}

lyric_runtime::Operand
lyric_runtime::HeapManager::allocateString(tempo_utils::Rope<char> rope, bool isPermanent)
{
    auto *instance = new StringRef(m_preludeTables.StringTable, rope);
    if (isPermanent) {
        instance->setPermanent();
    }
    m_heap->insertInstance(instance);
    return Operand::fromString(instance);
}

tempo_utils::Status
lyric_runtime::HeapManager::loadLiteralStringOntoStack(tu_uint32 address)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);
    auto *sp = currentCoro->peekSP();
    TU_ASSERT (sp != nullptr);

    tempo_utils::Status status;
    auto literal = m_segmentManager->resolveString(sp, address, status);
    TU_RETURN_IF_NOT_OK (status);

    auto *instance = new StringRef(m_preludeTables.StringTable, literal);
    m_heap->insertInstance(instance);
    auto operand = Operand::fromString(instance);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(operand));

    return {};
}

tempo_utils::Status
lyric_runtime::HeapManager::loadStringOntoStack(std::string_view string)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);

    auto *instance = new StringRef(m_preludeTables.StringTable, string.data(), string.size());
    m_heap->insertInstance(instance);
    auto operand = Operand::fromString(instance);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(operand));

    return {};
}

tempo_utils::Status
lyric_runtime::HeapManager::loadStringOntoStack(tempo_utils::Rope<char> rope)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);

    auto *instance = new StringRef(m_preludeTables.StringTable, rope);
    m_heap->insertInstance(instance);
    auto operand = Operand::fromString(instance);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(operand));

    return {};
}

lyric_runtime::Operand
lyric_runtime::HeapManager::allocateBytes(std::span<const tu_uint8> bytes)
{
    auto *instance = new BytesRef(m_preludeTables.BytesTable, bytes.data(), bytes.size());
    m_heap->insertInstance(instance);
    return Operand::fromBytes(instance);
}

lyric_runtime::Operand
lyric_runtime::HeapManager::allocateBytes(tempo_utils::Rope<tu_uint8> rope)
{
    auto *instance = new BytesRef(m_preludeTables.BytesTable, rope);
    m_heap->insertInstance(instance);
    return Operand::fromBytes(instance);
}

tempo_utils::Status
lyric_runtime::HeapManager::loadLiteralBytesOntoStack(tu_uint32 address)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);
    auto *sp = currentCoro->peekSP();
    TU_ASSERT (sp != nullptr);

    tempo_utils::Status status;
    auto literal = m_segmentManager->resolveString(sp, address, status);
    TU_RETURN_IF_NOT_OK (status);

    auto *instance = new BytesRef(m_preludeTables.BytesTable, literal);
    m_heap->insertInstance(instance);
    auto operand = Operand::fromBytes(instance);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(operand));

    return status;
}

tempo_utils::Status
lyric_runtime::HeapManager::loadBytesOntoStack(std::span<const tu_uint8> bytes)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);

    auto *instance = new BytesRef(m_preludeTables.BytesTable, bytes.data(), bytes.size());
    m_heap->insertInstance(instance);
    auto operand = Operand::fromBytes(instance);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(operand));

    return {};
}

tempo_utils::Status
lyric_runtime::HeapManager::loadBytesOntoStack(tempo_utils::Rope<tu_uint8> rope)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);

    auto *instance = new BytesRef(m_preludeTables.BytesTable, rope);
    m_heap->insertInstance(instance);
    auto operand = Operand::fromBytes(instance);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(operand));

    return {};
}

inline const lyric_runtime::VirtualTable *
status_code_to_vtable(tempo_utils::StatusCode statusCode, const lyric_runtime::PreludeTables &preludeTables)
{
    switch (statusCode) {
        case tempo_utils::StatusCode::kOk:
            return preludeTables.OkTable;
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

lyric_runtime::Operand
lyric_runtime::HeapManager::allocateStatus(const VirtualTable *vtable)
{
    auto *instance = new StatusRef(vtable);
    m_heap->insertInstance(instance);
    return Operand::fromStatus(instance);
}

lyric_runtime::Operand
lyric_runtime::HeapManager::allocateStatus(tempo_utils::StatusCode statusCode, std::string_view statusMessage)
{
    auto *messageRef = new StringRef(m_preludeTables.StringTable, statusMessage.data(), statusMessage.size());
    m_heap->insertInstance(messageRef);
    const VirtualTable *vtable = status_code_to_vtable(statusCode, m_preludeTables);
    auto *statusRef = new StatusRef(vtable, statusCode, messageRef);
    m_heap->insertInstance(statusRef);
    return Operand::fromStatus(statusRef);
}

tempo_utils::Status
lyric_runtime::HeapManager::loadStatusOntoStack(tempo_utils::StatusCode statusCode, std::string_view statusMessage)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);
    auto operand = allocateStatus(statusCode, statusMessage);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(operand));
    return {};
}

lyric_runtime::Operand
lyric_runtime::HeapManager::allocateRest(const CallCell &frame)
{
    std::vector<Operand> restArgs;
    for (int i = 0; i < frame.numRest(); i++) {
        restArgs.push_back(frame.getRest(i));
    }
    auto *instance = new RestRef(m_preludeTables.RestTable, std::move(restArgs));
    m_heap->insertInstance(instance);
    return Operand::fromRest(instance);
}

tempo_utils::Status
lyric_runtime::HeapManager::loadRestOntoStack(const CallCell &frame)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);
    auto operand = allocateRest(frame);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(operand));
    return {};
}

lyric_runtime::Operand
lyric_runtime::HeapManager::allocateProtocol(const Operand &descriptor)
{
    DescriptorEntry *protocolDescriptor;
    if (!descriptor.getDescriptor(protocolDescriptor))
        return {};
    if (protocolDescriptor->getLinkageSection() != lyric_object::LinkageSection::Protocol)
        return {};
    auto *segment = protocolDescriptor->getSegment();
    auto object = segment->getObject();
    auto walker = object.getProtocol(protocolDescriptor->getDescriptorIndex());
    lyric_common::SymbolUrl url(segment->getObjectLocation(), walker.getSymbolPath());
    auto port = walker.getPort();
    auto comm = walker.getCommunication();
    auto *protocolType = segment->lookupType(walker.getProtocolType().getDescriptorOffset());

    auto *instance = new ProtocolRef(m_preludeTables.ProtocolTable, url, protocolDescriptor, protocolType, port, comm);
    m_heap->insertInstance(instance);
    return Operand::fromProtocol(instance);
}

tempo_utils::Status
lyric_runtime::HeapManager::loadProtocolOntoStack(const Operand &descriptor)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);
    auto operand = allocateProtocol(descriptor);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(operand));
    return {};
}

lyric_runtime::Operand
lyric_runtime::HeapManager::allocateNamespace(const Operand &descriptor)
{
    DescriptorEntry *namespaceDescriptor;
    if (!descriptor.getDescriptor(namespaceDescriptor))
        return {};
    if (namespaceDescriptor->getLinkageSection() == lyric_object::LinkageSection::Namespace)
        return {};
    auto *segment = namespaceDescriptor->getSegment();
    auto object = segment->getObject();
    auto walker = object.getNamespace(namespaceDescriptor->getDescriptorIndex());
    lyric_common::SymbolUrl url(segment->getObjectLocation(), walker.getSymbolPath());
    auto *namespaceType = segment->lookupType(walker.getNamespaceType().getDescriptorOffset());

    auto *instance = new NamespaceRef(m_preludeTables.NamespaceTable, url, namespaceDescriptor, namespaceType);
    m_heap->insertInstance(instance);
    return Operand::fromNamespace(instance);
}

tempo_utils::Status
lyric_runtime::HeapManager::loadNamespaceOntoStack(const Operand &descriptor)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);
    auto operand = allocateNamespace(descriptor);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(operand));
    return {};
}

static void
set_reachable_for_task(lyric_runtime::Task *task)
{
    if (task == nullptr)
        return;

    auto *coro = task->stackfulCoroutine();

    // walk the data stack and mark all reachable instances
    auto iterator = coro->iterateData();
    lyric_runtime::Operand operand;
    while (iterator.getNext(operand)) {
        operand.setReachable();
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
            waiter->setReachable();
            waiter = next;
            next = waiter->nextWaiter();
        } while (next != nullptr && next != waiterHead);
    }

    // scan the heap and delete all unreachable instances
    m_heap->deleteUnreachable();

    return {};
}
