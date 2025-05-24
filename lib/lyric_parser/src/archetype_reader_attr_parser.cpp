
#include <lyric_parser/archetype_reader_attr_parser.h>
#include <lyric_parser/generated/archetype.h>
#include <lyric_parser/internal/archetype_reader.h>
#include <tempo_schema/schema_result.h>

lyric_parser::ArchetypeReaderAttrParser::ArchetypeReaderAttrParser(
    std::shared_ptr<const internal::ArchetypeReader> reader)
    : m_reader(reader)
{
    TU_ASSERT (m_reader != nullptr);
}

std::shared_ptr<const lyric_parser::internal::ArchetypeReader> *
lyric_parser::ArchetypeReaderAttrParser::getParserState()
{
    return &m_reader;
}

tempo_utils::Status
lyric_parser::ArchetypeReaderAttrParser::getNil(tu_uint32 index, std::nullptr_t &nil)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lyi1::Value::TrueFalseNilValue)
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_TrueFalseNilValue();
    if (value->tfn() != lyi1::TrueFalseNil::Nil)
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    nil = nullptr;
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeReaderAttrParser::getBool(tu_uint32 index, bool &b)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lyi1::Value::TrueFalseNilValue)
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_TrueFalseNilValue();
    switch (value->tfn()) {
        case lyi1::TrueFalseNil::True:
            b = true;
            return {};
        case lyi1::TrueFalseNil::False:
            b = false;
            return {};
        case lyi1::TrueFalseNil::Nil:
        default:
            return tempo_schema::SchemaStatus::forCondition(
                tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    }
}

tempo_utils::Status
lyric_parser::ArchetypeReaderAttrParser::getInt64(tu_uint32 index, tu_int64 &i64)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lyi1::Value::Int64Value)
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_Int64Value();
    i64 = value->i64();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeReaderAttrParser::getFloat64(tu_uint32 index, double &dbl)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lyi1::Value::Float64Value)
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_Float64Value();
    dbl = value->f64();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeReaderAttrParser::getUInt64(tu_uint32 index, tu_uint64 &u64)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lyi1::Value::UInt64Value)
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_UInt64Value();
    u64 = value->u64();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeReaderAttrParser::getUInt32(tu_uint32 index, tu_uint32 &u32)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lyi1::Value::UInt32Value)
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_UInt32Value();
    u32 = value->u32();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeReaderAttrParser::getUInt16(tu_uint32 index, tu_uint16 &u16)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lyi1::Value::UInt16Value)
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_UInt16Value();
    u16 = value->u16();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeReaderAttrParser::getUInt8(tu_uint32 index, tu_uint8 &u8)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lyi1::Value::UInt8Value)
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_UInt8Value();
    u8 = value->u8();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeReaderAttrParser::getString(tu_uint32 index, std::string &str)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lyi1::Value::StringValue)
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_StringValue();
    str = value->utf8()? value->utf8()->str() : std::string();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeReaderAttrParser::getHandle(tu_uint32 index, tempo_schema::AttrHandle &handle)
{
    auto *attr = m_reader->getAttr(index);
    TU_ASSERT (attr != nullptr);
    if (attr->attr_value_type() != lyi1::Value::NodeValue)
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kWrongType,"attr type mismatch");
    auto *value = attr->attr_value_as_NodeValue();
    handle = tempo_schema::AttrHandle{value->node()};
    return {};
}
