
#include <lyric_parser/archetype_state_attr_parser.h>
#include <lyric_parser/archetype_state.h>

lyric_parser::ArchetypeStateAttrParser::ArchetypeStateAttrParser(ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
}

lyric_parser::ArchetypeState *
lyric_parser::ArchetypeStateAttrParser::getParserState()
{
    return m_state;
}

tempo_utils::Status
lyric_parser::ArchetypeStateAttrParser::getNil(tu_uint32 index, std::nullptr_t &nil)
{
    auto *attr = m_state->getAttr(index);
    TU_ASSERT (attr != nullptr);
    auto value = attr->getAttrValue();
    if (!value.isLiteral())
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto literal = value.getLiteral();
    if (literal.getType() != tempo_utils::ValueType::Nil)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    nil = nullptr;
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeStateAttrParser::getBool(tu_uint32 index, bool &b)
{
    auto *attr = m_state->getAttr(index);
    TU_ASSERT (attr != nullptr);
    auto value = attr->getAttrValue();
    if (!value.isLiteral())
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto literal = value.getLiteral();
    if (literal.getType() != tempo_utils::ValueType::Bool)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    b = literal.getBool();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeStateAttrParser::getInt64(tu_uint32 index, tu_int64 &i64)
{
    auto *attr = m_state->getAttr(index);
    TU_ASSERT (attr != nullptr);
    auto value = attr->getAttrValue();
    if (!value.isLiteral())
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto literal = value.getLiteral();
    if (literal.getType() != tempo_utils::ValueType::Int64)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    i64 = literal.getInt64();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeStateAttrParser::getFloat64(tu_uint32 index, double &dbl)
{
    auto *attr = m_state->getAttr(index);
    TU_ASSERT (attr != nullptr);
    auto value = attr->getAttrValue();
    if (!value.isLiteral())
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto literal = value.getLiteral();
    if (literal.getType() != tempo_utils::ValueType::Float64)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    dbl = literal.getFloat64();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeStateAttrParser::getUInt64(tu_uint32 index, tu_uint64 &u64)
{
    auto *attr = m_state->getAttr(index);
    TU_ASSERT (attr != nullptr);
    auto value = attr->getAttrValue();
    if (!value.isLiteral())
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto literal = value.getLiteral();
    if (literal.getType() != tempo_utils::ValueType::UInt64)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    u64 = literal.getUInt64();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeStateAttrParser::getUInt32(tu_uint32 index, tu_uint32 &u32)
{
    auto *attr = m_state->getAttr(index);
    TU_ASSERT (attr != nullptr);
    auto value = attr->getAttrValue();
    if (!value.isLiteral())
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto literal = value.getLiteral();
    if (literal.getType() != tempo_utils::ValueType::UInt32)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    u32 = literal.getUInt32();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeStateAttrParser::getUInt16(tu_uint32 index, tu_uint16 &u16)
{
    auto *attr = m_state->getAttr(index);
    TU_ASSERT (attr != nullptr);
    auto value = attr->getAttrValue();
    if (!value.isLiteral())
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto literal = value.getLiteral();
    if (literal.getType() != tempo_utils::ValueType::UInt16)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    u16 = literal.getUInt16();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeStateAttrParser::getUInt8(tu_uint32 index, tu_uint8 &u8)
{
    auto *attr = m_state->getAttr(index);
    TU_ASSERT (attr != nullptr);
    auto value = attr->getAttrValue();
    if (!value.isLiteral())
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto literal = value.getLiteral();
    if (literal.getType() != tempo_utils::ValueType::UInt8)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    u8 = literal.getUInt8();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeStateAttrParser::getString(tu_uint32 index, std::string &str)
{
    auto *attr = m_state->getAttr(index);
    TU_ASSERT (attr != nullptr);
    auto value = attr->getAttrValue();
    if (!value.isLiteral())
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto literal = value.getLiteral();
    if (literal.getType() != tempo_utils::ValueType::String)
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    str = literal.getString();
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeStateAttrParser::getHandle(tu_uint32 index, tempo_utils::AttrHandle &handle)
{
    auto *attr = m_state->getAttr(index);
    TU_ASSERT (attr != nullptr);
    auto value = attr->getAttrValue();
    if (!value.isNode())
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kWrongType,"attr type mismatch");
    auto *node = value.getNode();
    TU_ASSERT (node != nullptr);
    auto *archetypeId = node->getArchetypeId();
    handle = tempo_utils::AttrHandle{archetypeId->getId()};
    return {};
}
