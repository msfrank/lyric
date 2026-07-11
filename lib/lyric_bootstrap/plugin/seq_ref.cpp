
#include <absl/strings/substitute.h>

#include "seq_ref.h"

static void set_reachable(const tempo_utils::Rope<lyric_runtime::Operand> &rope)
{
    auto it = rope.iterateChunks();
    tempo_utils::RopeChunk<lyric_runtime::Operand> chunk;
    while (it.getNext(chunk)) {
        for (auto &element : chunk) {
            element.setReachable();
        }
    }
}

static void clear_reachable(const tempo_utils::Rope<lyric_runtime::Operand> &rope)
{
    auto it = rope.iterateChunks();
    tempo_utils::RopeChunk<lyric_runtime::Operand> chunk;
    while (it.getNext(chunk)) {
        for (auto &element : chunk) {
            element.clearReachable();
        }
    }
}

SeqRef::SeqRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
    m_rope = tempo_utils::Rope<lyric_runtime::Operand>();
}

SeqRef::~SeqRef()
{
    TU_LOG_VV << "free " << SeqRef::toString();
}

tu_uint64
SeqRef::getTypeTag() const
{
    return type_tag();
}

std::string
SeqRef::toString() const
{
    return absl::Substitute("<$0: SeqRef>", this);
}

OperandSeq
SeqRef::getSeq() const
{
    return m_rope;
}

void
SeqRef::setSeq(const OperandSeq &seq)
{
    m_rope = seq;
}

bool
SeqRef::isEmpty() const
{
    return m_rope.isEmpty();
}

size_t
SeqRef::numElements() const
{
    return m_rope.numElements();
}

bool
SeqRef::getElement(tu_int64 index, lyric_runtime::Operand &element) const
{
    return m_rope.getElement(index, element);
}

OperandSeq
SeqRef::slice(tu_int64 start, tu_int64 length) const
{
    return m_rope.subspan(start, length);
}

void
SeqRef::setMembersReachable()
{
    set_reachable(m_rope);
}

void
SeqRef::clearMembersReachable()
{
    clear_reachable(m_rope);
}

SeqIterator::SeqIterator(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
    tempo_utils::Rope<lyric_runtime::Operand> rope;
    auto iterator = rope.iterateElements();
    m_priv = std::make_shared<Priv>(std::move(rope), iterator);
}

SeqIterator::SeqIterator(
    const lyric_runtime::VirtualTable *vtable,
    OperandSeq rope)
    : BaseRef(vtable)
{
    auto iterator = rope.iterateElements();
    m_priv = std::make_shared<Priv>(std::move(rope), iterator);
}

tu_uint64
SeqIterator::getTypeTag() const
{
    return type_tag();
}

std::string
SeqIterator::toString() const
{
    return absl::Substitute("<$0: SeqIterator>", this);
}

bool
SeqIterator::iteratorValid()
{
    return m_priv->iterator.hasNext();
}

bool
SeqIterator::iteratorNext(lyric_runtime::Operand &cell)
{
    lyric_runtime::Operand element;
    if (!m_priv->iterator.getNext(element))
        return false;
    cell = element;
    return true;
}

void
SeqIterator::setMembersReachable()
{
    set_reachable(m_priv->rope);
}

void
SeqIterator::clearMembersReachable()
{
    clear_reachable(m_priv->rope);
}

tempo_utils::Status
seq_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();

    auto ref = state->heapManager()->allocateRef<SeqRef>(vtable);
    return currentCoro->pushData(ref);
}

tempo_utils::Status
seq_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    SeqRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    if (frame.numRest() > 0) {
        std::vector<lyric_runtime::Operand> elements(frame.numRest());
        for (tu_uint16 i = 0; i < frame.numRest(); i++) {
            elements[i] = frame.getRest(i);
        }
        auto rope = tempo_utils::make_rope(std::move(elements));
        instance->setSeq(rope);
    }

    return {};
}

tempo_utils::Status
seq_size(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    SeqRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 0);
    auto size = lyric_runtime::Operand::fromI64(instance->numElements());
    return currentCoro->pushData(size);
}

tempo_utils::Status
seq_get(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    SeqRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    tu_int64 index;
    TU_ASSERT (arg0.getI64(index));
    const auto &arg1 = frame.getArgument(1);

    lyric_runtime::Operand element;
    if (!instance->getElement(index, element)) {
        element = arg1;
    }
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(element));
    return {};
}

tempo_utils::Status
seq_append(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    SeqRef *instance;
    TU_ASSERT(receiver.castRef(instance));
    auto rope = instance->getSeq();

    std::vector<lyric_runtime::Operand> elements(1 + frame.numRest());

    TU_ASSERT (frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.isValid());
    elements[0] = arg0;

    for (tu_uint16 i = 0; i < frame.numRest(); i++) {
        elements[i + 1] = frame.getRest(i);
    }

    auto concat = rope.append(tempo_utils::make_rope(std::move(elements)));

    auto *vtable = instance->getVirtualTable();
    TU_ASSERT(vtable != nullptr);
    auto ref = state->heapManager()->allocateRef<SeqRef>(vtable);
    SeqRef *appended;
    TU_ASSERT (ref.castRef(appended));
    appended->setSeq(concat);

    return currentCoro->pushData(ref);
}

tempo_utils::Status
seq_extend(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    SeqRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    SeqRef *other;
    TU_ASSERT(arg0.castRef(other));

    auto rope = instance->getSeq();
    auto concat = rope.append(other->getSeq());

    auto *vtable = instance->getVirtualTable();
    TU_ASSERT(vtable != nullptr);
    auto ref = state->heapManager()->allocateRef<SeqRef>(vtable);
    SeqRef *extended;
    TU_ASSERT (ref.castRef(extended));
    extended->setSeq(concat);
    return currentCoro->pushData(ref);
}

tempo_utils::Status
seq_slice(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    SeqRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    tu_int64 index;
    TU_ASSERT (arg0.getI64(index));
    const auto &arg1 = frame.getArgument(1);
    tu_int64 length;
    TU_ASSERT (arg1.getI64(length));

    if (instance->isEmpty())
        return currentCoro->pushData(receiver);

    auto slice = instance->slice(index, length);

    auto *vtable = instance->getVirtualTable();
    TU_ASSERT (vtable != nullptr);
    auto ref = state->heapManager()->allocateRef<SeqRef>(vtable);
    SeqRef *sliced;
    TU_ASSERT (ref.castRef(sliced));
    sliced->setSeq(slice);
    return currentCoro->pushData(ref);
}

tempo_utils::Status
seq_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    SeqRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    lyric_runtime::Operand cell;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(cell));

    lyric_runtime::InterpreterStatus status;
    auto *vtable = state->segmentManager()->resolveClassVirtualTable(cell, status);
    if (vtable == nullptr)
        return status;

    auto rope = instance->getSeq();
    auto ref = state->heapManager()->allocateRef<SeqIterator>(vtable, rope);
    return currentCoro->pushData(ref);
}

tempo_utils::Status
seq_iterator_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();

    auto ref = state->heapManager()->allocateRef<SeqIterator>(vtable);
    return currentCoro->pushData(ref);
}

tempo_utils::Status
seq_iterator_valid(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    SeqIterator *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT(frame.numArguments() == 0);
    auto valid = lyric_runtime::Operand::fromBool(instance->iteratorValid());
    return currentCoro->pushData(valid);
}

tempo_utils::Status
seq_iterator_next(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    SeqIterator *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT(frame.numArguments() == 0);

    lyric_runtime::Operand next;
    if (!instance->iteratorNext(next)) {
        next = lyric_runtime::Operand();
    }
    return currentCoro->pushData(next);
}
