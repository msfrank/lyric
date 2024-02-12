
#include <lyric_serde/remove_operation_walker.h>
#include <lyric_serde/internal/patchset_reader.h>

lyric_serde::RemoveOperationWalker::RemoveOperationWalker()
{
}

lyric_serde::RemoveOperationWalker::RemoveOperationWalker(
    std::shared_ptr<const internal::PatchsetReader> reader,
    tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_serde::RemoveOperationWalker::RemoveOperationWalker(const RemoveOperationWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
lyric_serde::RemoveOperationWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_index < m_reader->numChanges();
}

lyric_serde::OperationPath
lyric_serde::RemoveOperationWalker::getPath() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::RemoveOperation)
        return {};
    auto *op = change->operation_as_RemoveOperation();
    if (op == nullptr || op->path() == nullptr)
        return {};
    return OperationPath::fromString(op->path()->string_view());
}