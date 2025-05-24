
#include <lyric_parser/archetype_attr.h>
#include <lyric_parser/archetype_state_attr_writer.h>
#include <lyric_parser/archetype_namespace.h>
#include <lyric_parser/archetype_state.h>
#include <tempo_schema/schema_result.h>

lyric_parser::ArchetypeStateAttrWriter::ArchetypeStateAttrWriter(const tempo_schema::AttrKey &key, ArchetypeState *state)
    : m_key(key),
      m_state(state),
      m_attr(nullptr)
{
    TU_ASSERT (m_state != nullptr);
}

lyric_parser::ArchetypeAttr *
lyric_parser::ArchetypeStateAttrWriter::getAttr()
{
    return m_attr;
}

lyric_parser::ArchetypeState *
lyric_parser::ArchetypeStateAttrWriter::getWriterState()
{
    return m_state;
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeStateAttrWriter::putNamespace(const tempo_utils::Url &nsUrl)
{
    auto putNamespaceResult = m_state->putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return tempo_schema::SchemaStatus::forCondition(
            tempo_schema::SchemaCondition::kConversionError, "failed to create namespace");
    auto *ns = putNamespaceResult.getResult();
    return ns->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeStateAttrWriter::putNil()
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_schema::AttrValue(nullptr))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeStateAttrWriter::putBool(bool b)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_schema::AttrValue(b))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeStateAttrWriter::putInt64(tu_int64 i64)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_schema::AttrValue(i64))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeStateAttrWriter::putFloat64(double dbl)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_schema::AttrValue(dbl))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeStateAttrWriter::putUInt64(tu_uint64 u64)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_schema::AttrValue(u64))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeStateAttrWriter::putUInt32(tu_uint32 u32)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_schema::AttrValue(u32))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeStateAttrWriter::putUInt16(tu_uint16 u16)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_schema::AttrValue(u16))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeStateAttrWriter::putUInt8(tu_uint8 u8)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_schema::AttrValue(u8))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeStateAttrWriter::putString(std::string_view str)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_schema::AttrValue(str))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeStateAttrWriter::putHandle(tempo_schema::AttrHandle handle)
{
    auto *archetypeId = m_state->getId(handle.handle);
    TU_ASSERT (archetypeId->getType() == ArchetypeDescriptorType::Attr);
    m_attr = m_state->getAttr(archetypeId->getOffset());
    return m_attr->getArchetypeId()->getId();
}
