
#include <lyric_parser/archetype_attr.h>
#include <lyric_parser/archetype_attr_writer.h>
#include <lyric_parser/archetype_namespace.h>
#include <lyric_parser/archetype_state.h>

lyric_parser::ArchetypeAttrWriter::ArchetypeAttrWriter(const tempo_utils::AttrKey &key, ArchetypeState *state)
    : m_key(key),
      m_state(state),
      m_attr(nullptr)
{
    TU_ASSERT (m_state != nullptr);
}

lyric_parser::ArchetypeAttr *
lyric_parser::ArchetypeAttrWriter::getAttr()
{
    return m_attr;
}

lyric_parser::ArchetypeState *
lyric_parser::ArchetypeAttrWriter::getWriterState()
{
    return m_state;
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeAttrWriter::putNamespace(const tempo_utils::Url &nsUrl)
{
    auto putNamespaceResult = m_state->putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return tempo_utils::AttrStatus::forCondition(
            tempo_utils::AttrCondition::kConversionError, "failed to create namespace");
    auto *ns = putNamespaceResult.getResult();
    return ns->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeAttrWriter::putNil()
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_utils::AttrValue(nullptr))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeAttrWriter::putBool(bool b)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_utils::AttrValue(b))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeAttrWriter::putInt64(tu_int64 i64)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_utils::AttrValue(i64))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeAttrWriter::putFloat64(double dbl)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_utils::AttrValue(dbl))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeAttrWriter::putUInt64(tu_uint64 u64)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_utils::AttrValue(u64))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeAttrWriter::putUInt32(tu_uint32 u32)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_utils::AttrValue(u32))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeAttrWriter::putUInt16(tu_uint16 u16)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_utils::AttrValue(u16))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeAttrWriter::putUInt8(tu_uint8 u8)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_utils::AttrValue(u8))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeAttrWriter::putString(std::string_view str)
{
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, m_state->putNamespace(m_key.ns));
    TU_ASSIGN_OR_RETURN (m_attr, m_state->appendAttr(
        AttrId(ns, m_key.id), AttrValue(tempo_utils::AttrValue(str))));
    return m_attr->getArchetypeId()->getId();
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeAttrWriter::putHandle(tempo_utils::AttrHandle handle)
{
    auto *archetypeId = m_state->getId(handle.handle);
    TU_ASSERT (archetypeId->getType() == ArchetypeDescriptorType::Attr);
    m_attr = m_state->getAttr(archetypeId->getOffset());
    return m_attr->getArchetypeId()->getId();
}
