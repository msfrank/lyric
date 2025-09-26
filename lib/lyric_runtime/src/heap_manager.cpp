
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytes_ref.h>
#include <lyric_runtime/heap_manager.h>
#include <lyric_runtime/system_scheduler.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_runtime/url_ref.h>
#include <tempo_utils/big_endian.h>

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

lyric_runtime::NativeFunc
lyric_runtime::HeapManager::prepareNew(uint8_t newType, tu_uint32 address, tempo_utils::Status &status)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);
    auto *sp = currentCoro->peekSP();
    TU_ASSERT (sp != nullptr);

    // determine the constructor receiver
    auto descriptor = m_segmentManager->resolveReceiver(sp, address, status);
    if (!descriptor.isValid())
        return nullptr;

    // resolve the vtable for the receiver
    const VirtualTable *vtable;
    switch (descriptor.type) {
        case DataCellType::CLASS: {
            vtable = m_segmentManager->resolveClassVirtualTable(descriptor, status);
            if (vtable == nullptr)
                return nullptr;
            break;
        }
        case DataCellType::ENUM: {
            vtable = m_segmentManager->resolveEnumVirtualTable(descriptor, status);
            if (vtable == nullptr)
                return nullptr;
            break;
        }
        case DataCellType::INSTANCE: {
            vtable = m_segmentManager->resolveInstanceVirtualTable(descriptor, status);
            if (vtable == nullptr)
                return nullptr;
            break;
        }
        case DataCellType::STRUCT: {
            vtable = m_segmentManager->resolveStructVirtualTable(descriptor, status);
            if (vtable == nullptr)
                return nullptr;
            break;
        }
        default:
            status = InterpreterStatus::forCondition(InterpreterCondition::kInvalidReceiver,
                "invalid constructor receiver");
            return nullptr;
    }

    // find the allocator and the segment which contains it
    BytecodeSegment *segment = nullptr;
    NativeFunc allocator = nullptr;
    for (auto *curr = vtable; curr != nullptr; curr = curr->getParent()) {
        segment = curr->getSegment();
        allocator = curr->getAllocator();
        if (allocator)
            break;
    }

    if (allocator == nullptr) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing allocator");
        return nullptr;
    }

    TU_ASSERT (segment != nullptr);

    auto callSegment = segment->getSegmentIndex();

    // determine the return IP and SP so returnToCaller will restore correctly
    auto returnSegment = currentCoro->peekSP()->getSegmentIndex();
    auto returnIP = currentCoro->peekIP();

    // construct a minimal activation frame just to pass the vtable to the allocator
    CallCell allocatorFrame(INVALID_ADDRESS_U32, callSegment,
        INVALID_ADDRESS_U32, returnSegment, returnIP, true,
        currentCoro->dataStackSize(), 0, 0, 0, 0, {}, vtable);

    // the stack is now prepared to invoke the allocator
    currentCoro->pushCall(allocatorFrame, lyric_object::BytecodeIterator(), nullptr);

    return allocator;
}

bool
lyric_runtime::HeapManager::constructNew(std::vector<DataCell> &args, tempo_utils::Status &status)
{
    auto *currentCoro = m_systemScheduler->currentCoro();
    TU_ASSERT(currentCoro != nullptr);

    DataCell *receiver;
    status = currentCoro->peekData(&receiver);
    if (status.notOk())
        return false;

    const VirtualTable *vtable;
    switch (receiver->type) {
        case DataCellType::REF:
            vtable = receiver->data.ref->getVirtualTable();
            break;
        case DataCellType::STATUS:
            vtable = receiver->data.status->getVirtualTable();
            break;
        default:
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kInvalidReceiver, "invalid data cell");
            return false;
    }

    // get the ctor method
    if (vtable == nullptr) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing vtable");
        return false;
    }
    const auto *ctor = vtable->getCtor();
    if (ctor == nullptr) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "missing ctor");
        return false;
    }

    auto *segment = ctor->getSegment();
    const auto callIndex = ctor->getCallIndex();
    const auto procOffset = ctor->getProcOffset();

    // proc offset must be within the segment bytecode
    auto *bytecodeData = segment->getBytecodeData();
    auto bytecodeSize = segment->getBytecodeSize();
    if (bytecodeSize <= procOffset) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid proc offset");
        return false;
    }

    const uint8_t *header = bytecodeData + procOffset;
    const BytecodeSegment *returnSP = currentCoro->peekSP();
    const lyric_object::BytecodeIterator returnIP = currentCoro->peekIP();
    auto stackGuard = currentCoro->dataStackSize();

    // read the call proc header
    auto procSize = tempo_utils::read_u32_and_advance(header);
    auto numArguments = tempo_utils::read_u16_and_advance(header);
    auto numLocals = tempo_utils::read_u16_and_advance(header);
    auto numLexicals = tempo_utils::read_u16_and_advance(header);
    auto codeSize = procSize - 6;

    // maximum number of args is 2^16
    if (std::numeric_limits<uint16_t>::max() <= args.size()) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "too many arguments");
        return false;
    }

    // all required args must be present
    if (args.size() < numArguments) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "not enough arguments");
        return false;
    }

    auto numRest = static_cast<uint16_t>(args.size()) - numArguments;

    // construct the activation call frame
    CallCell frame(callIndex, segment->getSegmentIndex(),
        procOffset, returnSP->getSegmentIndex(), returnIP, true,
        stackGuard, numArguments, numRest, numLocals, numLexicals,
        args, *receiver);

    // import each lexical from the latest activation and push it onto the stack
    for (uint16_t i = 0; i < numLexicals; i++) {

        // read the call proc lexical
        auto activationCall = tempo_utils::read_u32_and_advance(header);
        auto targetOffset = tempo_utils::read_u32_and_advance(header);
        auto lexicalTarget = tempo_utils::read_u8_and_advance(header);
        codeSize -= 9;

        bool found = false;
        for (auto iterator = currentCoro->callsBegin(); iterator != currentCoro->callsEnd(); iterator++) {
            const auto &ancestor = *iterator;

            if (ancestor.getCallSegment() != segment->getSegmentIndex())    // lexical must exist in callee segment
                continue;
            if (ancestor.getCallIndex() != activationCall)                  // frame does not match activation call
                continue;
            switch (lexicalTarget) {
                case lyric_object::LEXICAL_ARGUMENT:
                    frame.setLexical(i, ancestor.getArgument(targetOffset));
                    break;
                case lyric_object::LEXICAL_LOCAL:
                    frame.setLexical(i, ancestor.getLocal(targetOffset));
                    break;
                default:
                    status = InterpreterStatus::forCondition(
                        InterpreterCondition::kRuntimeInvariant, "invalid lexical target");
                    return false;
            }
            found = true;                       // we found the lexical, break loop early
            break;
        }
        if (!found) {
            status = InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "missing lexical");
            return false;
        }
    }

    lyric_object::BytecodeIterator ip(header, codeSize);
    currentCoro->pushCall(frame, ip, segment);               // push the activation onto the call stack
    TU_LOG_V << "moved ip to " << ip;

    return true;
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

static void
set_reachable_for_waiter(lyric_runtime::Waiter *waiter)
{
    waiter->promise->setReachable();
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
            set_reachable_for_waiter(waiter);
            waiter = next;
            next = waiter->nextWaiter();
        } while (next != nullptr && next != waiterHead);
    }

    // scan the heap and delete all unreachable instances
    m_heap->deleteUnreachable();

    return {};
}
