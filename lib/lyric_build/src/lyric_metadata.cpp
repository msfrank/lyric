
#include <lyric_build/internal/metadata_reader.h>
#include <lyric_build/lyric_metadata.h>

lyric_build::LyricMetadata::LyricMetadata()
{
}

lyric_build::LyricMetadata::LyricMetadata(std::shared_ptr<const tempo_utils::ImmutableBytes> immutableBytes)
    : m_bytes(immutableBytes)
{
    TU_ASSERT (m_bytes != nullptr);
    std::span<const tu_uint8> bytes(m_bytes->getData(), m_bytes->getSize());
    m_reader = std::make_shared<internal::MetadataReader>(bytes);
}

lyric_build::LyricMetadata::LyricMetadata(std::span<const tu_uint8> unownedBytes)
{
    m_reader = std::make_shared<internal::MetadataReader>(unownedBytes);
}

lyric_build::LyricMetadata::LyricMetadata(const lyric_build::LyricMetadata &other)
    : m_bytes(other.m_bytes),
      m_reader(other.m_reader)
{
}

bool
lyric_build::LyricMetadata::isValid() const
{
    if (m_reader == nullptr)
        return false;
    return m_reader->isValid();
}

lyric_build::MetadataVersion
lyric_build::LyricMetadata::getABI() const
{
    if (m_reader == nullptr)
        return MetadataVersion::Unknown;
    switch (m_reader->getABI()) {
        case lbm1::MetadataVersion::Version1:
            return MetadataVersion::Version1;
        case lbm1::MetadataVersion::Unknown:
        default:
            return MetadataVersion::Unknown;
    }
}

bool
lyric_build::LyricMetadata::hasAttr(const tempo_schema::AttrKey &key) const
{
    auto index = findIndexForAttr(key);
    return index != METADATA_INVALID_OFFSET_U32;
}

bool
lyric_build::LyricMetadata::hasAttr(const tempo_schema::AttrValidator &validator) const
{
    return hasAttr(validator.getKey());
}

tu_uint32
lyric_build::LyricMetadata::findIndexForAttr(const tempo_schema::AttrKey &key) const
{
    if (!isValid())
        return METADATA_INVALID_OFFSET_U32;
    for (tu_uint32 i = 0; i < m_reader->numAttrs(); i++) {
        auto *attr = m_reader->getAttr(i);
        TU_ASSERT (attr != nullptr);
        auto *ns = m_reader->getNamespace(attr->attr_ns());
        TU_ASSERT (ns != nullptr);
        auto *nsUrl = ns->ns_url();
        if (nsUrl == nullptr)
            continue;
        if (std::string_view(key.ns) == nsUrl->string_view() && key.id == attr->attr_id())
            return i;
    }
    return METADATA_INVALID_OFFSET_U32;
}

int
lyric_build::LyricMetadata::numAttrs() const
{
    if (!isValid())
        return 0;
    return m_reader->numAttrs();
}

tempo_schema::Attr
lyric_build::LyricMetadata::getAttr(tu_uint32 index) const
{
    if (!isValid())
        return {};
    auto *attr = m_reader->getAttr(index);
    if (attr == nullptr)
        return {};

    auto *ns = m_reader->getNamespace(attr->attr_ns());
    if (ns == nullptr)
        return {};
    if (ns->ns_url() == nullptr)
        return {};

    tempo_schema::AttrKey key(ns->ns_url()->c_str(), attr->attr_id());

    tempo_schema::AttrValue value;
    switch (attr->attr_value_type()) {
        case lbm1::Value::TrueFalseNilValue: {
            auto *tfn = attr->attr_value_as_TrueFalseNilValue();
            if (tfn->tfn() == lbm1::TrueFalseNil::Nil) {
                value = tempo_schema::AttrValue(nullptr);
            } else {
                value = tempo_schema::AttrValue(tfn->tfn() == lbm1::TrueFalseNil::True);
            }
            break;
        }
        case lbm1::Value::Int64Value: {
            auto i64 = attr->attr_value_as_Int64Value();
            value = tempo_schema::AttrValue(i64->i64());
            break;
        }
        case lbm1::Value::Float64Value: {
            auto f64 = attr->attr_value_as_Float64Value();
            value = tempo_schema::AttrValue(f64->f64());
            break;
        }
        case lbm1::Value::UInt64Value: {
            auto u64 = attr->attr_value_as_UInt64Value();
            value = tempo_schema::AttrValue(u64->u64());
            break;
        }
        case lbm1::Value::UInt32Value: {
            auto u32 = attr->attr_value_as_UInt32Value();
            value = tempo_schema::AttrValue(u32->u32());
            break;
        }
        case lbm1::Value::UInt16Value: {
            auto u16 = attr->attr_value_as_UInt16Value();
            value = tempo_schema::AttrValue(u16->u16());
            break;
        }
        case lbm1::Value::UInt8Value: {
            auto u8 = attr->attr_value_as_UInt8Value();
            value = tempo_schema::AttrValue(u8->u8());
            break;
        }
        case lbm1::Value::StringValue: {
            auto str = attr->attr_value_as_StringValue();
            if (str->utf8()) {
                value = tempo_schema::AttrValue(str->utf8()->string_view());
            } else {
                value = tempo_schema::AttrValue(std::string_view());
            }
            break;
        }
        default:
            return {};
    }

    return tempo_schema::Attr(key, value);
}

std::shared_ptr<const lyric_build::internal::MetadataReader>
lyric_build::LyricMetadata::getReader() const
{
    return m_reader;
}

std::span<const tu_uint8>
lyric_build::LyricMetadata::bytesView() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->bytesView();
}

std::string
lyric_build::LyricMetadata::dumpJson() const
{
    if (m_reader == nullptr)
        return {};
    return m_reader->dumpJson();
}

bool
lyric_build::LyricMetadata::verify(std::span<const tu_uint8> bytes)
{
    flatbuffers::Verifier verifier(bytes.data(), bytes.size());
    return lbm1::VerifyMetadataBuffer(verifier);
}
