#include <absl/strings/substitute.h>

#include <lyric_runtime/operand.h>

#include "map_ref.h"
#include "pair_ref.h"

static void set_reachable(OperandMap map)
{
    auto iterator = map.iterate();
    tempo_utils::HamtEntry<MapKey,lyric_runtime::Operand> entry;
    while (iterator.getNext(entry)) {
        auto &key = entry.entryKey();
        key.key.setReachable();
        auto &value = entry.entryValue();
        value.setReachable();
    }
}

static void clear_reachable(OperandMap map)
{
    auto iterator = map.iterate();
    tempo_utils::HamtEntry<MapKey,lyric_runtime::Operand> entry;
    while (iterator.getNext(entry)) {
        auto &key = entry.entryKey();
        key.key.clearReachable();
        auto &value = entry.entryValue();
        value.clearReachable();
    }
}

MapRef::MapRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
    TU_ASSERT (vtable != nullptr);
}

MapRef::~MapRef()
{
    TU_LOG_VV << "free " << MapRef::toString();
}

tu_uint64
MapRef::getTypeTag() const
{
    return type_tag();
}

std::string
MapRef::toString() const
{
    return absl::Substitute("<$0: MapRef>", this);
}

OperandMap
MapRef::getMap() const
{
    return m_hamt;
}

void
MapRef::setMap(const OperandMap &map)
{
    m_hamt = map;
}

size_t
MapRef::numEntries()
{
    return m_hamt.numEntries();
}

bool
MapRef::containsEntry(const MapKey &key)
{
    return m_hamt.contains(key);
}

bool
MapRef::getEntry(const MapKey &key, lyric_runtime::Operand &value)
{
    auto entry = m_hamt.get(key);
    if (!entry.isValid())
        return false;
    value = entry.entryValue();
    return true;
}

OperandMap
MapRef::update(const MapKey &key, const lyric_runtime::Operand &value)
{
    return m_hamt.update(key, value);
}

OperandMap
MapRef::remove(const MapKey &key)
{
    return m_hamt.remove(key);
}

void
MapRef::setMembersReachable()
{
    set_reachable(m_hamt);
}


void
MapRef::clearMembersReachable()
{
    clear_reachable(m_hamt);
}

MapIterator::MapIterator(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable)
{
    OperandMap hamt;
    auto iterator = hamt.iterate();
    m_priv = std::make_shared<Priv>(std::move(hamt), iterator);
}

MapIterator::MapIterator(const lyric_runtime::VirtualTable *vtable, OperandMap hamt)
    : BaseRef(vtable)
{
    auto iterator = hamt.iterate();
    m_priv = std::make_shared<Priv>(std::move(hamt), iterator);
}

tu_uint64
MapIterator::getTypeTag() const
{
    return type_tag();
}

std::string
MapIterator::toString() const
{
    return absl::Substitute("<$0: MapIterator>", this);
}

bool
MapIterator::iteratorValid()
{
    return m_priv->iterator.hasNext();
}

bool
MapIterator::iteratorNext(lyric_runtime::Operand &cell)
{
    tempo_utils::HamtEntry<MapKey,lyric_runtime::Operand> entry;
    if (!m_priv->iterator.getNext(entry))
        return false;
    cell = entry.entryValue();
    return true;
}

void
MapIterator::setMembersReachable()
{
    set_reachable(m_priv->hamt);
}

void
MapIterator::clearMembersReachable()
{
    clear_reachable(m_priv->hamt);
}

tempo_utils::Status
map_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();

    auto ref = state->heapManager()->allocateRef<MapRef>(vtable);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
map_ctor(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    MapRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    std::vector<std::pair<MapKey,lyric_runtime::Operand>> entries;
    for (tu_uint16 i = 0; i < frame.numRest(); i++) {
        auto arg = frame.getRest(i);
        PairRef *pair;
        TU_ASSERT (arg.castRef(pair));
        entries.emplace_back(MapKey(pair->pairFirst()), pair->pairSecond());
    }

    OperandMap hamt(entries);
    instance->setMap(hamt);
    return {};
}

tempo_utils::Status
map_size(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    MapRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 0);
    auto size = lyric_runtime::Operand::fromI64(instance->numEntries());
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(size));
    return {};
}

tempo_utils::Status
map_contains(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    MapRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 1);
    auto arg0 = frame.getArgument(0);
    auto contains = lyric_runtime::Operand::fromBool(instance->containsEntry(MapKey(arg0)));
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(contains));
    return {};
}

tempo_utils::Status
map_get(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    MapRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 2);
    auto arg0 = frame.getArgument(0);
    auto arg1 = frame.getArgument(1);

    lyric_runtime::Operand value;
    if (!instance->getEntry(MapKey(arg0), value)) {
        value = arg1;
    }
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(value));
    return {};
}

tempo_utils::Status
map_update(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    MapRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 2);
    auto arg0 = frame.getArgument(0);
    auto arg1 = frame.getArgument(1);

    auto hamt = instance->update(MapKey(arg0), arg1);
    auto ref = state->heapManager()->allocateRef<MapRef>(instance->getVirtualTable());
    MapRef *updated;
    TU_ASSERT (ref.castRef(updated));
    updated->setMap(hamt);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
map_remove(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    MapRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT (frame.numArguments() == 1);
    auto arg0 = frame.getArgument(0);

    auto hamt = instance->remove(MapKey(arg0));
    auto ref = state->heapManager()->allocateRef<MapRef>(instance->getVirtualTable());
    MapRef *removed;
    TU_ASSERT (ref.castRef(removed));
    removed->setMap(hamt);
    TU_RETURN_IF_NOT_OK (currentCoro->pushData(ref));
    return {};
}

tempo_utils::Status
map_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    MapRef *instance;
    TU_ASSERT(receiver.castRef(instance));

    lyric_runtime::Operand cell;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(cell));

    lyric_runtime::InterpreterStatus status;
    auto *vtable = state->segmentManager()->resolveClassVirtualTable(cell, status);
    if (vtable == nullptr)
        return status;

    auto hamt = instance->getMap();
    auto iterator = state->heapManager()->allocateRef<MapIterator>(vtable, hamt);
    return currentCoro->pushData(iterator);
}

tempo_utils::Status
map_iterator_alloc(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    TU_ASSERT(vtable != nullptr);
    auto *currentCoro = state->currentCoro();

    auto ref = state->heapManager()->allocateRef<MapIterator>(vtable);
    return currentCoro->pushData(ref);
}

tempo_utils::Status
map_iterator_valid(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    MapIterator *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT(frame.numArguments() == 0);
    auto valid = lyric_runtime::Operand::fromBool(instance->iteratorValid());
    return currentCoro->pushData(valid);
}

tempo_utils::Status
map_iterator_next(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    MapIterator *instance;
    TU_ASSERT(receiver.castRef(instance));

    TU_ASSERT(frame.numArguments() == 0);

    lyric_runtime::Operand next;
    if (!instance->iteratorNext(next)) {
        next = lyric_runtime::Operand();
    }
    return currentCoro->pushData(next);
}

size_t
KeyHash::operator()(const MapKey &key)
{
    return absl::HashOf(key);
}

bool
KeyEqual::operator()(const MapKey &lhs, const MapKey &rhs) const
{
    return lhs.key.isEqualTo(rhs.key);
}
