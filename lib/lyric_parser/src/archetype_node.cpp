
#include <lyric_parser/archetype_attr.h>
#include <lyric_parser/archetype_namespace.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>

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

std::string_view
lyric_parser::ArchetypeNode::namespaceView() const
{
    return m_namespace->namespaceView();
}

bool
lyric_parser::ArchetypeNode::isNamespace(const tempo_utils::SchemaNs &schemaNs) const
{
    return std::string_view(schemaNs.getNs()) == namespaceView();
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

lyric_parser::ArchetypeAttr *
lyric_parser::ArchetypeNode::findAttr(const char *nsString, tu_uint32 idValue) const
{
    std::string_view nsView(nsString);
    for (const auto &entry : m_attrs) {
        const auto &attrId = entry.first;
        if (attrId.namespaceView() == nsView && attrId.getType() == idValue)
            return entry.second;
    }
    return nullptr;
}

tu_uint32
lyric_parser::ArchetypeNode::findAttrIndex(const char *nsString, tu_uint32 idValue) const
{
    auto *archetypeAttr = findAttr(nsString, idValue);
    if (archetypeAttr == nullptr)
        return INVALID_ADDRESS_U32;
    auto *archetypeId = archetypeAttr->getArchetypeId();
    return archetypeId->getOffset();
}

lyric_parser::AttrValue
lyric_parser::ArchetypeNode::getAttrValue(const char *nsString, tu_uint32 idValue) const
{
    auto *attr = findAttr(nsString, idValue);
    if (attr != nullptr)
        return attr->getAttrValue();
    return {};

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

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeNode::getChild(int index) const
{
    if (0 <= index && std::cmp_less(index, m_children.size()))
        return m_children.at(index);
    return {};
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

void
lyric_parser::ArchetypeNode::insertChild(int index, ArchetypeNode *child)
{
    TU_ASSERT (child != nullptr);
    if (index == 0) {
        prependChild(child);
    } else if (index > 0) {
        if (index < m_children.size()) {
            auto it = m_children.begin();
            std::advance(it, index);
            m_children.insert(it, child);
        } else {
            appendChild(child);
        }
    } else {
        if (m_children.size() + index >= 0) {
            auto it = m_children.end();
            std::advance(it, index + 1);
            m_children.insert(it, child);
        } else {
            prependChild(child);
        }
    }
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeNode::replaceChild(int index, ArchetypeNode *child)
{
    TU_ASSERT (child != nullptr);
    if (0 <= index && std::cmp_less(index, m_children.size())) {
        auto *prev = m_children.at(index);
        m_children[index] = child;
        return prev;
    }
    return nullptr;
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeNode::removeChild(int index)
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
