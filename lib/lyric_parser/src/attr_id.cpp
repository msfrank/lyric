
#include <lyric_parser/archetype_namespace.h>
#include <lyric_parser/attr_id.h>

lyric_parser::AttrId::AttrId()
    : m_namespace(nullptr),
      m_type(INVALID_ADDRESS_U32)
{
}

lyric_parser::AttrId::AttrId(ArchetypeNamespace *attrNamespace, tu_uint32 type)
    : m_namespace(attrNamespace),
      m_type(type)
{
    TU_ASSERT (m_namespace != nullptr);
    TU_ASSERT (m_type != INVALID_ADDRESS_U32);
}

lyric_parser::AttrId::AttrId(const AttrId &other)
    : m_namespace(other.m_namespace),
      m_type(other.m_type)
{
}

bool
lyric_parser::AttrId::isValid() const
{
    return m_namespace != nullptr && m_type != INVALID_ADDRESS_U32;
}

lyric_parser::ArchetypeNamespace *
lyric_parser::AttrId::getNamespace() const
{
    return m_namespace;
}

std::string_view
lyric_parser::AttrId::namespaceView() const
{
    return m_namespace->namespaceView();
}

bool
lyric_parser::AttrId::isNamespace(const tempo_utils::SchemaNs &schemaNs) const
{
    return std::string_view(schemaNs.getNs()) == namespaceView();
}

tu_uint32
lyric_parser::AttrId::getType() const
{
    return m_type;
}

bool
lyric_parser::AttrId::operator==(const AttrId &other) const
{
    if (!isValid())
        return !other.isValid();
    if (!other.isValid())
        return false;
    return m_type == other.m_type
           && m_namespace->getArchetypeId()->getId() == other.m_namespace->getArchetypeId()->getId();
}

tu_uint32
lyric_parser::AttrId::getIdOffset() const
{
    if (isValid())
        return m_namespace->getArchetypeId()->getId();
    return INVALID_ADDRESS_U32;
}
