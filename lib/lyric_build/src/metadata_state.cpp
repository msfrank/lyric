
#include <absl/container/flat_hash_map.h>

#include <lyric_build/build_result.h>
#include <lyric_build/build_types.h>
#include <lyric_build/generated/metadata.h>
#include <lyric_build/metadata_attr.h>
#include <lyric_build/metadata_namespace.h>
#include <lyric_build/metadata_state.h>
#include <tempo_utils/memory_bytes.h>

lyric_build::MetadataState::MetadataState()
{
}

tempo_utils::Status
lyric_build::MetadataState::load(const LyricMetadata &metadata)
{
    if (!m_metadataAttrs.empty() || !m_metadataNamespaces.empty())
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "metadata state is already initialized");

    auto root = metadata.getMetadata();
    for (tu_uint32 i = 0; i < root.numAttrs(); i++) {
        auto attr = root.getAttr(i);
        if (!attr.second.isValid())
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "invalid metadata attr");
        const auto &key = attr.first;
        const auto &value = attr.second;
        auto nsUrl = tempo_utils::Url::fromString(attr.first.ns);
        MetadataNamespace *ns;
        TU_ASSIGN_OR_RETURN (ns, putNamespace(nsUrl));
        AttrId id(ns->getAddress(), key.id);
        TU_RETURN_IF_STATUS (appendAttr(id, value));
    }
    return {};
}

bool
lyric_build::MetadataState::hasNamespace(const tempo_utils::Url &nsUrl) const
{
    return getNamespace(nsUrl) != nullptr;
}

lyric_build::MetadataNamespace *
lyric_build::MetadataState::getNamespace(int index) const
{
    if (0 <= index && std::cmp_less(index, m_metadataNamespaces.size()))
        return m_metadataNamespaces.at(index);
    return nullptr;
}

lyric_build::MetadataNamespace *
lyric_build::MetadataState::getNamespace(const tempo_utils::Url &nsUrl) const
{
    if (!m_namespaceIndex.contains(nsUrl))
        return nullptr;
    return getNamespace(m_namespaceIndex.at(nsUrl));
}

tempo_utils::Result<lyric_build::MetadataNamespace *>
lyric_build::MetadataState::putNamespace(const tempo_utils::Url &nsUrl)
{
    if (m_namespaceIndex.contains(nsUrl)) {
        auto index = m_namespaceIndex.at(nsUrl);
        TU_ASSERT (0 <= index && index < m_metadataNamespaces.size());
        return m_metadataNamespaces.at(index);
    }
    NamespaceAddress address(m_metadataNamespaces.size());
    auto *ns = new MetadataNamespace(nsUrl, address, this);
    m_metadataNamespaces.push_back(ns);
    m_namespaceIndex[nsUrl] = address.getAddress();
    return ns;
}

std::vector<lyric_build::MetadataNamespace *>::const_iterator
lyric_build::MetadataState::namespacesBegin() const
{
    return m_metadataNamespaces.cbegin();
}

std::vector<lyric_build::MetadataNamespace *>::const_iterator
lyric_build::MetadataState::namespacesEnd() const
{
    return m_metadataNamespaces.cend();
}

int
lyric_build::MetadataState::numNamespaces() const
{
    return m_metadataNamespaces.size();
}

tempo_utils::Result<lyric_build::MetadataAttr *>
lyric_build::MetadataState::appendAttr(AttrId id, const tempo_schema::AttrValue &value)
{
    AttrAddress address(m_metadataAttrs.size());
    auto *attr = new MetadataAttr(id, value, address, this);
    m_metadataAttrs.push_back(attr);
    return attr;
}

lyric_build::MetadataAttr *
lyric_build::MetadataState::getAttr(int index) const
{
    if (0 <= index && std::cmp_less(index, m_metadataAttrs.size()))
        return m_metadataAttrs.at(index);
    return nullptr;
}

std::vector<lyric_build::MetadataAttr *>::const_iterator
lyric_build::MetadataState::attrsBegin() const
{
    return m_metadataAttrs.cbegin();
}

std::vector<lyric_build::MetadataAttr *>::const_iterator
lyric_build::MetadataState::attrsEnd() const
{
    return m_metadataAttrs.cend();
}

int
lyric_build::MetadataState::numAttrs() const
{
    return m_metadataAttrs.size();
}

static std::pair<lbm1::Value,flatbuffers::Offset<void>>
serialize_value(flatbuffers::FlatBufferBuilder &buffer, const tempo_schema::AttrValue &value)
{
    switch (value.getType()) {
        case tempo_schema::ValueType::Nil: {
            auto type = lbm1::Value::TrueFalseNilValue;
            auto offset = lbm1::CreateTrueFalseNilValue(buffer, lbm1::TrueFalseNil::Nil).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::Bool: {
            auto type = lbm1::Value::TrueFalseNilValue;
            auto tfn = value.getBool()? lbm1::TrueFalseNil::True : lbm1::TrueFalseNil::False;
            auto offset = lbm1::CreateTrueFalseNilValue(buffer, tfn).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::Int64: {
            auto type = lbm1::Value::Int64Value;
            auto offset = lbm1::CreateInt64Value(buffer, value.getInt64()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::Float64: {
            auto type = lbm1::Value::Float64Value;
            auto offset = lbm1::CreateFloat64Value(buffer, value.getFloat64()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::UInt64: {
            auto type = lbm1::Value::UInt64Value;
            auto offset = lbm1::CreateUInt64Value(buffer, value.getUInt64()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::UInt32: {
            auto type = lbm1::Value::UInt32Value;
            auto offset = lbm1::CreateUInt32Value(buffer, value.getUInt32()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::UInt16: {
            auto type = lbm1::Value::UInt16Value;
            auto offset = lbm1::CreateUInt16Value(buffer, value.getUInt16()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::UInt8: {
            auto type = lbm1::Value::UInt8Value;
            auto offset = lbm1::CreateUInt8Value(buffer, value.getUInt8()).Union();
            return {type, offset};
        }
        case tempo_schema::ValueType::String: {
            auto type = lbm1::Value::StringValue;
            auto offset = lbm1::CreateStringValue(buffer, buffer.CreateSharedString(value.stringView())).Union();
            return {type, offset};
        }
        default:
            TU_UNREACHABLE();
    }
}

tempo_utils::Result<lyric_build::LyricMetadata>
lyric_build::MetadataState::toMetadata() const
{
    flatbuffers::FlatBufferBuilder buffer;

    std::vector<flatbuffers::Offset<lbm1::NamespaceDescriptor>> namespaces_vector;
    std::vector<flatbuffers::Offset<lbm1::AttrDescriptor>> attrs_vector;

    // serialize namespaces
    for (const auto *ns : m_metadataNamespaces) {
        auto fb_nsUrl = buffer.CreateString(ns->getNsUrl().toString());
        namespaces_vector.push_back(lbm1::CreateNamespaceDescriptor(buffer, fb_nsUrl));
    }
    auto fb_namespaces = buffer.CreateVector(namespaces_vector);

    // serialize attributes
    for (const auto *attr : m_metadataAttrs) {
        auto id = attr->getAttrId();
        auto value = attr->getAttrValue();
        auto p = serialize_value(buffer, value);
        attrs_vector.push_back(lbm1::CreateAttrDescriptor(buffer,
            id.getAddress().getAddress(), id.getType(), p.first, p.second));
    }
    auto fb_attrs = buffer.CreateVector(attrs_vector);

    // build package from buffer
    lbm1::MetadataBuilder metadataBuilder(buffer);

    metadataBuilder.add_abi(lbm1::MetadataVersion::Version1);
    metadataBuilder.add_namespaces(fb_namespaces);
    metadataBuilder.add_attrs(fb_attrs);

    // serialize package and mark the buffer as finished
    auto metadata = metadataBuilder.Finish();
    buffer.Finish(metadata, lbm1::MetadataIdentifier());

    // copy the flatbuffer into our own byte array and instantiate package
    auto bytes = tempo_utils::MemoryBytes::copy(buffer.GetBufferSpan());
    return LyricMetadata(bytes);
}
