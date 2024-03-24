
#include <lyric_parser/archetype_attr.h>
#include <lyric_parser/archetype_namespace.h>
#include <lyric_parser/archetype_node.h>

lyric_parser::ArchetypeNode::ArchetypeNode(
    tu_uint32 nsOffset,
    tu_uint32 typeOffset,
    tu_uint32 lineNr,
    tu_uint32 columnNr,
    tu_uint32 fileOffset,
    tu_uint32 textSpan,
    NodeAddress address,
    ArchetypeState *state)
    : m_nsOffset(nsOffset),
      m_typeOffset(typeOffset),
      m_lineNr(lineNr),
      m_columnNr(columnNr),
      m_fileOffset(fileOffset),
      m_textSpan(textSpan),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);
}

lyric_parser::ArchetypeNode::ArchetypeNode(
    tu_uint32 nsOffset,
    tu_uint32 typeOffset,
    antlr4::Token *token,
    NodeAddress address,
    ArchetypeState *state)
    : m_nsOffset(nsOffset),
      m_typeOffset(typeOffset),
      m_address(address),
      m_state(state)
{
    TU_ASSERT (m_address.isValid());
    TU_ASSERT (m_state != nullptr);

    TU_ASSERT (token != nullptr);
    m_lineNr = token->getLine();
    m_columnNr = token->getCharPositionInLine();
    m_fileOffset = token->getStartIndex();
    m_textSpan = token->getStopIndex() - token->getStartIndex();
}

tu_uint32
lyric_parser::ArchetypeNode::getNsOffset() const
{
    return m_nsOffset;
}

tu_uint32
lyric_parser::ArchetypeNode::getTypeOffset() const
{
    return m_typeOffset;
}

tu_uint32
lyric_parser::ArchetypeNode::getFileOffset() const
{
    return m_fileOffset;
}

tu_uint32
lyric_parser::ArchetypeNode::getLineNumber() const
{
    return m_lineNr;
}

tu_uint32
lyric_parser::ArchetypeNode::getColumnNumber() const
{
    return m_columnNr;
}

tu_uint32
lyric_parser::ArchetypeNode::getTextSpan() const
{
    return m_textSpan;
}

lyric_parser::NodeAddress
lyric_parser::ArchetypeNode::getAddress() const
{
    return m_address;
}

bool
lyric_parser::ArchetypeNode::hasAttr(const AttrId &attrId) const
{
    return m_attrs.contains(attrId);
}

lyric_parser::AttrAddress
lyric_parser::ArchetypeNode::getAttr(const AttrId &attrId) const
{
    if (m_attrs.contains(attrId))
        return m_attrs.at(attrId);
    return {};
}

void
lyric_parser::ArchetypeNode::putAttr(ArchetypeAttr *attr)
{
    TU_ASSERT (attr != nullptr);

    auto attrId = attr->getAttrId();
    if (m_attrs.contains(attrId)) {
        m_state->throwParseInvariant(m_address, "node contains duplicate attr");
    }
    m_attrs[attrId] = attr->getAddress();
}

absl::flat_hash_map<lyric_parser::AttrId,lyric_parser::AttrAddress>::const_iterator
lyric_parser::ArchetypeNode::attrsBegin() const
{
    return m_attrs.cbegin();
}

absl::flat_hash_map<lyric_parser::AttrId,lyric_parser::AttrAddress>::const_iterator
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
    m_children.insert(m_children.begin(), child->getAddress());
}

void
lyric_parser::ArchetypeNode::appendChild(ArchetypeNode *child)
{
    TU_ASSERT (child != nullptr);
    m_children.push_back(child->getAddress());
}

lyric_parser::NodeAddress
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
        auto address = *iterator;
        auto *child = m_state->getNode(address.getAddress());
        m_children.erase(iterator);
        return child;
    }
    return nullptr;
}

std::vector<lyric_parser::NodeAddress>::const_iterator
lyric_parser::ArchetypeNode::childrenBegin() const
{
    return m_children.cbegin();
}

std::vector<lyric_parser::NodeAddress>::const_iterator
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
    auto *ns = m_state->getNamespace(m_nsOffset);
    TU_ASSERT (ns != nullptr);
    auto nsUrl = ns->getNsUrl();
    TU_ASSERT (nsUrl.isValid());
    return idValue == m_typeOffset && std::string_view(nsString) == nsUrl.toString();
}
