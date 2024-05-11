
#include <lyric_serde/patchset_change.h>
#include <lyric_serde/patchset_namespace.h>

lyric_serde::PatchsetChange::PatchsetChange(std::string_view id, ChangeAddress address, PatchsetState *state)
    : m_id(id),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

std::string
lyric_serde::PatchsetChange::getId() const
{
    return m_id;
}

lyric_serde::ChangeOperation
lyric_serde::PatchsetChange::getOperation() const
{
    return m_operation;
}

lyric_serde::ChangeAddress
lyric_serde::PatchsetChange::getAddress() const
{
    return m_address;
}

lyric_serde::OperationPath
lyric_serde::PatchsetChange::getOperationPath() const
{
    return m_path;
}

lyric_serde::ValueAddress
lyric_serde::PatchsetChange::getOperationValue() const
{
    return m_value;
}

tu_uint32
lyric_serde::PatchsetChange::getOperationIndex() const
{
    return m_index;
}

tu_int16
lyric_serde::PatchsetChange::getOperationNsKey() const
{
    return m_nsKey;
}

tu_uint32
lyric_serde::PatchsetChange::getOperationIdValue() const
{
    return m_idValue;
}

void
lyric_serde::PatchsetChange::setAppendOperation(const OperationPath &path, ValueAddress value)
{
    m_operation = ChangeOperation::AppendOperation;
    m_path = path;
    m_value = value;
}

void
lyric_serde::PatchsetChange::setInsertOperation(const OperationPath &path, tu_uint32 index, ValueAddress value)
{
    m_operation = ChangeOperation::InsertOperation;
    m_path = path;
    m_index = index;
    m_value = value;
}

void
lyric_serde::PatchsetChange::setUpdateOperation(
    const OperationPath &path,
    const char *nsUrl,
    tu_uint32 idValue,
    ValueAddress value)
{
    auto nsUrl_ = tempo_utils::Url::fromString(nsUrl);
    auto putNamespaceResult = m_state->putNamespace(nsUrl_);
    TU_ASSERT (putNamespaceResult.isResult());
    auto *ns = putNamespaceResult.getResult();

    m_operation = ChangeOperation::UpdateOperation;
    m_path = path;
    m_nsKey = ns->getAddress().getAddress();
    m_idValue = idValue;
    m_value = value;
}

void
lyric_serde::PatchsetChange::setReplaceOperation(const OperationPath &path, ValueAddress value)
{
    m_operation = ChangeOperation::ReplaceOperation;
    m_path = path;
    m_value = value;
}

void
lyric_serde::PatchsetChange::setRemoveOperation(const OperationPath &path)
{
    m_operation = ChangeOperation::InsertOperation;
    m_path = path;
}

void
lyric_serde::PatchsetChange::setEmitOperation(lyric_serde::ValueAddress value)
{
    m_operation = ChangeOperation::EmitOperation;
    m_value = value;
}
