//
//#include <lyric_serde/patchset_value_parser.h>
//
//#include "patchset_reader.h"
//
//lyric_serde::PatchsetValueParser::PatchsetValueParser(std::shared_ptr<const internal::PatchsetReader> reader)
//    : m_reader(reader)
//{
//    TU_ASSERT (m_reader != nullptr);
//}
//
//tempo_utils::AttrStatus
//lyric_serde::PatchsetValueParser::getNil(tu_uint32 index, std::nullptr_t &nil)
//{
//    auto *value = m_reader->getValue(index);
//    TU_ASSERT (value != nullptr);
//    if (value->value_type() != lps1::Value::TrueFalseNilValue)
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
//    auto *v = value->value_as_TrueFalseNilValue();
//    if (v->tfn() != lps1::TrueFalseNil::Nil)
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
//    nil = nullptr;
//    return tempo_utils::AttrStatus::ok();
//}
//
//tempo_utils::AttrStatus
//lyric_serde::PatchsetValueParser::getBool(tu_uint32 index, bool &b)
//{
//    auto *value = m_reader->getValue(index);
//    TU_ASSERT (value != nullptr);
//    if (value->value_type() != lps1::Value::TrueFalseNilValue)
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
//    auto *v = value->value_as_TrueFalseNilValue();
//    switch (v->tfn()) {
//        case lps1::TrueFalseNil::True:
//            b = true;
//            return tempo_utils::AttrStatus::ok();
//        case lps1::TrueFalseNil::False:
//            b = false;
//            return tempo_utils::AttrStatus::ok();
//        case lps1::TrueFalseNil::Nil:
//        default:
//            return tempo_utils::AttrStatus::forCondition(
//                tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
//    }
//}
//
//tempo_utils::AttrStatus
//lyric_serde::PatchsetValueParser::getInt64(tu_uint32 index, tu_int64 &i64)
//{
//    auto *value = m_reader->getValue(index);
//    TU_ASSERT (value != nullptr);
//    if (value->value_type() != lps1::Value::Int64Value)
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
//    auto *v = value->value_as_Int64Value();
//    i64 = v->i64();
//    return tempo_utils::AttrStatus::ok();
//}
//
//tempo_utils::AttrStatus
//lyric_serde::PatchsetValueParser::getFloat64(tu_uint32 index, double &dbl)
//{
//    auto *value = m_reader->getValue(index);
//    TU_ASSERT (value != nullptr);
//    if (value->value_type() != lps1::Value::Float64Value)
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
//    auto *v = value->value_as_Float64Value();
//    dbl = v->f64();
//    return tempo_utils::AttrStatus::ok();
//}
//
//tempo_utils::AttrStatus
//lyric_serde::PatchsetValueParser::getUInt64(tu_uint32 index, tu_uint64 &u64)
//{
//    auto *value = m_reader->getValue(index);
//    TU_ASSERT (value != nullptr);
//    if (value->value_type() != lps1::Value::UInt64Value)
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
//    auto *v = value->value_as_UInt64Value();
//    u64 = v->u64();
//    return tempo_utils::AttrStatus::ok();
//}
//
//tempo_utils::AttrStatus
//lyric_serde::PatchsetValueParser::getUInt32(tu_uint32 index, tu_uint32 &u32)
//{
//    auto *value = m_reader->getValue(index);
//    TU_ASSERT (value != nullptr);
//    if (value->value_type() != lps1::Value::UInt32Value)
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
//    auto *v = value->value_as_UInt32Value();
//    u32 = v->u32();
//    return tempo_utils::AttrStatus::ok();
//}
//
//tempo_utils::AttrStatus
//lyric_serde::PatchsetValueParser::getUInt16(tu_uint32 index, tu_uint16 &u16)
//{
//    auto *value = m_reader->getValue(index);
//    TU_ASSERT (value != nullptr);
//    if (value->value_type() != lps1::Value::UInt16Value)
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
//    auto *v = value->value_as_UInt16Value();
//    u16 = v->u16();
//    return tempo_utils::AttrStatus::ok();
//}
//
//tempo_utils::AttrStatus
//lyric_serde::PatchsetValueParser::getUInt8(tu_uint32 index, tu_uint8 &u8)
//{
//    auto *value = m_reader->getValue(index);
//    TU_ASSERT (value != nullptr);
//    if (value->value_type() != lps1::Value::UInt8Value)
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
//    auto *v = value->value_as_UInt8Value();
//    u8 = v->u8();
//    return tempo_utils::AttrStatus::ok();
//}
//
//tempo_utils::AttrStatus
//lyric_serde::PatchsetValueParser::getString(tu_uint32 index, std::string &str)
//{
//    auto *value = m_reader->getValue(index);
//    TU_ASSERT (value != nullptr);
//    if (value->value_type() != lps1::Value::StringValue)
//        return tempo_utils::AttrStatus::forCondition(
//            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
//    auto *v = value->value_as_StringValue();
//    str = v->utf8()? v->utf8()->str() : std::string();
//    return tempo_utils::AttrStatus::ok();
//}
