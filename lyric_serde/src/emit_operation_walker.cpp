
#include <lyric_serde/emit_operation_walker.h>
#include <lyric_serde/internal/patchset_reader.h>

lyric_serde::EmitOperationWalker::EmitOperationWalker()
{
}

lyric_serde::EmitOperationWalker::EmitOperationWalker(
    std::shared_ptr<const internal::PatchsetReader> reader,
    tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_serde::EmitOperationWalker::EmitOperationWalker(const EmitOperationWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
lyric_serde::EmitOperationWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_index < m_reader->numChanges();
}

lyric_serde::ValueWalker
lyric_serde::EmitOperationWalker::getValue() const
{
    if (!isValid())
        return {};
    auto *change = m_reader->getChange(m_index);
    if (change == nullptr || change->operation_type() != lps1::Operation::EmitOperation)
        return {};
    auto *op = change->operation_as_EmitOperation();
    if (op == nullptr)
        return {};
    return ValueWalker(m_reader, op->value());
}
