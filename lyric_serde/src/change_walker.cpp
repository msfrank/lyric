
#include <lyric_serde/change_walker.h>
#include <lyric_serde/internal/patchset_reader.h>

lyric_serde::ChangeWalker::ChangeWalker()
    : m_reader(),
      m_index(0)
{
}

lyric_serde::ChangeWalker::ChangeWalker(std::shared_ptr<const internal::PatchsetReader> reader, tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_serde::ChangeWalker::ChangeWalker(const ChangeWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
lyric_serde::ChangeWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_index < m_reader->numChanges();
}

tu_uint32
lyric_serde::ChangeWalker::getIndex() const
{
    return m_index;
}

std::string
lyric_serde::ChangeWalker::getId() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->id() == nullptr)
        return {};
    return change->id()->str();
}

lyric_serde::ChangeOperation
lyric_serde::ChangeWalker::getOperationType() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr)
        return {};
    switch (change->operation_type()) {
        case lps1::Operation::SyncOperation:
            return ChangeOperation::SyncOperation;
        case lps1::Operation::AppendOperation:
            return ChangeOperation::AppendOperation;
        case lps1::Operation::InsertOperation:
            return ChangeOperation::InsertOperation;
        case lps1::Operation::UpdateOperation:
            return ChangeOperation::UpdateOperation;
        case lps1::Operation::ReplaceOperation:
            return ChangeOperation::ReplaceOperation;
        case lps1::Operation::RemoveOperation:
            return ChangeOperation::RemoveOperation;
        case lps1::Operation::EmitOperation:
            return ChangeOperation::EmitOperation;
        case lps1::Operation::InvokeOperation:
            return ChangeOperation::InvokeOperation;
        case lps1::Operation::AcceptOperation:
            return ChangeOperation::AcceptOperation;
        default:
            return ChangeOperation::Invalid;
    }
}

lyric_serde::AppendOperationWalker
lyric_serde::ChangeWalker::getAppendOperation() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::AppendOperation)
        return {};
    return AppendOperationWalker(m_reader, m_index);
}

lyric_serde::EmitOperationWalker
lyric_serde::ChangeWalker::getEmitOperation() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::EmitOperation)
        return {};
    return EmitOperationWalker(m_reader, m_index);
}

lyric_serde::InsertOperationWalker
lyric_serde::ChangeWalker::getInsertOperation() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::InsertOperation)
        return {};
    return InsertOperationWalker(m_reader, m_index);
}

lyric_serde::RemoveOperationWalker
lyric_serde::ChangeWalker::getRemoveOperation() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::RemoveOperation)
        return {};
    return RemoveOperationWalker(m_reader, m_index);
}

lyric_serde::ReplaceOperationWalker
lyric_serde::ChangeWalker::getReplaceOperation() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::ReplaceOperation)
        return {};
    return ReplaceOperationWalker(m_reader, m_index);
}

lyric_serde::UpdateOperationWalker
lyric_serde::ChangeWalker::getUpdateOperation() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::UpdateOperation)
        return {};
    return UpdateOperationWalker(m_reader, m_index);
}