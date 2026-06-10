#include <absl/strings/substitute.h>

#include <lyric_runtime/operand.h>

#include "map_key.h"
#include "map_ref.h"
#include "pair_ref.h"

template<class ValueType>
bool is_equal_value(const lyric_runtime::Operand &lhs, const lyric_runtime::Operand &rhs)
{
    ValueType l, r;
    if (!lyric_runtime::operand_to_value(lhs, l))
        return false;
    if (!lyric_runtime::operand_to_value(rhs, r))
        return false;
    return l == r;
}

inline bool
is_equal(const lyric_runtime::Operand &lhs, const lyric_runtime::Operand &rhs)
{
    if (lhs.getType() != rhs.getType())
        return false;
    switch (lhs.getType()) {
        case lyric_runtime::OperandType::Nil:
        case lyric_runtime::OperandType::Undef:
            return true;
        case lyric_runtime::OperandType::Bool:
            return is_equal_value<bool>(lhs, rhs);
        case lyric_runtime::OperandType::Int8:
            return is_equal_value<tu_int8>(lhs, rhs);
        case lyric_runtime::OperandType::Int16:
            return is_equal_value<tu_int16>(lhs, rhs);
        case lyric_runtime::OperandType::Int32:
            return is_equal_value<tu_int32>(lhs, rhs);
        case lyric_runtime::OperandType::Int64:
            return is_equal_value<tu_int64>(lhs, rhs);
        case lyric_runtime::OperandType::UInt8:
            return is_equal_value<tu_uint8>(lhs, rhs);
        case lyric_runtime::OperandType::UInt16:
            return is_equal_value<tu_uint16>(lhs, rhs);
        case lyric_runtime::OperandType::UInt32:
            return is_equal_value<tu_uint32>(lhs, rhs);
        case lyric_runtime::OperandType::UInt64:
            return is_equal_value<tu_uint64>(lhs, rhs);
        case lyric_runtime::OperandType::Float32:
            return is_equal_value<float>(lhs, rhs);
        case lyric_runtime::OperandType::Float64:
            return is_equal_value<double>(lhs, rhs);
        case lyric_runtime::OperandType::Char32:
            return is_equal_value<char32_t>(lhs, rhs);

        case lyric_runtime::OperandType::Ref: {
            lyric_runtime::BaseRef *l, *r;
            TU_ASSERT (lhs.getRef(l));
            TU_ASSERT (rhs.getRef(r));
            if (l->getVirtualTable() != r->getVirtualTable())
                return false;
            return l->equals(r);
        }
        case lyric_runtime::OperandType::String: {
            lyric_runtime::StringRef *l, *r;
            TU_ASSERT (lhs.getString(l));
            TU_ASSERT (rhs.getString(r));
            return l->equals(r);
        }
        case lyric_runtime::OperandType::Bytes: {
            lyric_runtime::BytesRef *l, *r;
            TU_ASSERT (lhs.getBytes(l));
            TU_ASSERT (rhs.getBytes(r));
            return l->equals(r);
        }
        default:
            return false;
    }
}

static IndexMapNode *
mutable_insert(
    ValueMapNode *prev,
    const lyric_runtime::Operand &key,
    const lyric_runtime::Operand &value,
    tu_uint32 hash,
    tu_uint8 bits_remaining,
    tu_uint16 trie_level)
{
    // FIXME: if tree depth becomes larger than 32 then store values in an unordered list
    TU_ASSERT (trie_level < 32);

    // if there aren't enough bits remaining to continue the insertion then perform a rehash
    if (bits_remaining < BITS_PER_LEVEL) {
        hash = absl::HashOf(MapKey{key}, trie_level);
        prev->hash = absl::HashOf(MapKey{prev->key}, trie_level);
        bits_remaining = 32;
    }

    auto *index = new IndexMapNode();
    index->type = MapNodeType::INDEX;
    index->table.fill(nullptr);
    index->refcount = 1;

    uint32_t i = hash >> (32 - BITS_PER_LEVEL);
    uint32_t prev_i = prev->hash >> (32 - BITS_PER_LEVEL);

    if (i != prev_i) {
        index->table[prev_i] = prev;                            // put the prev value in its slot

        auto *node = new ValueMapNode();                        // create a new value node
        node->type = MapNodeType::VALUE;
        node->hash = hash;                                      // store the hash in the node
        node->key = key;                                        // store the key in the node
        node->value = value;                                    // store the value in the node
        node->refcount = 1;                                     // initialize node refcount to 1
        index->table[i] = node;                                 // put the new value in its slot

        return index;
    }

    index->table[i] = mutable_insert(prev, key, value, hash << BITS_PER_LEVEL,
        bits_remaining - BITS_PER_LEVEL, trie_level + 1);
    return index;
}

static void
construct(
    IndexMapNode *index,
    const lyric_runtime::Operand &key,
    const lyric_runtime::Operand &value,
    tu_uint32 hash,
    tu_uint8 bits_remaining,
    tu_uint16 trie_level)
{
    // if there aren't enough bits remaining to continue the insertion then perform a rehash
    if (bits_remaining < BITS_PER_LEVEL) {
        hash = absl::HashOf(MapKey{key}, trie_level);
        bits_remaining = 32;
    }

    uint32_t i = hash >> (32 - BITS_PER_LEVEL);
    auto *child = index->table.at(i);

    // case 1: table value at index is nullptr, so key is not present in map
    if (child == nullptr) {
        auto *node = new ValueMapNode();                        // create a new value node
        node->type = MapNodeType::VALUE;
        node->hash = hash;                                      // store the hash in the node
        node->key = key;                                        // store the key in the node
        node->value = value;                                    // store the value in the node
        node->refcount = 1;                                     // initialize node refcount to 1
        index->table[i] = node;                                 // store the node in the index
        return;
    }

    // case 2: table value at index is a ValueMapNode
    if (child->type == MapNodeType::VALUE) {
        auto *prev = static_cast<ValueMapNode *>(child);
        if (is_equal(prev->key, key)) {
            // if the existing value node has the same key, then replace the value and hash
            prev->key = key;
            prev->value = value;
            prev->hash = hash;
        } else {
            // otherwise replace the slot with an index node containing both the existing key and the new key
            prev->hash = prev->hash << BITS_PER_LEVEL;
            index->table[i] = mutable_insert(prev, key, value, hash << BITS_PER_LEVEL,
                bits_remaining - BITS_PER_LEVEL, trie_level + 1);
        }
        return;
    }

    // case 3: table value at index is an IndexMapNode
    return construct(static_cast<IndexMapNode *>(child), key, value, hash << BITS_PER_LEVEL,
        bits_remaining - BITS_PER_LEVEL, trie_level + 1);
}

static IndexMapNode *
update(
    const IndexMapNode *curr,
    const lyric_runtime::Operand &key,
    const lyric_runtime::Operand &value,
    tu_uint32 hash,
    tu_uint8 bits_remaining,
    tu_uint16 trie_level)
{
    auto *updated = new IndexMapNode();
    updated->type = MapNodeType::INDEX;
    updated->refcount = 0;

    // if there aren't enough bits remaining to continue the search then perform a rehash
    if (bits_remaining < BITS_PER_LEVEL) {
        hash = absl::HashOf(MapKey{key}, trie_level);
        bits_remaining = 32;
    }

    tu_uint32 index = hash >> (32 - BITS_PER_LEVEL);

    // copy all slots into updated node except the node at index
    for (tu_uint32 i = 0; i < curr->table.size(); i++) {
        if (i == index)
            continue;
        auto *child = curr->table[i];
        updated->table[i] = child;
        if (child != nullptr) {
            // update the refcount for valid nodes
            child->refcount++;
        }
    }

    auto *child = curr->table.at(index);

    // case 1: table value at index is nullptr, so key is not present in map
    if (child == nullptr) {
        auto *node = new ValueMapNode();
        node->type = MapNodeType::VALUE;
        node->refcount = 1;
        node->key = key;
        node->value = value;
        node->hash = hash;
        updated->table[index] = node;
        return updated;
    }

    // case 2: table value at index is a ValueMapNode
    if (child->type == MapNodeType::VALUE) {
        auto *existing = static_cast<ValueMapNode *>(child);
        if (is_equal(existing->key, key)) {
            // if key matches existing then replace it
            auto *node = new ValueMapNode();
            node->type = MapNodeType::VALUE;
            node->refcount = 1;
            node->key = key;
            node->value = value;
            node->hash = hash;
            updated->table[index] = node;
        } else {
            // otherwise we create a new index node, then insert the existing key and new key
            auto *node = new IndexMapNode();
            node->type = MapNodeType::INDEX;
            node->refcount = 1;
            construct(node, existing->key, existing->value, existing->hash << BITS_PER_LEVEL,
                bits_remaining - BITS_PER_LEVEL, trie_level + 1);
            construct(node, key, value, hash << BITS_PER_LEVEL,
                bits_remaining - BITS_PER_LEVEL, trie_level + 1);
            updated->table[index] = node;
        }
        return updated;
    }

    // case 3: table value at index is an IndexMapNode
    auto *node = update(static_cast<IndexMapNode *>(child), key, value,hash << BITS_PER_LEVEL,
        bits_remaining - BITS_PER_LEVEL, trie_level + 1);
    TU_ASSERT (node != nullptr);
    updated->table[index] = node;
    return updated;
}

static IndexMapNode *
remove(
    const IndexMapNode *curr,
    const lyric_runtime::Operand &key,
    tu_uint32 hash,
    tu_uint8 bits_remaining,
    tu_uint16 trie_level)
{
    // if there aren't enough bits remaining to continue the search then perform a rehash
    if (bits_remaining < BITS_PER_LEVEL) {
        hash = absl::HashOf(MapKey{key}, trie_level);
        bits_remaining = 32;
    }

    tu_uint32 index = hash >> (32 - BITS_PER_LEVEL);
    auto *child = curr->table.at(index);

    // case 1: table value at index is nullptr, so key is not present in map
    if (child == nullptr)
        return nullptr;

    // case 2: table value at index is a ValueMapNode
    if (child->type == MapNodeType::VALUE) {
        auto *existing = static_cast<ValueMapNode *>(child);
        // special case: if key does not match existing value, then return nullptr
        // indicating the key is not present in map
        if (!is_equal(existing->key, key))
            return nullptr;

        // otherwise we create a copy of the index node with the value removed
        auto *removed = new IndexMapNode();
        removed->type = MapNodeType::INDEX;
        removed->refcount = 0;     // refcount is expected to be incremented by callee

        // copy all slots into removed node except the node at index
        for (tu_uint32 i = 0; i < curr->table.size(); i++) {
            if (i == index) {
                removed->table[i] = nullptr;
            } else {
                auto *node = curr->table[i];
                removed->table[i] = node;
                if (node != nullptr) {
                    node->refcount++;   // update the refcount for valid nodes
                }
            }
        }
        return removed;
    }

    // case 3: table value at index is an IndexMapNode
    child = remove(static_cast<IndexMapNode *>(child), key, hash << BITS_PER_LEVEL,
        bits_remaining - BITS_PER_LEVEL, trie_level + 1);

    // propagate the nullptr if the key is not present in the map
    if (child == nullptr)
        return nullptr;

    // create a new index node
    auto *removed = new IndexMapNode();
    removed->type = MapNodeType::INDEX;
    removed->refcount = 0;      // refcount is expected to be incremented by callee

    // copy all slots into removed node except the node at index
    for (tu_uint32 i = 0; i < curr->table.size(); i++) {
        if (i == index)
            continue;
        auto *node = curr->table[i];
        removed->table[i] = node;
        if (node != nullptr) {
            node->refcount++;   // update the refcount for valid nodes
        }
    }

    // insert the new child into the removed node
    removed->table[index] = child;
    child->refcount++;
    return removed;
}

static int
size(const MapNode *node, int count)
{
    TU_ASSERT (node != nullptr);
    if (node->type == MapNodeType::VALUE)
        return count + 1;
    for (const auto *child : static_cast<const IndexMapNode *>(node)->table) {
        if (child == nullptr)
            continue;
        if (child->type == MapNodeType::INDEX) {
            count = size(static_cast<const IndexMapNode *>(child), count);
        } else {
            count++;
        }
    }
    return count;
}

static lyric_runtime::Operand
search(
    const IndexMapNode *node,
    const lyric_runtime::Operand &key,
    tu_uint32 hash,
    tu_uint8 bits_remaining,
    tu_uint16 trie_level)
{
    // if there aren't enough bits remaining to continue the search then perform a rehash
    if (bits_remaining < BITS_PER_LEVEL) {
        hash = absl::HashOf(MapKey{key}, trie_level);
        bits_remaining = 32;
    }

    uint32_t index = hash >> (32 - BITS_PER_LEVEL);
    auto *child = node->table.at(index);

    // case 1: table value at index is nullptr, so key is not present in map
    if (child == nullptr)
        return {};

    // case 2: table value at index is a ValueMapNode
    if (child->type == MapNodeType::VALUE) {
        auto *curr = static_cast<ValueMapNode *>(child);
        // if key matches pair then return the value, otherwise key is not present in map
        return is_equal(curr->key, key)? curr->value : lyric_runtime::Operand();
    }

    // case 3: table value at index is an IndexMapNode, continue searching
    return search(static_cast<IndexMapNode *>(child), key, hash << BITS_PER_LEVEL,
        bits_remaining - BITS_PER_LEVEL, trie_level + 1);
}

static void
free_node(MapNode *node)
{
    TU_ASSERT (node != nullptr);
    node->refcount--;
    if (node->refcount > 0)
        return;

    switch (node->type) {
        case MapNodeType::INDEX: {
            auto *index = static_cast<IndexMapNode *>(node);
            for (auto *child : index->table) {
                if (child != nullptr)
                    free_node(child);
            }
            break;
        }
        case MapNodeType::VALUE:
        default:
            break;
    }
    delete node;
}

MapRef::MapRef(const lyric_runtime::VirtualTable *vtable)
    : lyric_runtime::BaseRef(vtable),
      m_node(nullptr)
{
    TU_ASSERT (vtable != nullptr);
}

MapRef::~MapRef()
{
    TU_LOG_VV << "free " << MapRef::toString();
    if (m_node) {
        free_node(m_node);
    }
}

std::string
MapRef::toString() const
{
    return absl::Substitute("<$0: MapRef>", this);
}

MapNode *
MapRef::getNode() const
{
    return m_node;
}

void
MapRef::setNode(MapNode *node)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (m_node == nullptr);
    m_node = node;
    m_node->refcount++;
}

int
MapRef::mapSize() const
{
    // case 1: map is empty
    if (m_node == nullptr)
        return 0;

    // case 2: map contains a single value node
    if (m_node->type == MapNodeType::VALUE)
        return 1;

    // case 3: map contains multiple values
    return size(static_cast<const IndexMapNode *>(m_node), 0);
}

bool
MapRef::mapContains(const lyric_runtime::Operand &key) const
{
    auto result = mapGet(key);
    return result.isValid();
}

lyric_runtime::Operand
MapRef::mapGet(const lyric_runtime::Operand &key) const
{
    // case 1: map is empty
    if (m_node == nullptr)
        return {};

    // case 2: map contains a single value node
    if (m_node->type == MapNodeType::VALUE) {
        auto *curr = static_cast<ValueMapNode *>(m_node);
        // if key matches pair then return the value, otherwise key is not present in map
        return is_equal(curr->key, key)? curr->value : lyric_runtime::Operand();
    }

    // case 3: root node is an index, so perform search
    uint32_t hash = absl::HashOf(MapKey{key});
    return search(static_cast<IndexMapNode *>(m_node), key, hash, 32, 0);
}

MapNode *
MapRef::mapUpdate(const lyric_runtime::Operand &key, const lyric_runtime::Operand &value) const
{
    // case 1: map is empty
    if (m_node == nullptr) {
        auto *node = new ValueMapNode();
        node->type = MapNodeType::VALUE;
        node->refcount = 0;
        node->key = key;
        node->value = value;
        node->hash = absl::HashOf(MapKey{key});
        return node;
    }

    // case 2: map contains a single value node
    if (m_node->type == MapNodeType::VALUE) {
        auto *curr = static_cast<ValueMapNode *>(m_node);
        // special case: if key matches existing value, then return a new value node
        if (is_equal(curr->key, key)) {
            auto *node = new ValueMapNode();
            node->type = MapNodeType::VALUE;
            node->refcount = 0;
            node->key = key;
            node->value = value;
            node->hash = absl::HashOf(MapKey{key});
            return node;
        }
        // otherwise return a new index node containing the existing entry and new entry
        auto *node = new IndexMapNode();
        node->type = MapNodeType::INDEX;
        node->refcount = 0;
        uint32_t index = curr->hash >> (32 - BITS_PER_LEVEL);
        curr->refcount++;
        node->table[index] = curr;
        auto hash = absl::HashOf(MapKey{key});
        construct(node, key, value, hash, 32, 0);
        return node;
    }

    // case 3: root node is an index, so find the appropriate slot to update
    auto hash = absl::HashOf(MapKey{key});
    return update(static_cast<IndexMapNode *>(m_node), key, value, hash, 32, 0);
}

MapNode *
MapRef::mapRemove(const lyric_runtime::Operand &key) const
{
    // case 1: map is empty
    if (m_node == nullptr)
        return nullptr;

    // case 2: map contains a single value node
    if (m_node->type == MapNodeType::VALUE) {
        auto *curr = static_cast<ValueMapNode *>(m_node);
        // special case: if key matches existing value, then return a new empty index node
        if (is_equal(curr->key, key)) {
            auto *node = new IndexMapNode();
            node->type = MapNodeType::INDEX;
            node->refcount = 0;
            return node;
        }
        // otherwise return nullptr indicating the key is not present in map
        return nullptr;
    }

    // case 3: root node is an index, so find the appropriate slot to remove
    auto hash = absl::HashOf(MapKey{key});
    return remove(static_cast<IndexMapNode *>(m_node), key, hash, 32, 0);
}

static void
set_reachable(IndexMapNode *node)
{
    TU_ASSERT (node != nullptr);
    for (auto *child : node->table) {
        if (child == nullptr)
            continue;
        if (child->type == MapNodeType::INDEX) {
            set_reachable(static_cast<IndexMapNode *>(child));
        } else {
            auto *value = static_cast<ValueMapNode *>(child);
            value->key.setReachable();
            value->value.setReachable();
        }
    }
}

void
MapRef::setMembersReachable()
{
    if (m_node == nullptr)
        return;
    if (m_node->type == MapNodeType::VALUE) {
        auto *value = static_cast<ValueMapNode *>(m_node);
        value->key.setReachable();
        value->value.setReachable();
    } else {
        set_reachable(static_cast<IndexMapNode *>(m_node));
    }
}

static void
clear_reachable(IndexMapNode *index)
{
    TU_ASSERT (index != nullptr);
    for (auto *child : index->table) {
        if (child == nullptr)
            continue;
        if (child->type == MapNodeType::INDEX) {
            clear_reachable(static_cast<IndexMapNode *>(child));
        } else {
            auto *value = static_cast<ValueMapNode *>(child);
            value->key.clearReachable();
            value->value.clearReachable();
        }
    }
}

void
MapRef::clearMembersReachable()
{
    if (m_node == nullptr)
        return;
    if (m_node->type == MapNodeType::VALUE) {
        auto *value = static_cast<ValueMapNode *>(m_node);
        value->key.clearReachable();
        value->value.clearReachable();
    } else {
        clear_reachable(static_cast<IndexMapNode *>(m_node));
    }
}

MapIterator::MapIterator(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_map(nullptr)
{
}

inline int
find_valid_index_node_child(int start, IndexMapNode *node)
{
    TU_ASSERT (node != nullptr);
    for (int i = start; i < node->table.size(); i++) {
        if (node->table.at(i) != nullptr)
            return i;
    }
    return -1;
}

inline void
build_stack(std::stack<NodePointer> &stack, MapNode *curr)
{
    TU_ASSERT (curr != nullptr);

    while (curr != nullptr) {
        switch (curr->type) {

            case MapNodeType::VALUE: {
                stack.push({curr, -1});     // invariant: NodePointer on top of stack will always have index = -1
                curr = nullptr;
                break;
            }

            case MapNodeType::INDEX: {
                auto *inode = static_cast<IndexMapNode *>(curr);
                auto index = find_valid_index_node_child(0, inode);
                TU_ASSERT (index >= 0);
                stack.push({curr, index});
                curr = inode->table.at(index);
                TU_ASSERT (curr != nullptr);
                break;
            }

            default:
                TU_UNREACHABLE();
        }
    }
}

void
init_node_pointer_stack(std::stack<NodePointer> &stack, MapRef *map)
{
    auto *node = map->m_node;
    if (node == nullptr)
        return;

    build_stack(stack, node);
}

void
update_node_pointer_stack(std::stack<NodePointer> &stack)
{
    MapNode *node = nullptr;

    while (!stack.empty()) {
        auto &nodepointer = stack.top();
        TU_ASSERT (nodepointer.node->type == MapNodeType::INDEX);
        auto *inode = static_cast<IndexMapNode *>(nodepointer.node);

        auto index = find_valid_index_node_child(nodepointer.index + 1, inode);
        if (index > nodepointer.index) {
            nodepointer.index = index;
            node = inode->table.at(index);
            break;
        }

        stack.pop();
    }

    if (stack.empty())
        return;
    build_stack(stack, node);
}

MapIterator::MapIterator(const lyric_runtime::VirtualTable *vtable, MapRef *map)
    : BaseRef(vtable),
      m_map(map)
{
    TU_ASSERT (m_map != nullptr);
    init_node_pointer_stack(m_stack, m_map);
}

std::string
MapIterator::toString() const
{
    return absl::Substitute("<$0: MapIterator>", this);
}

bool
MapIterator::iteratorValid()
{
    return !m_stack.empty();
}

bool
MapIterator::iteratorNext(lyric_runtime::Operand &cell)
{
    if (m_stack.empty())
        return false;

    // get the entry on the top of the stack
    auto &nodepointer = m_stack.top();
    TU_ASSERT (nodepointer.node->type == MapNodeType::VALUE);
    auto *vnode = static_cast<ValueMapNode *>(nodepointer.node);
    cell = vnode->value;    // FIXME: vnode should store Pair[KeyType,ValueType]
    m_stack.pop();

    // find the next entry
    update_node_pointer_stack(m_stack);
    return true;
}

void
MapIterator::setMembersReachable()
{
    m_map->setReachable();
}

void
MapIterator::clearMembersReachable()
{
    m_map->clearReachable();
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
    currentCoro->pushData(ref);

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
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *map = static_cast<MapRef *>(ref);

    if (frame.numRest() == 1) {
        auto *node = new ValueMapNode();
        node->type = MapNodeType::VALUE;
        node->refcount = 0;
        auto arg = frame.getRest(0);
        TU_ASSERT (arg.getRef(ref));
        auto *pair = static_cast<PairRef *>(ref);
        node->key = pair->pairFirst();
        node->value = pair->pairSecond();
        node->hash = absl::HashOf(MapKey{node->key});
        map->setNode(node);
    } else {
        auto *node = new IndexMapNode();
        node->type = MapNodeType::INDEX;
        node->table.fill(nullptr);
        node->refcount = 0;
        for (uint16_t i = 0; i < frame.numRest(); i++) {
            auto arg = frame.getRest(i);
            TU_ASSERT (arg.getRef(ref));
            auto *pair = static_cast<PairRef *>(ref);
            auto key = pair->pairFirst();
            auto value = pair->pairSecond();
            uint32_t hash = absl::HashOf(MapKey{key});
            construct(node, key, value, hash, 32, 0);
        }
        map->setNode(node);
    }

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
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *map = static_cast<MapRef *>(ref);

    TU_ASSERT (frame.numArguments() == 0);
    tu_int64 size = map->mapSize();
    return currentCoro->pushData(lyric_runtime::Operand::fromI64(size));
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
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *map = static_cast<MapRef *>(ref);

    TU_ASSERT (frame.numArguments() == 1);
    auto arg0 = frame.getArgument(0);

    return currentCoro->pushData(lyric_runtime::Operand::fromBool(map->mapContains(arg0)));
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
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *map = static_cast<MapRef *>(ref);

    TU_ASSERT (frame.numArguments() == 2);
    auto arg0 = frame.getArgument(0);
    auto arg1 = frame.getArgument(1);

    auto value = map->mapGet(arg0);
    if (!value.isValid()) {
        value = arg1;
    }
    currentCoro->pushData(value);
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
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *map = static_cast<MapRef *>(ref);

    TU_ASSERT (frame.numArguments() == 2);
    auto arg0 = frame.getArgument(0);
    auto arg1 = frame.getArgument(1);

    auto *node = map->mapUpdate(arg0, arg1);
    TU_ASSERT (node != nullptr);

    auto copy = state->heapManager()->allocateRef<MapRef>(map->getVirtualTable());
    TU_ASSERT (copy.getRef(ref));
    auto *mapcopy = static_cast<MapRef *>(ref);
    mapcopy->setNode(node);
    return currentCoro->pushData(copy);
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
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *map = static_cast<MapRef *>(ref);

    TU_ASSERT (frame.numArguments() == 1);
    auto arg0 = frame.getArgument(0);

    auto *node = map->mapRemove(arg0);
    if (node != nullptr) {
        // key was removed, so allocate a new Map containing the changed structure
        auto copy = state->heapManager()->allocateRef<MapRef>(map->getVirtualTable());
        TU_ASSERT (copy.getRef(ref));
        auto *mapcopy = static_cast<MapRef *>(ref);
        mapcopy->setNode(node);
        return currentCoro->pushData(copy);
    } else {
        // key was not present in the map, so return the existing reference
        return currentCoro->pushData(receiver);
    }
}

tempo_utils::Status
map_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *map = static_cast<MapRef *>(ref);

    lyric_runtime::Operand cell;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(cell));

    lyric_runtime::InterpreterStatus status;
    vtable = state->segmentManager()->resolveClassVirtualTable(cell, status);
    if (vtable == nullptr)
        return status;

    auto iterator = state->heapManager()->allocateRef<MapIterator>(vtable, map);
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
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));

    TU_ASSERT(frame.numArguments() == 0);
    return currentCoro->pushData(lyric_runtime::Operand::fromBool(ref->iteratorValid()));
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
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));

    TU_ASSERT(frame.numArguments() == 0);

    lyric_runtime::Operand next;
    if (!ref->iteratorNext(next)) {
        next = lyric_runtime::Operand();
    }
    return currentCoro->pushData(next);
}
