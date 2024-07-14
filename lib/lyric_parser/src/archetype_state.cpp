
#include <absl/container/flat_hash_map.h>

#include <lyric_parser/archetype_attr.h>
#include <lyric_parser/archetype_namespace.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/generated/archetype.h>
#include <lyric_parser/internal/archetype_writer.h>
#include <lyric_parser/lyric_archetype.h>
#include <lyric_parser/parser_types.h>
#include <lyric_parser/parse_result.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/memory_bytes.h>

lyric_parser::ArchetypeState::ArchetypeState(
    const tempo_utils::Url &sourceUrl,
    tempo_tracing::ScopeManager *scopeManager)
    : m_sourceUrl(sourceUrl),
      m_scopeManager(scopeManager)
{
}

bool
lyric_parser::ArchetypeState::isEmpty()
{
    return m_nodeStack.empty();
}

void
lyric_parser::ArchetypeState::pushNode(ArchetypeNode *node)
{
    m_nodeStack.push(node);
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeState::popNode()
{
    if (m_nodeStack.empty()) {
        throw tempo_utils::StatusException(
            ParseStatus::forCondition(ParseCondition::kParseInvariant,
                "node stack is empty"));
    }
    auto *top = m_nodeStack.top();
    m_nodeStack.pop();
    return top;
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeState::peekNode()
{
    if (m_nodeStack.empty())
        return nullptr;
    return m_nodeStack.top();
}

tempo_tracing::ScopeManager *
lyric_parser::ArchetypeState::scopeManager() const
{
    return m_scopeManager;
}

lyric_parser::ArchetypeId *
lyric_parser::ArchetypeState::makeId(ArchetypeDescriptorType type, tu_uint32 offset)
{
    tu_uint32 id = m_archetypeIds.size();
    auto *archetypeId = new ArchetypeId(type, id, offset);
    m_archetypeIds.push_back(archetypeId);
    return archetypeId;
}

lyric_parser::ArchetypeId *
lyric_parser::ArchetypeState::getId(int index) const
{
    if (0 <= index && std::cmp_less(index, m_archetypeIds.size()))
        return m_archetypeIds.at(index);
    return nullptr;
}

int
lyric_parser::ArchetypeState::numIds() const
{
    return m_archetypeIds.size();
}

bool
lyric_parser::ArchetypeState::hasNamespace(const tempo_utils::Url &nsUrl) const
{
    return m_namespaceIndex.contains(nsUrl);
}

lyric_parser::ArchetypeNamespace *
lyric_parser::ArchetypeState::getNamespace(int index) const
{
    if (0 <= index && std::cmp_less(index, m_archetypeNamespaces.size()))
        return m_archetypeNamespaces.at(index);
    return nullptr;
}

lyric_parser::ArchetypeNamespace *
lyric_parser::ArchetypeState::getNamespace(const tempo_utils::Url &nsUrl) const
{
    if (!m_namespaceIndex.contains(nsUrl))
        return nullptr;
    auto nsOffset = m_namespaceIndex.at(nsUrl);
    if (0 <= nsOffset && nsOffset < m_archetypeNamespaces.size())
        return m_archetypeNamespaces.at(nsOffset);
    return nullptr;
}

tempo_utils::Result<lyric_parser::ArchetypeNamespace *>
lyric_parser::ArchetypeState::putNamespace(const tempo_utils::Url &nsUrl)
{
    if (m_namespaceIndex.contains(nsUrl)) {
        auto index = m_namespaceIndex.at(nsUrl);
        TU_ASSERT (0 <= index && index < m_archetypeNamespaces.size());
        return m_archetypeNamespaces.at(index);
    }
    tu_uint32 offset = m_archetypeNamespaces.size();
    auto *archetypeId = makeId(ArchetypeDescriptorType::Namespace, offset);
    auto *ns = new ArchetypeNamespace(nsUrl, archetypeId, this);
    m_archetypeNamespaces.push_back(ns);
    m_namespaceIndex[nsUrl] = archetypeId->getId();
    return ns;
}

tempo_utils::Result<lyric_parser::ArchetypeNamespace *>
lyric_parser::ArchetypeState::putNamespace(const char *nsString)
{
    auto nsUrl = tempo_utils::Url::fromString(nsString);
    return putNamespace(nsUrl);
}

std::vector<lyric_parser::ArchetypeNamespace *>::const_iterator
lyric_parser::ArchetypeState::namespacesBegin() const
{
    return m_archetypeNamespaces.cbegin();
}

std::vector<lyric_parser::ArchetypeNamespace *>::const_iterator
lyric_parser::ArchetypeState::namespacesEnd() const
{
    return m_archetypeNamespaces.cend();
}

int
lyric_parser::ArchetypeState::numNamespaces() const
{
    return m_archetypeNamespaces.size();
}

tempo_utils::Result<lyric_parser::ArchetypeAttr *>
lyric_parser::ArchetypeState::appendAttr(AttrId id, AttrValue value)
{
    tu_uint32 offset = m_archetypeAttrs.size();
    auto *archetypeId = makeId(ArchetypeDescriptorType::Attr, offset);
    auto *attr = new ArchetypeAttr(id, value, archetypeId, this);
    m_archetypeAttrs.push_back(attr);
    return attr;
}

lyric_parser::ArchetypeAttr *
lyric_parser::ArchetypeState::getAttr(int index) const
{
    if (0 <= index && std::cmp_less(index, m_archetypeAttrs.size()))
        return m_archetypeAttrs.at(index);
    return nullptr;
}

std::vector<lyric_parser::ArchetypeAttr *>::const_iterator
lyric_parser::ArchetypeState::attrsBegin() const
{
    return m_archetypeAttrs.cbegin();
}

std::vector<lyric_parser::ArchetypeAttr *>::const_iterator
lyric_parser::ArchetypeState::attrsEnd() const
{
    return m_archetypeAttrs.cend();
}

int
lyric_parser::ArchetypeState::numAttrs() const
{
    return m_archetypeAttrs.size();
}

tempo_utils::Result<lyric_parser::ArchetypeNode *>
lyric_parser::ArchetypeState::appendNode(
    ArchetypeNamespace *nodeNamespace,
    tu_uint32 nodeId,
    const ParseLocation &location)
{
    tu_uint32 offset = m_archetypeNodes.size();
    auto *archetypeId = makeId(ArchetypeDescriptorType::Node, offset);
    auto *node = new ArchetypeNode(nodeNamespace, nodeId, location, archetypeId, this);
    m_archetypeNodes.push_back(node);
    return node;
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeState::getNode(int index) const
{
    if (0 <= index && std::cmp_less(index, m_archetypeNodes.size()))
        return m_archetypeNodes.at(index);
    return nullptr;
}

std::vector<lyric_parser::ArchetypeNode *>::const_iterator
lyric_parser::ArchetypeState::nodesBegin() const
{
    return m_archetypeNodes.cbegin();
}

std::vector<lyric_parser::ArchetypeNode *>::const_iterator
lyric_parser::ArchetypeState::nodesEnd() const
{
    return m_archetypeNodes.cend();
}

int
lyric_parser::ArchetypeState::numNodes() const
{
    return m_archetypeNodes.size();
}

void
lyric_parser::ArchetypeState::pushSymbol(const std::string &identifier)
{
    m_symbolStack.push_back(identifier);
}

std::string
lyric_parser::ArchetypeState::popSymbol()
{
    if (m_symbolStack.empty())
        throw tempo_utils::StatusException(
            ParseStatus::forCondition(ParseCondition::kParseInvariant,
                "symbol stack is empty"));
    auto identifier = m_symbolStack.back();
    m_symbolStack.pop_back();
    return identifier;
}

std::string
lyric_parser::ArchetypeState::popSymbolAndCheck(std::string_view checkIdentifier)
{
    if (m_symbolStack.empty())
        throw tempo_utils::StatusException(
            ParseStatus::forCondition(ParseCondition::kParseInvariant,
                "symbol stack is empty"));
    auto identifier = m_symbolStack.back();
    if (identifier != checkIdentifier)
        throw tempo_utils::StatusException(
            ParseStatus::forCondition(ParseCondition::kParseInvariant,
                "unexpected symbol on top of symbol stack"));
    m_symbolStack.pop_back();
    return identifier;
}

std::string
lyric_parser::ArchetypeState::peekSymbol()
{
    if (m_symbolStack.empty())
        return {};
    return m_symbolStack.back();
}

std::vector<std::string>
lyric_parser::ArchetypeState::currentSymbolPath() const
{
    return m_symbolStack;
}

std::string
lyric_parser::ArchetypeState::currentSymbolString() const
{
    std::string symbolString;
    if (m_symbolStack.empty())
        return symbolString;
    auto iterator = m_symbolStack.cbegin();
    symbolString = *iterator;
    for (; iterator != m_symbolStack.cend(); iterator++) {
        symbolString.push_back('.');
        symbolString.append(*iterator);
    }
    return symbolString;
}

tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::ArchetypeState::toArchetype() const
{
    return internal::ArchetypeWriter::createArchetype(this);

//    flatbuffers::FlatBufferBuilder buffer;
//
//    std::vector<flatbuffers::Offset<lyi1::NamespaceDescriptor>> namespaces_vector;
//    std::vector<flatbuffers::Offset<lyi1::AttributeDescriptor>> attributes_vector;
//    std::vector<flatbuffers::Offset<lyi1::NodeDescriptor>> nodes_vector;
//
//    // serialize namespaces
//    for (const auto *ns : m_archetypeNamespaces) {
//        auto fb_nsUrl = buffer.CreateString(ns->getNsUrl().toString());
//        namespaces_vector.push_back(lyi1::CreateNamespaceDescriptor(buffer, fb_nsUrl));
//    }
//    auto fb_namespaces = buffer.CreateVector(namespaces_vector);
//
//    // serialize attributes
//    for (const auto *attr : m_archetypeAttrs) {
//        auto id = attr->getAttrId();
//        auto value = attr->getAttrValue();
//        auto p = serialize_value(buffer, value);
//
//        attributes_vector.push_back(lyi1::CreateAttributeDescriptor(buffer,
//            id.getAddress().getAddress(), id.getType(), p.first, p.second));
//    }
//    auto fb_attributes = buffer.CreateVector(attributes_vector);
//
//    // serialize nodes
//    for (const auto *node : m_archetypeNodes) {
//
//        // serialize entry attrs
//        std::vector<tu_uint32> node_attrs;
//        for (auto iterator = node->attrsBegin(); iterator != node->attrsEnd(); iterator++) {
//            node_attrs.push_back(iterator->second.getAddress());
//        }
//        auto fb_node_attrs = buffer.CreateVector(node_attrs);
//
//        // serialize entry children
//        std::vector<tu_uint32> node_children;
//        for (auto iterator = node->childrenBegin(); iterator != node->childrenEnd(); iterator++) {
//            node_children.push_back(iterator->getAddress());
//        }
//        auto fb_node_children = buffer.CreateVector(node_children);
//
//        nodes_vector.push_back(lyi1::CreateNodeDescriptor(buffer,
//            node->getNsOffset(), node->getTypeOffset(),
//            fb_node_attrs, fb_node_children,
//            node->getFileOffset(), node->getLineNumber(), node->getColumnNumber(), node->getTextSpan()));
//    }
//    auto fb_nodes = buffer.CreateVector(nodes_vector);
//
//    // build archetype from buffer
//    lyi1::ArchetypeBuilder archetypeBuilder(buffer);
//
//    archetypeBuilder.add_abi(lyi1::ArchetypeVersion::Version1);
//    archetypeBuilder.add_namespaces(fb_namespaces);
//    archetypeBuilder.add_attributes(fb_attributes);
//    archetypeBuilder.add_nodes(fb_nodes);
//
//    // serialize archetype and mark the buffer as finished
//    auto archetype = archetypeBuilder.Finish();
//    buffer.Finish(archetype, lyi1::ArchetypeIdentifier());
//
//    // copy the flatbuffer into our own byte array and instantiate archetype
//    auto bytes = tempo_utils::MemoryBytes::copy(buffer.GetBufferSpan());
//    return lyric_parser::LyricArchetype(bytes);
}
