
#include <lyric_build/metadata_attr.h>
#include <lyric_build/metadata_attr_writer.h>
#include <lyric_build/metadata_namespace.h>
#include <tempo_schema/schema_result.h>

lyric_build::MetadataAttrWriter::MetadataAttrWriter(const tempo_schema::AttrKey &key, MetadataState *state)
    : m_key(key),
      m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

tempo_utils::Result<tu_uint32>
lyric_build::MetadataAttrWriter::putNamespace(const tempo_utils::Url &nsUrl)
{
    auto putNamespaceResult = m_state->putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kConversionError,"failed to create namespace");
    auto *ns = putNamespaceResult.getResult();
    return ns->getAddress().getAddress();
}

tempo_utils::Result<tu_uint32>
lyric_build::MetadataAttrWriter::putNil()
{
    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
    auto putNamespaceResult = putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return putNamespaceResult.getStatus();
    auto nsAddress = putNamespaceResult.getResult();
    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id),
        tempo_schema::AttrValue(nullptr));
    if (appendAttrResult.isStatus())
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kConversionError, "failed to append attr");
    auto *attr = appendAttrResult.getResult();
    return attr->getAddress().getAddress();
}

tempo_utils::Result<tu_uint32>
lyric_build::MetadataAttrWriter::putBool(bool b)
{
    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
    auto putNamespaceResult = putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return putNamespaceResult.getStatus();
    auto nsAddress = putNamespaceResult.getResult();
    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id),
        tempo_schema::AttrValue(b));
    if (appendAttrResult.isStatus())
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kConversionError, "failed to append attr");
    auto *attr = appendAttrResult.getResult();
    return attr->getAddress().getAddress();
}

tempo_utils::Result<tu_uint32>
lyric_build::MetadataAttrWriter::putInt64(tu_int64 i64)
{
    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
    auto putNamespaceResult = putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return putNamespaceResult.getStatus();
    auto nsAddress = putNamespaceResult.getResult();
    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id),
        tempo_schema::AttrValue(i64));
    if (appendAttrResult.isStatus())
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kConversionError, "failed to append attr");
    auto *attr = appendAttrResult.getResult();
    return attr->getAddress().getAddress();
}

tempo_utils::Result<tu_uint32>
lyric_build::MetadataAttrWriter::putFloat64(double dbl)
{
    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
    auto putNamespaceResult = putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return putNamespaceResult.getStatus();
    auto nsAddress = putNamespaceResult.getResult();
    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id),
        tempo_schema::AttrValue(dbl));
    if (appendAttrResult.isStatus())
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kConversionError, "failed to append attr");
    auto *attr = appendAttrResult.getResult();
    return attr->getAddress().getAddress();
}

tempo_utils::Result<tu_uint32>
lyric_build::MetadataAttrWriter::putUInt64(tu_uint64 u64)
{
    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
    auto putNamespaceResult = putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return putNamespaceResult.getStatus();
    auto nsAddress = putNamespaceResult.getResult();
    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id),
        tempo_schema::AttrValue(u64));
    if (appendAttrResult.isStatus())
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kConversionError, "failed to append attr");
    auto *attr = appendAttrResult.getResult();
    return attr->getAddress().getAddress();
}

tempo_utils::Result<tu_uint32>
lyric_build::MetadataAttrWriter::putUInt32(tu_uint32 u32)
{
    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
    auto putNamespaceResult = putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return putNamespaceResult.getStatus();
    auto nsAddress = putNamespaceResult.getResult();
    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id),
        tempo_schema::AttrValue(u32));
    if (appendAttrResult.isStatus())
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kConversionError, "failed to append attr");
    auto *attr = appendAttrResult.getResult();
    return attr->getAddress().getAddress();
}

tempo_utils::Result<tu_uint32>
lyric_build::MetadataAttrWriter::putUInt16(tu_uint16 u16)
{
    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
    auto putNamespaceResult = putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return putNamespaceResult.getStatus();
    auto nsAddress = putNamespaceResult.getResult();
    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id),
        tempo_schema::AttrValue(u16));
    if (appendAttrResult.isStatus())
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kConversionError, "failed to append attr");
    auto *attr = appendAttrResult.getResult();
    return attr->getAddress().getAddress();
}

tempo_utils::Result<tu_uint32>
lyric_build::MetadataAttrWriter::putUInt8(tu_uint8 u8)
{
    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
    auto putNamespaceResult = putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return putNamespaceResult.getStatus();
    auto nsAddress = putNamespaceResult.getResult();
    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id),
        tempo_schema::AttrValue(u8));
    if (appendAttrResult.isStatus())
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kConversionError, "failed to append attr");
    auto *attr = appendAttrResult.getResult();
    return attr->getAddress().getAddress();
}

tempo_utils::Result<tu_uint32>
lyric_build::MetadataAttrWriter::putString(std::string_view str)
{
    auto nsUrl = tempo_utils::Url::fromString(m_key.ns);
    auto putNamespaceResult = putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return putNamespaceResult.getStatus();
    auto nsAddress = putNamespaceResult.getResult();
    auto appendAttrResult = m_state->appendAttr(AttrId(NamespaceAddress(nsAddress), m_key.id),
        tempo_schema::AttrValue(str));
    if (appendAttrResult.isStatus())
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kConversionError, "failed to append attr");
    auto *attr = appendAttrResult.getResult();
    return attr->getAddress().getAddress();
}