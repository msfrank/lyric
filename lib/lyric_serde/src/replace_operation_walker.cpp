
#include <lyric_serde/replace_operation_walker.h>
#include <lyric_serde/internal/patchset_reader.h>

lyric_serde::ReplaceOperationWalker::ReplaceOperationWalker()
{
}

lyric_serde::ReplaceOperationWalker::ReplaceOperationWalker(
    std::shared_ptr<const internal::PatchsetReader> reader,
    tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_serde::ReplaceOperationWalker::ReplaceOperationWalker(const ReplaceOperationWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
lyric_serde::ReplaceOperationWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_index < m_reader->numChanges();
}

lyric_serde::OperationPath
lyric_serde::ReplaceOperationWalker::getPath() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::ReplaceOperation)
        return {};
    auto *op = change->operation_as_ReplaceOperation();
    if (op == nullptr || op->path() == nullptr)
        return {};
    return OperationPath::fromString(op->path()->string_view());
}

lyric_serde::ValueWalker
lyric_serde::ReplaceOperationWalker::getValue() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::ReplaceOperation)
        return {};
    auto *op = change->operation_as_ReplaceOperation();
    if (op == nullptr)
        return {};
    return ValueWalker(m_reader, op->value());
}
