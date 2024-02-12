
#include <lyric_serde/update_operation_walker.h>
#include <lyric_serde/internal/patchset_reader.h>

lyric_serde::UpdateOperationWalker::UpdateOperationWalker()
{
}

lyric_serde::UpdateOperationWalker::UpdateOperationWalker(
    std::shared_ptr<const internal::PatchsetReader> reader,
    tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_serde::UpdateOperationWalker::UpdateOperationWalker(const UpdateOperationWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
lyric_serde::UpdateOperationWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_index < m_reader->numChanges();
}

lyric_serde::OperationPath
lyric_serde::UpdateOperationWalker::getPath() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::UpdateOperation)
        return {};
    auto *op = change->operation_as_UpdateOperation();
    if (op == nullptr || op->path() == nullptr)
        return {};
    return OperationPath::fromString(op->path()->string_view());
}

tu_int16
lyric_serde::UpdateOperationWalker::getNs() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::UpdateOperation)
        return {};
    auto *op = change->operation_as_UpdateOperation();
    if (op == nullptr)
        return {};
    return op->ns();
}

tu_uint32
lyric_serde::UpdateOperationWalker::getId() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::UpdateOperation)
        return {};
    auto *op = change->operation_as_UpdateOperation();
    if (op == nullptr)
        return {};
    return op->id();
}

lyric_serde::ValueWalker
lyric_serde::UpdateOperationWalker::getValue() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::UpdateOperation)
        return {};
    auto *op = change->operation_as_UpdateOperation();
    if (op == nullptr)
        return {};
    return ValueWalker(m_reader, op->value());
}
