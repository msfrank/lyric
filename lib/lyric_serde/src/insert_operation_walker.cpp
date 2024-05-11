
#include <lyric_serde/insert_operation_walker.h>
#include <lyric_serde/internal/patchset_reader.h>

lyric_serde::InsertOperationWalker::InsertOperationWalker()
{
}

lyric_serde::InsertOperationWalker::InsertOperationWalker(
    std::shared_ptr<const internal::PatchsetReader> reader,
    tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_serde::InsertOperationWalker::InsertOperationWalker(const InsertOperationWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
lyric_serde::InsertOperationWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_index < m_reader->numChanges();
}

lyric_serde::OperationPath
lyric_serde::InsertOperationWalker::getPath() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::InsertOperation)
        return {};
    auto *op = change->operation_as_InsertOperation();
    if (op == nullptr || op->path() == nullptr)
        return {};
    return OperationPath::fromString(op->path()->string_view());
}

tu_uint32
lyric_serde::InsertOperationWalker::getIndex() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::InsertOperation)
        return {};
    auto *op = change->operation_as_InsertOperation();
    if (op == nullptr)
        return {};
    return op->index();
}

lyric_serde::ValueWalker
lyric_serde::InsertOperationWalker::getValue() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::InsertOperation)
        return {};
    auto *op = change->operation_as_InsertOperation();
    if (op == nullptr)
        return {};
    return ValueWalker(m_reader, op->value());
}
