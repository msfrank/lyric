
#include <lyric_runtime/internal/construct_protocol.h>
#include <tempo_utils/big_endian.h>

tempo_utils::Status
lyric_runtime::internal::construct_protocol(
    tu_uint32 address,
    tu_uint8 flags,
    StackfulCoroutine *currentCoro,
    SegmentManager *segmentManager,
    HeapManager *heapManager,
    InterpreterState *state)
{
    TU_ASSERT (currentCoro != nullptr);
    TU_ASSERT (segmentManager != nullptr);
    TU_ASSERT (heapManager != nullptr);
    TU_ASSERT (state != nullptr);

    tempo_utils::Status status;
    auto existing = segmentManager->loadProtocol(address, currentCoro, status);
    TU_RETURN_IF_NOT_OK (status);

    if (existing.type != DataCellType::INVALID) {
        TU_LOG_V << "loaded protocol " << existing;
        return currentCoro->pushData(existing);
    }

    auto *sp = currentCoro->peekSP();
    TU_ASSERT (sp != nullptr);

    // resolve the protocol descriptor
    auto receiver = segmentManager->resolveDescriptor(
        sp, lyric_object::LinkageSection::Protocol, address, status);
    TU_RETURN_IF_NOT_OK (status);
    TU_ASSERT (receiver.isValid());

    auto ref = heapManager->allocateProtocol(receiver);

    // store the instance
    if (!segmentManager->storeProtocol(address, ref, currentCoro, status))
        return status;

    TU_LOG_V << "constructed protocol " << ref;
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));

    return {};
}
