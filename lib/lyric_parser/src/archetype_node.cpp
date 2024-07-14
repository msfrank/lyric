
#include <lyric_parser/archetype_attr.h>
#include <lyric_parser/archetype_namespace.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>

//lyric_parser::ArchetypeNode::ArchetypeNode(
//    ArchetypeNamespace *nodeNamespace,
//    tu_uint32 typeOffset,
//    tu_uint32 lineNr,
//    tu_uint32 columnNr,
//    tu_uint32 fileOffset,
//    tu_uint32 textSpan,
//    ArchetypeId *archetypeId,
//    ArchetypeState *state)
//    : m_namespace(nodeNamespace),
//      m_typeOffset(typeOffset),
//      m_lineNr(lineNr),
//      m_columnNr(columnNr),
//      m_fileOffset(fileOffset),
//      m_textSpan(textSpan),
//      m_archetypeId(archetypeId),
//      m_state(state)
//{
//    TU_ASSERT (m_namespace != nullptr);
//    TU_ASSERT (m_archetypeId != nullptr);
//    TU_ASSERT (m_state != nullptr);
//}

lyric_parser::ArchetypeNode::ArchetypeNode(
    ArchetypeNamespace *nodeNamespace,
    tu_uint32 typeOffset,
    const ParseLocation &location,
    ArchetypeId *archetypeId,
    ArchetypeState *state)
    : m_namespace(nodeNamespace),
      m_typeOffset(typeOffset),
      m_location(location),
      m_archetypeId(archetypeId),
      m_state(state)
{
    TU_ASSERT (m_namespace != nullptr);
    TU_ASSERT (m_archetypeId != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_parser::ArchetypeNamespace *
lyric_parser::ArchetypeNode::getNamespace() const
{
    return m_namespace;
}

tu_uint32
lyric_parser::ArchetypeNode::getTypeOffset() const
{
    return m_typeOffset;
}

lyric_parser::ParseLocation
lyric_parser::ArchetypeNode::getLocation() const
{
    return m_location;
}

lyric_parser::ArchetypeId *
lyric_parser::ArchetypeNode::getArchetypeId() const
{
    return m_archetypeId;
}

bool
lyric_parser::ArchetypeNode::hasAttr(const AttrId &attrId) const
{
    return m_attrs.contains(attrId);
}

lyric_parser::ArchetypeAttr *
lyric_parser::ArchetypeNode::getAttr(const AttrId &attrId) const
{
    auto entry = m_attrs.find(attrId);
    if (entry != m_attrs.cend())
        return entry->second;
    return nullptr;
}

void
lyric_parser::ArchetypeNode::putAttr(ArchetypeAttr *attr)
{
    TU_ASSERT (attr != nullptr);

    auto attrId = attr->getAttrId();
    if (m_attrs.contains(attrId)) {
        m_state->throwParseInvariant("node contains duplicate attr");
    }
    m_attrs[attrId] = attr;
}

absl::flat_hash_map<lyric_parser::AttrId,lyric_parser::ArchetypeAttr *>::const_iterator
lyric_parser::ArchetypeNode::attrsBegin() const
{
    return m_attrs.cbegin();
}

absl::flat_hash_map<lyric_parser::AttrId,lyric_parser::ArchetypeAttr *>::const_iterator
lyric_parser::ArchetypeNode::attrsEnd() const
{
    return m_attrs.cend();
}

int
lyric_parser::ArchetypeNode::numAttrs() const
{
    return m_attrs.size();
}

void
lyric_parser::ArchetypeNode::prependChild(ArchetypeNode *child)
{
    TU_ASSERT (child != nullptr);
    m_children.insert(m_children.begin(), child);
}

void
lyric_parser::ArchetypeNode::appendChild(ArchetypeNode *child)
{
    TU_ASSERT (child != nullptr);
    m_children.push_back(child);
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeNode::getChild(int index) const
{
    if (0 <= index && std::cmp_less(index, m_children.size()))
        return m_children.at(index);
    return {};
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeNode::detachChild(int index)
{
    if (0 <= index && std::cmp_less(index, m_children.size())) {
        auto iterator = m_children.begin();
        std::advance(iterator, index);
        auto *child = *iterator;
        m_children.erase(iterator);
        return child;
    }
    return nullptr;
}

std::vector<lyric_parser::ArchetypeNode *>::const_iterator
lyric_parser::ArchetypeNode::childrenBegin() const
{
    return m_children.cbegin();
}

std::vector<lyric_parser::ArchetypeNode *>::const_iterator
lyric_parser::ArchetypeNode::childrenEnd() const
{
    return m_children.cend();
}

int
lyric_parser::ArchetypeNode::numChildren() const
{
    return m_children.size();
}

bool
lyric_parser::ArchetypeNode::matchesNsAndId(const char *nsString, tu_uint32 idValue) const
{
    auto nsUrl = m_namespace->getNsUrl();
    TU_ASSERT (nsUrl.isValid());
    return idValue == m_typeOffset && std::string_view(nsString) == nsUrl.toString();
}
