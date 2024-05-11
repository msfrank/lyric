
#include <lyric_packaging/generated/manifest.h>
#include <lyric_packaging/internal/manifest_reader.h>
#include <lyric_packaging/manifest_attr_parser.h>

lyric_packaging::ManifestAttrParser::ManifestAttrParser(std::shared_ptr<const internal::ManifestReader> reader)
    : m_reader(reader)
{
    TU_ASSERT (m_reader != nullptr);
}

tempo_utils::Status
lyric_packaging::ManifestAttrParser::getNil(tu_uint32 index, std::nullptr_t &nil)
{
    if (!m_reader->isValid())
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lpm1::Value::TrueFalseNilValue)
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_TrueFalseNilValue();
    if (value->tfn() != lpm1::TrueFalseNil::Nil)
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    nil = nullptr;
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
lyric_packaging::ManifestAttrParser::getBool(tu_uint32 index, bool &b)
{
    if (!m_reader->isValid())
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lpm1::Value::TrueFalseNilValue)
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_TrueFalseNilValue();
    switch (value->tfn()) {
        case lpm1::TrueFalseNil::True:
            b = true;
            return tempo_utils::AttrStatus::ok();
        case lpm1::TrueFalseNil::False:
            b = false;
            return tempo_utils::AttrStatus::ok();
        case lpm1::TrueFalseNil::Nil:
        default:
            return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    }
}

tempo_utils::Status
lyric_packaging::ManifestAttrParser::getInt64(tu_uint32 index, tu_int64 &i64)
{
    if (!m_reader->isValid())
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lpm1::Value::Int64Value)
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_Int64Value();
    i64 = value->i64();
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
lyric_packaging::ManifestAttrParser::getFloat64(tu_uint32 index, double &dbl)
{
    if (!m_reader->isValid())
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lpm1::Value::Float64Value)
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_Float64Value();
    dbl = value->f64();
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
lyric_packaging::ManifestAttrParser::getUInt64(tu_uint32 index, tu_uint64 &u64)
{
    if (!m_reader->isValid())
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lpm1::Value::UInt64Value)
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_UInt64Value();
    u64 = value->u64();
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
lyric_packaging::ManifestAttrParser::getUInt32(tu_uint32 index, tu_uint32 &u32)
{
    if (!m_reader->isValid())
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lpm1::Value::UInt32Value)
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_UInt32Value();
    u32 = value->u32();
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
lyric_packaging::ManifestAttrParser::getUInt16(tu_uint32 index, tu_uint16 &u16)
{
    if (!m_reader->isValid())
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lpm1::Value::UInt16Value)
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_UInt16Value();
    u16 = value->u16();
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
lyric_packaging::ManifestAttrParser::getUInt8(tu_uint32 index, tu_uint8 &u8)
{
    if (!m_reader->isValid())
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lpm1::Value::UInt8Value)
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_UInt8Value();
    u8 = value->u8();
    return tempo_utils::AttrStatus::ok();
}

tempo_utils::Status
lyric_packaging::ManifestAttrParser::getString(tu_uint32 index, std::string &str)
{
    if (!m_reader->isValid())
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kParseError, "invalid reader");
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lpm1::Value::StringValue)
        return tempo_utils::AttrStatus::forCondition(tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_StringValue();
    str = value->utf8()? value->utf8()->str() : std::string();
    return tempo_utils::AttrStatus::ok();
}
