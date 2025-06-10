
#include <lyric_parser/archetype_attr.h>
#include <lyric_parser/archetype_namespace.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>

lyric_parser::ArchetypeNode::ArchetypeNode(
    ArchetypeNamespace *nodeNamespace,
    tu_uint32 idValue,
    const ParseLocation &location,
    ArchetypeId *archetypeId,
    ArchetypeState *state)
    : m_namespace(nodeNamespace),
      m_idValue(idValue),
      m_location(location),
      m_archetypeId(archetypeId),
      m_state(state)
{
    TU_ASSERT (m_namespace != nullptr);
    TU_ASSERT (m_archetypeId != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_parser::ArchetypeNode::ArchetypeNode(
    const NodeWalker &archetypeNode,
    ArchetypeNamespace *nodeNamespace,
    tu_uint32 idValue,
    const ParseLocation &location,
    ArchetypeId *archetypeId,
    ArchetypeState *state)
    : ArchetypeNode(nodeNamespace, idValue, location, archetypeId, state)
{
    m_archetypeNode = archetypeNode;
    TU_ASSERT (m_archetypeNode.isValid());
}

lyric_parser::NodeWalker
lyric_parser::ArchetypeNode::getArchetypeNode() const
{
    return m_archetypeNode;
}

lyric_parser::ArchetypeNamespace *
lyric_parser::ArchetypeNode::getNamespace() const
{
    return m_namespace;
}

tu_uint32
lyric_parser::ArchetypeNode::getIdValue() const
{
    return m_idValue;
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
lyric_parser::ArchetypeNode::isNamespace(const tempo_schema::SchemaNs &schemaNs) const
{
    return std::string_view(schemaNs.getNs()) == namespaceView();
}

bool
lyric_parser::ArchetypeNode::hasAttr(const AttrId &attrId) const
{
    return m_attrs.contains(attrId);
}

bool
lyric_parser::ArchetypeNode::hasAttr(const tempo_schema::AttrValidator &validator) const
{
    auto key = validator.getKey();
    return findAttr(key.ns, key.id) != nullptr;
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

tempo_utils::Status
lyric_parser::ArchetypeNode::putAttr(ArchetypeAttr *attr)
{
    TU_ASSERT (attr != nullptr);

    auto attrId = attr->getAttrId();
    if (m_attrs.contains(attrId))
        return ParseStatus::forCondition(ParseCondition::kParseInvariant,
            "node contains duplicate attr");

    auto limits = m_state->getLimits();
    if (m_children.size() >= limits.maxNodeAttrs)
        return ParseStatus::forCondition(ParseCondition::kResourceExhausted,
            "node attr count exceeds {}", limits.maxNodeAttrs);
    m_attrs[attrId] = attr;
    return {};
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

tempo_utils::Status
lyric_parser::ArchetypeNode::prependChild(ArchetypeNode *child)
{
    TU_ASSERT (child != nullptr);
    auto limits = m_state->getLimits();
    if (m_children.size() >= limits.maxNodeChildren)
        return ParseStatus::forCondition(ParseCondition::kResourceExhausted,
            "node children count exceeds {}", limits.maxNodeChildren);
    m_children.insert(m_children.begin(), child);
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeNode::appendChild(ArchetypeNode *child)
{
    TU_ASSERT (child != nullptr);
    auto limits = m_state->getLimits();
    if (m_children.size() >= limits.maxNodeChildren)
        return ParseStatus::forCondition(ParseCondition::kResourceExhausted,
            "node children count exceeds {}", limits.maxNodeChildren);
    m_children.push_back(child);
    return {};
}

tempo_utils::Status
lyric_parser::ArchetypeNode::insertChild(int index, ArchetypeNode *child)
{
    TU_ASSERT (child != nullptr);
    if (index == 0)
        return prependChild(child);
    auto limits = m_state->getLimits();
    if (m_children.size() >= limits.maxNodeChildren)
        return ParseStatus::forCondition(ParseCondition::kResourceExhausted,
            "node children count exceeds {}", limits.maxNodeChildren);
    if (index > 0) {
        if (index < m_children.size()) {
            auto it = m_children.begin();
            std::advance(it, index);
            m_children.insert(it, child);
        } else {
            return appendChild(child);
        }
    } else {
        if (m_children.size() + index >= 0) {
            auto it = m_children.end();
            std::advance(it, index + 1);
            m_children.insert(it, child);
        } else {
            return prependChild(child);
        }
    }
    return {};
}

tempo_utils::Result<lyric_parser::ArchetypeNode *>
lyric_parser::ArchetypeNode::replaceChild(int index, ArchetypeNode *child)
{
    TU_ASSERT (child != nullptr);
    if (0 <= index && std::cmp_less(index, m_children.size())) {
        auto *prev = m_children.at(index);
        m_children[index] = child;
        return prev;
    }
    return ParseStatus::forCondition(ParseCondition::kParseInvariant,
        "invalid child index {}", index);
}

tempo_utils::Result<lyric_parser::ArchetypeNode *>
lyric_parser::ArchetypeNode::removeChild(int index)
{
    if (0 <= index && std::cmp_less(index, m_children.size())) {
        auto iterator = m_children.begin();
        std::advance(iterator, index);
        auto *child = *iterator;
        m_children.erase(iterator);
        return child;
    }
    return ParseStatus::forCondition(ParseCondition::kParseInvariant,
        "invalid child index {}", index);
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
    return idValue == m_idValue && std::string_view(nsString) == nsUrl.toString();
}
