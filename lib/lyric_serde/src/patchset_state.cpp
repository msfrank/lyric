
#include <absl/container/flat_hash_map.h>
#include <unicode/umachine.h>
#include <unicode/ustring.h>

#include <lyric_serde/generated/patchset.h>
#include <lyric_serde/patchset_change.h>
#include <lyric_serde/patchset_namespace.h>
#include <lyric_serde/patchset_value.h>
#include <lyric_serde/patchset_state.h>
#include <lyric_serde/serde_types.h>
#include <tempo_utils/memory_bytes.h>

lyric_serde::PatchsetState::PatchsetState()
{
}

bool
lyric_serde::PatchsetState::hasNamespace(const tempo_utils::Url &nsUrl) const
{
    return getNamespace(nsUrl) != nullptr;
}

lyric_serde::PatchsetNamespace *
lyric_serde::PatchsetState::getNamespace(int index) const
{
    if (0 <= index && std::cmp_less(index, m_patchsetNamespaces.size()))
        return m_patchsetNamespaces.at(index);
    return nullptr;
}

lyric_serde::PatchsetNamespace *
lyric_serde::PatchsetState::getNamespace(const tempo_utils::Url &nsUrl) const
{
    if (!m_namespaceIndex.contains(nsUrl))
        return nullptr;
    return getNamespace(m_namespaceIndex.at(nsUrl));
}

tempo_utils::Result<lyric_serde::PatchsetNamespace *>
lyric_serde::PatchsetState::putNamespace(const tempo_utils::Url &nsUrl)
{
    if (m_namespaceIndex.contains(nsUrl)) {
        auto index = m_namespaceIndex.at(nsUrl);
        TU_ASSERT (0 <= index && std::cmp_less(index, m_patchsetNamespaces.size()));
        return m_patchsetNamespaces.at(index);
    }
    NamespaceAddress address(m_patchsetNamespaces.size());
    auto *ns = new PatchsetNamespace(nsUrl, address, this);
    m_patchsetNamespaces.push_back(ns);
    m_namespaceIndex[nsUrl] = static_cast<tu_int16>(address.getAddress());
    return ns;
}

std::vector<lyric_serde::PatchsetNamespace *>::const_iterator
lyric_serde::PatchsetState::namespacesBegin() const
{
    return m_patchsetNamespaces.cbegin();
}

std::vector<lyric_serde::PatchsetNamespace *>::const_iterator
lyric_serde::PatchsetState::namespacesEnd() const
{
    return m_patchsetNamespaces.cend();
}

int
lyric_serde::PatchsetState::numNamespaces() const
{
    return m_patchsetNamespaces.size();
}

tempo_utils::Result<lyric_serde::PatchsetValue *>
lyric_serde::PatchsetState::appendValue(const tempo_schema::AttrValue &intrinsicValue)
{
    ValueAddress address(m_patchsetValues.size());
    auto variant = std::make_unique<ValueVariant>();
    variant->type = VariantType::Intrinsic;
    variant->intrinsic = intrinsicValue;
    auto *value = new PatchsetValue(std::move(variant), address, this);
    m_patchsetValues.push_back(value);
    return value;
}

tempo_utils::Result<lyric_serde::PatchsetValue *>
lyric_serde::PatchsetState::appendValue(
    const char *nsUrl,
    tu_uint32 idValue,
    ValueAddress attrValue)
{
    auto nsUrl_ = tempo_utils::Url::fromString(nsUrl);

    if (!m_namespaceIndex.contains(nsUrl_)) {
        auto putNamespaceResult = putNamespace(nsUrl_);
        if (putNamespaceResult.isStatus())
            return putNamespaceResult.getStatus();
    }
    auto nsKey = m_namespaceIndex.at(nsUrl_);

    ValueAddress address(m_patchsetValues.size());
    auto variant = std::make_unique<ValueVariant>();
    variant->type = VariantType::Attr;
    variant->attr.ns = nsKey;
    variant->attr.id = idValue;
    variant->attr.value = attrValue;
    auto *value = new PatchsetValue(std::move(variant), address, this);

    m_patchsetValues.push_back(value);
    return value;
}

tempo_utils::Result<lyric_serde::PatchsetValue *>
lyric_serde::PatchsetState::appendValue(
    const char *nsUrl,
    tu_uint32 idValue,
    const std::vector<ValueAddress> &elementChildren)
{
    auto nsUrl_ = tempo_utils::Url::fromString(nsUrl);

    if (!m_namespaceIndex.contains(nsUrl_)) {
        auto putNamespaceResult = putNamespace(nsUrl_);
        if (putNamespaceResult.isStatus())
            return putNamespaceResult.getStatus();
    }
    auto nsKey = m_namespaceIndex.at(nsUrl_);

    ValueAddress address(m_patchsetValues.size());
    auto variant = std::make_unique<ValueVariant>();
    variant->type = VariantType::Element;
    variant->element.ns = nsKey;
    variant->element.id = idValue;
    variant->element.children = elementChildren;
    auto *value = new PatchsetValue(std::move(variant), address, this);

    m_patchsetValues.push_back(value);
    return value;
}

lyric_serde::PatchsetValue *
lyric_serde::PatchsetState::getValue(int index) const
{
    if (0 <= index && std::cmp_less(index, m_patchsetValues.size()))
        return m_patchsetValues.at(index);
    return nullptr;
}

std::vector<lyric_serde::PatchsetValue *>::const_iterator
lyric_serde::PatchsetState::valuesBegin() const
{
    return m_patchsetValues.cbegin();
}

std::vector<lyric_serde::PatchsetValue *>::const_iterator
lyric_serde::PatchsetState::valuesEnd() const
{
    return m_patchsetValues.cend();
}

int
lyric_serde::PatchsetState::numValues() const
{
    return m_patchsetValues.size();
}

bool
lyric_serde::PatchsetState::hasChange(std::string_view id) const
{
    return getChange(id) != nullptr;
}

lyric_serde::PatchsetChange *
lyric_serde::PatchsetState::getChange(int index) const
{
    if (0 <= index && std::cmp_less(index, m_patchsetChanges.size()))
        return m_patchsetChanges.at(index);
    return nullptr;
}

lyric_serde::PatchsetChange *
lyric_serde::PatchsetState::getChange(std::string_view id) const
{
    if (!m_idIndex.contains(id))
        return nullptr;
    return getChange(m_idIndex.at(id));
}

tempo_utils::Result<lyric_serde::PatchsetChange *>
lyric_serde::PatchsetState::appendChange(std::string_view id)
{
    if (m_idIndex.contains(id))
        return SerdeStatus::forCondition(
            SerdeCondition::kDuplicateChange, "change already exists with id {}", id);

    ChangeAddress address(m_patchsetChanges.size());
    auto *change = new PatchsetChange(id, address, this);
    m_patchsetChanges.push_back(change);
    m_idIndex[id] = address.getAddress();
    return change;
}

std::vector<lyric_serde::PatchsetChange *>::const_iterator
lyric_serde::PatchsetState::changesBegin() const
{
    return m_patchsetChanges.cbegin();
}

std::vector<lyric_serde::PatchsetChange *>::const_iterator
lyric_serde::PatchsetState::changesEnd() const
{
    return m_patchsetChanges.cend();
}

int
lyric_serde::PatchsetState::numChanges() const
{
    return m_patchsetChanges.size();
}

static std::pair<lps1::Value,flatbuffers::Offset<void>>
serialize_value(flatbuffers::FlatBufferBuilder &buffer, const tempo_schema::AttrValue &value)
{
    switch (value.getType()) {
        case tempo_schema::ValueType::Nil: {
            auto type = lps1::Value::TrueFalseNilValue;
            auto offset = lps1::CreateTrueFalseNilValue(buffer, lps1::TrueFalseNil::Nil).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::Bool: {
            auto type = lps1::Value::TrueFalseNilValue;
            auto tfn = value.getBool()? lps1::TrueFalseNil::True : lps1::TrueFalseNil::False;
            auto offset = lps1::CreateTrueFalseNilValue(buffer, tfn).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::Int64: {
            auto type = lps1::Value::Int64Value;
            auto offset = lps1::CreateInt64Value(buffer, value.getInt64()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::Float64: {
            auto type = lps1::Value::Float64Value;
            auto offset = lps1::CreateFloat64Value(buffer, value.getFloat64()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::UInt64: {
            auto type = lps1::Value::UInt64Value;
            auto offset = lps1::CreateUInt64Value(buffer, value.getUInt64()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::UInt32: {
            auto type = lps1::Value::UInt32Value;
            auto offset = lps1::CreateUInt32Value(buffer, value.getUInt32()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::UInt16: {
            auto type = lps1::Value::UInt16Value;
            auto offset = lps1::CreateUInt16Value(buffer, value.getUInt16()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::UInt8: {
            auto type = lps1::Value::UInt8Value;
            auto offset = lps1::CreateUInt8Value(buffer, value.getUInt8()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::String: {
            auto type = lps1::Value::StringValue;
            auto offset = lps1::CreateStringValue(buffer, buffer.CreateSharedString(value.stringView())).Union();
            return {type, offset};
        }
        default:
            TU_UNREACHABLE();
    }
}

tempo_utils::Result<lyric_serde::LyricPatchset>
lyric_serde::PatchsetState::toPatchset() const
{
    flatbuffers::FlatBufferBuilder buffer;

    std::vector<flatbuffers::Offset<lps1::NamespaceDescriptor>> namespaces_vector;
    std::vector<flatbuffers::Offset<lps1::ValueDescriptor>> values_vector;
    std::vector<flatbuffers::Offset<lps1::ChangeDescriptor>> changes_vector;

    // serialize namespaces
    for (const auto *ns : m_patchsetNamespaces) {
        auto fb_nsUrl = buffer.CreateString(ns->getNsUrl().toString());
        namespaces_vector.push_back(lps1::CreateNamespaceDescriptor(buffer, fb_nsUrl));
    }
    auto fb_namespaces = buffer.CreateVector(namespaces_vector);

    // serialize values
    for (const auto *value : m_patchsetValues) {
        const auto *variant = value->getValue();

        std::pair<lps1::Value, flatbuffers::Offset<void>> p;
        switch (variant->type) {
            case VariantType::Intrinsic: {
                p = serialize_value(buffer, variant->intrinsic);
                break;
            }
            case VariantType::Attr: {
                auto attrValue = lps1::CreateAttrValue(buffer, variant->attr.ns, variant->attr.id,
                    variant->attr.value.getAddress());
                p = {lps1::Value::AttrValue, attrValue.Union()};
                break;
            }
            case VariantType::Element: {
                std::vector<tu_uint32> children;
                for (const auto &childAddress : variant->element.children) {
                    children.push_back(childAddress.getAddress());
                }
                auto attrValue = lps1::CreateElementValue(buffer, variant->element.ns, variant->element.id,
                    buffer.CreateVector(children));
                p = {lps1::Value::ElementValue, attrValue.Union()};
                break;
            }
            default:
                return SerdeStatus::forCondition(SerdeCondition::kSerdeInvariant, "invalid value");
        }
        values_vector.push_back(lps1::CreateValueDescriptor(buffer, p.first, p.second));
    }
    auto fb_values = buffer.CreateVector(values_vector);

    // serialize changes
    for (const auto *change : m_patchsetChanges) {
        lps1::Operation operationType;
        flatbuffers::Offset<void> operationUnion;

        switch (change->getOperation()) {
            case ChangeOperation::AppendOperation: {
                operationType = lps1::Operation::AppendOperation;
                auto operation = lps1::CreateAppendOperation(buffer,
                    buffer.CreateSharedString(change->getOperationPath().toString()),
                    change->getOperationValue().getAddress());
                operationUnion = operation.Union();
                break;
            }
            case ChangeOperation::EmitOperation: {
                operationType = lps1::Operation::EmitOperation;
                auto operation = lps1::CreateEmitOperation(buffer, change->getOperationValue().getAddress());
                operationUnion = operation.Union();
                break;
            }
            case ChangeOperation::InsertOperation: {
                operationType = lps1::Operation::InsertOperation;
                auto operation = lps1::CreateInsertOperation(buffer,
                    buffer.CreateSharedString(change->getOperationPath().toString()),
                    change->getOperationIndex(), change->getOperationValue().getAddress());
                operationUnion = operation.Union();
                break;
            }
            case ChangeOperation::UpdateOperation: {
                operationType = lps1::Operation::UpdateOperation;
                auto operation = lps1::CreateUpdateOperation(buffer,
                    buffer.CreateSharedString(change->getOperationPath().toString()),
                    change->getOperationNsKey(), change->getOperationIdValue(),
                    change->getOperationValue().getAddress());
                operationUnion = operation.Union();
                break;
            }
            case ChangeOperation::ReplaceOperation: {
                operationType = lps1::Operation::ReplaceOperation;
                auto operation = lps1::CreateReplaceOperation(buffer,
                    buffer.CreateSharedString(change->getOperationPath().toString()),
                    change->getOperationValue().getAddress());
                operationUnion = operation.Union();
                break;
            }
            case ChangeOperation::RemoveOperation: {
                operationType = lps1::Operation::RemoveOperation;
                auto operation = lps1::CreateRemoveOperation(buffer,
                    buffer.CreateSharedString(change->getOperationPath().toString()));
                operationUnion = operation.Union();
                break;
            }
            default:
                return SerdeStatus::forCondition(
                    SerdeCondition::kSerdeInvariant, "invalid operation type");
        }

        changes_vector.push_back(lps1::CreateChangeDescriptor(buffer, buffer.CreateString(change->getId()),
            operationType, operationUnion));
    }
    auto fb_changes = buffer.CreateVector(changes_vector);

    // build package from buffer
    lps1::PatchsetBuilder patchsetBuilder(buffer);

    patchsetBuilder.add_abi(lps1::PatchsetVersion::Version1);
    patchsetBuilder.add_namespaces(fb_namespaces);
    patchsetBuilder.add_values(fb_values);
    patchsetBuilder.add_changes(fb_changes);

    // serialize package and mark the buffer as finished
    auto patchset = patchsetBuilder.Finish();
    buffer.Finish(patchset, lps1::PatchsetIdentifier());

    // copy the flatbuffer into our own byte array and instantiate package
    auto bytes = tempo_utils::MemoryBytes::copy(buffer.GetBufferSpan());
    return LyricPatchset(bytes);
}
