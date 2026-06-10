#include <queue>
#include <stack>

#include <absl/strings/substitute.h>

#include <lyric_schema/literal_schema.h>

#include "seq_ref.h"

SeqRef::SeqRef(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_node(nullptr)
{
    TU_ASSERT (vtable != nullptr);
}

static void
free_node(SeqNode *node)
{
    TU_ASSERT (node != nullptr);
    node->refcount--;
    if (node->refcount > 0)
        return;

    switch (node->type) {
        case SeqNodeType::LEAF: {
            auto *leaf = static_cast<LeafSeqNode *>(node);
            delete[] leaf->values;
            break;
        }
        case SeqNodeType::CONCAT: {
            auto *concat = static_cast<ConcatSeqNode *>(node);
            free_node(concat->left);
            free_node(concat->right);
            break;
        }
        default:
            break;
    }
    delete node;
}

SeqRef::~SeqRef()
{
    TU_LOG_VV << "free " << SeqRef::toString();
    if (m_node != nullptr) {
        free_node(m_node);
    }
}

std::string
SeqRef::toString() const
{
    return absl::Substitute("<$0: SeqRef>", this);
}

SeqNode *
SeqRef::getNode() const
{
    return m_node;
}

void
SeqRef::setNode(SeqNode *node)
{
    TU_ASSERT (node != nullptr);
    TU_ASSERT (m_node == nullptr);
    m_node = node;
    m_node->refcount++;
}

size_t
SeqRef::numElements() const
{
    return m_node != nullptr? m_node->count : 0;
}

lyric_runtime::Operand
SeqRef::seqGet(const lyric_runtime::Operand &index) const
{
    tu_int64 i;
    TU_ASSERT (index.getI64(i));

    // special case: empty seq returns invalid cell
    if (m_node == nullptr)
        return {};

    // if i is negative, then index from the end of the seq
    if (i < 0) {
        i = m_node->count + i;
    }

    // special case: index is out of range
    if (i < 0 || m_node->count <= i)
        return {};

    const SeqNode *curr = m_node;
    while (curr != nullptr) {
        if (curr->type == SeqNodeType::CONCAT) {
            auto *concat = static_cast<const ConcatSeqNode *>(curr);
            if (i < concat->left->count) {
                curr = concat->left;
            } else {
                i -= concat->left->count;
                curr = concat->right;
            }
        } else {
            auto *leaf = static_cast<const LeafSeqNode *>(curr);
            TU_ASSERT (0 <= i && i < leaf->count);
            return leaf->values[i];
        }
    }

    return {};
}

lyric_runtime::Operand
SeqRef::seqSize() const
{
    return lyric_runtime::Operand::fromI64(numElements());
}

static SeqNode *
slice(SeqNode *node, int start, int length)
{
    TU_ASSERT (node != nullptr);

    // case 1: we are slicing a leaf
    if (node->type == SeqNodeType::LEAF) {
        auto *leaf = static_cast<LeafSeqNode *>(node);

        // special case: if we use the entire leaf, then don't copy, just update the refcount
        if (start == 0 && leaf->count == length) {
            leaf->refcount++;
            return leaf;
        }

        // copy the contributing elements into a new leaf and return it
        auto *sliced = new LeafSeqNode();
        sliced->type = SeqNodeType::LEAF;
        sliced->refcount = 0;
        sliced->count = length;
        sliced->values = new lyric_runtime::Operand[sliced->count];
        for (int i = 0; i < length; i++) {
            sliced->values[i] = leaf->values[start + 1];
        }
        return sliced;
    }

    auto *concat = static_cast<ConcatSeqNode *>(node);

    // case 2: only elements in the left branch contribute to the slice
    if (start + length <= concat->left->count)
        return slice(concat->left, start, length);

    // case 3: no elements in the left branch contribute to the slice
    if (concat->left->count <= start)
        return slice(concat->right, start - concat->left->count, length);

    // case 4: left and right branches contribute to the slice, so create a new concat node
    auto *sliced = new ConcatSeqNode();
    sliced->type = SeqNodeType::CONCAT;
    sliced->refcount = 0;

    auto *left = slice(concat->left, start, concat->left->count - start);
    left->refcount++;
    int lheight = left->type == SeqNodeType::CONCAT? static_cast<ConcatSeqNode *>(left)->height : 1;

    auto *right = slice(concat->right, 0, length - (concat->left->count - start));
    right->refcount++;
    int rheight = right->type == SeqNodeType::CONCAT? static_cast<ConcatSeqNode *>(right)->height : 1;

    sliced->left = left;
    sliced->right = right;
    sliced->count = length;
    sliced->height = std::max(lheight, rheight) + 1;

    return sliced;
}

SeqNode *
SeqRef::seqSlice(const lyric_runtime::Operand &start, const lyric_runtime::Operand &length) const
{
    if (m_node == nullptr)
        return nullptr;

    tu_int64 s, l;
    TU_ASSERT (start.getI64(s));
    TU_ASSERT (length.getI64(l));

    if (s < 0) {                            // negative start indicates reverse slice starting from end of seq
        if (m_node->count <= l) {
            return m_node;                  // if length is equal or larger than seq size, then return existing seq
        } else {
            s = m_node->count - l;          // otherwise recalculate start index
        }
    } else {                                // positive start indicates slice starting from beginning of seq
        if (m_node->count <= s)
            return nullptr;                 // if start is past end of seq, then indicate empty slice
        if (m_node->count <= s + l)
            l = m_node->count - s;          // shrink length if necessary so it doesn't run past the end of seq
    }

    return slice(m_node, s, l);
}

static void
set_reachable(SeqNode *node)
{
    TU_ASSERT (node != nullptr);
    if (node->type == SeqNodeType::LEAF) {
        auto *leaf = static_cast<const LeafSeqNode *>(node);
        for (int i = 0; i < leaf->count; i++) {
            auto &cell = leaf->values[i];
            cell.setReachable();
        }
    } else {
        auto *concat = static_cast<const ConcatSeqNode *>(node);
        set_reachable(concat->left);
        set_reachable(concat->right);
    }
}

void
SeqRef::setMembersReachable()
{
    if (m_node == nullptr)
        return;
    set_reachable(m_node);
}

static void
clear_reachable(SeqNode *node)
{
    TU_ASSERT (node != nullptr);
    if (node->type == SeqNodeType::LEAF) {
        auto *leaf = static_cast<const LeafSeqNode *>(node);
        for (int i = 0; i < leaf->count; i++) {
            auto &cell = leaf->values[i];
            cell.clearReachable();
        }
    } else {
        auto *concat = static_cast<const ConcatSeqNode *>(node);
        set_reachable(concat->left);
        set_reachable(concat->right);
    }
}

void
SeqRef::clearMembersReachable()
{
    if (m_node == nullptr)
        return;
    clear_reachable(m_node);
}

SeqIterator::SeqIterator(const lyric_runtime::VirtualTable *vtable)
    : BaseRef(vtable),
      m_curr(0),
      m_size(0),
      m_seq(nullptr)
{
}

SeqIterator::SeqIterator(const lyric_runtime::VirtualTable *vtable, SeqRef *seq)
    : BaseRef(vtable),
      m_curr(0),
      m_size(0),
      m_seq(seq)
{
    TU_ASSERT (m_seq != nullptr);
    m_size = m_seq->numElements();
}

std::string
SeqIterator::toString() const
{
    return absl::Substitute("<$0: SeqIterator>", this);
}

bool
SeqIterator::iteratorValid()
{
    return m_curr < m_size;
}

bool
SeqIterator::iteratorNext(lyric_runtime::Operand &cell)
{
    if (m_curr < m_size) {
        cell = m_seq->seqGet(lyric_runtime::Operand::fromI64(static_cast<int64_t>(m_curr++)));
        return true;
    }
    cell = lyric_runtime::Operand();
    return false;
}

void
SeqIterator::setMembersReachable()
{
    m_seq->setReachable();
}

void
SeqIterator::clearMembersReachable()
{
    m_seq->clearReachable();
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
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *seq = static_cast<SeqRef *>(ref);

    if (frame.numRest() > 0) {
        auto *node = new LeafSeqNode();
        node->type = SeqNodeType::LEAF;
        node->refcount = 0;
        node->count = frame.numRest();
        node->values = new lyric_runtime::Operand[node->count];
        for (uint16_t i = 0; i < frame.numRest(); i++) {
            node->values[i] = frame.getRest(i);
        }
        seq->setNode(node);
    }

    return {};
}

tempo_utils::Status
seq_size(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *seq = static_cast<SeqRef *>(ref);

    TU_ASSERT (frame.numArguments() == 0);
    return currentCoro->pushData(seq->seqSize());
}

tempo_utils::Status
seq_get(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *seq = static_cast<SeqRef *>(ref);

    TU_ASSERT (frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    const auto &arg1 = frame.getArgument(1);

    auto value = seq->seqGet(arg0);
    if (!value.isValid()) {
        value = arg1;
    }
    currentCoro->pushData(value);
    return {};
}

tempo_utils::Status
seq_append(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *seq = static_cast<SeqRef *>(ref);

    TU_ASSERT (frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.isValid());

    auto *left = seq->getNode();
    TU_ASSERT (left != nullptr);
    left->refcount++;
    int lheight = left->type == SeqNodeType::CONCAT? static_cast<ConcatSeqNode *>(left)->height : 1;

    auto *right = new LeafSeqNode();
    right->type = SeqNodeType::LEAF;
    right->refcount = 1;
    right->count = frame.numRest() + 1;
    right->values = new lyric_runtime::Operand[right->count];
    right->values[0] = arg0;
    for (uint16_t i = 0; i < frame.numRest(); i++) {
        right->values[i+1] = frame.getRest(i);
    }

    vtable = seq->getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto *node = new ConcatSeqNode();
    node->type = SeqNodeType::CONCAT;
    node->refcount = 0;
    node->left = left;
    node->right = right;
    node->count = left->count + right->count;
    node->height = lheight + 1;

    auto copy = state->heapManager()->allocateRef<SeqRef>(vtable);
    TU_ASSERT (copy.getRef(ref));
    auto *seqcopy = static_cast<SeqRef *>(ref);
    seqcopy->setNode(node);
    return currentCoro->pushData(copy);
}

tempo_utils::Status
seq_extend(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *seq = static_cast<SeqRef *>(ref);

    TU_ASSERT (frame.numArguments() == 1);
    const auto &arg0 = frame.getArgument(0);
    TU_ASSERT(arg0.getRef(ref));
    auto *other = static_cast<SeqRef *>(ref);

    auto *left = seq->getNode();
    TU_ASSERT (left != nullptr);
    left->refcount++;
    int lheight = left->type == SeqNodeType::CONCAT? static_cast<ConcatSeqNode *>(left)->height : 1;

    auto *right = other->getNode();
    TU_ASSERT (right != nullptr);
    right->refcount++;
    int rheight = right->type == SeqNodeType::CONCAT? static_cast<ConcatSeqNode *>(right)->height : 1;

    vtable = seq->getVirtualTable();
    TU_ASSERT(vtable != nullptr);

    auto *node = new ConcatSeqNode();
    node->type = SeqNodeType::CONCAT;
    node->refcount = 1;
    node->left = left;
    node->right = right;
    node->count = left->count + right->count;
    node->height = std::max(lheight, rheight) + 1;

    auto copy = state->heapManager()->allocateRef<SeqRef>(vtable);
    TU_ASSERT (copy.getRef(ref));
    auto *seqcopy = static_cast<SeqRef *>(ref);
    seqcopy->setNode(node);
    return currentCoro->pushData(copy);
}

tempo_utils::Status
seq_slice(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *seq = static_cast<SeqRef *>(ref);

    TU_ASSERT (frame.numArguments() == 2);
    const auto &arg0 = frame.getArgument(0);
    const auto &arg1 = frame.getArgument(1);

    auto *node = seq->seqSlice(arg0, arg1);
    if (node != nullptr) {
        vtable = seq->getVirtualTable();
        node->refcount++;
        auto copy = state->heapManager()->allocateRef<SeqRef>(vtable);
        TU_ASSERT (copy.getRef(ref));
        auto *seqcopy = static_cast<SeqRef *>(ref);
        seqcopy->setNode(node);
        return currentCoro->pushData(copy);
    } else {
        return currentCoro->pushData(receiver);
    }
}

tempo_utils::Status
seq_iterate(
    lyric_runtime::BytecodeInterpreter *interp,
    lyric_runtime::InterpreterState *state,
    const lyric_runtime::VirtualTable *vtable)
{
    auto *currentCoro = state->currentCoro();

    auto &frame = currentCoro->currentCallOrThrow();
    auto receiver = frame.getReceiver();
    lyric_runtime::BaseRef *ref;
    TU_ASSERT(receiver.getRef(ref));
    auto *seq = static_cast<SeqRef *>(ref);

    lyric_runtime::Operand cell;
    TU_RETURN_IF_NOT_OK (currentCoro->popData(cell));

    lyric_runtime::InterpreterStatus status;
    vtable = state->segmentManager()->resolveClassVirtualTable(cell, status);
    if (vtable == nullptr)
        return status;

    auto iterator = state->heapManager()->allocateRef<SeqIterator>(vtable, seq);
    return currentCoro->pushData(iterator);
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
seq_iterator_next(
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
