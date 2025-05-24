
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
      m_scopeManager(scopeManager),
      m_rootNode(nullptr)
{
}

tempo_utils::Result<lyric_parser::ArchetypeAttr *>
lyric_parser::ArchetypeState::loadAttr(
    const lyric_parser::LyricArchetype &archetype,
    const tempo_schema::AttrKey &key,
    const tempo_schema::AttrValue &value,
    lyric_parser::ArchetypeState *state,
    std::vector<lyric_parser::ArchetypeNode *> &nodeTable)
{
    lyric_parser::ArchetypeNamespace *attrNamespace;
    TU_ASSIGN_OR_RETURN (attrNamespace, state->putNamespace(key.ns));
    lyric_parser::AttrId attrId(attrNamespace, key.id);

    if (value.getType() == tempo_schema::ValueType::Handle) {
        auto handleNode = archetype.getNode(value.getHandle().handle);
        TU_ASSERT (handleNode.isValid());
        lyric_parser::ArchetypeNode *attrNode;
        TU_ASSIGN_OR_RETURN (attrNode, loadNode(archetype, handleNode, state, nodeTable));
        return state->appendAttr(attrId, lyric_parser::AttrValue(attrNode));
    }

    return state->appendAttr(attrId, lyric_parser::AttrValue(value));
}

tempo_utils::Result<lyric_parser::ArchetypeNode *>
lyric_parser::ArchetypeState::loadNode(
    const lyric_parser::LyricArchetype &archetype,
    const lyric_parser::NodeWalker &node,
    lyric_parser::ArchetypeState *state,
    std::vector<lyric_parser::ArchetypeNode *> &nodeTable)
{
    auto address = node.getAddress().getAddress();

    // if node has already been loaded then return it immediately
    auto *existingNode = nodeTable.at(address);
    if (existingNode != nullptr)
        return existingNode;

    // load the node namespace
    lyric_parser::ArchetypeNamespace *nodeNamespace;
    TU_ASSIGN_OR_RETURN (nodeNamespace, state->putNamespace(node.namespaceView()));

    // load the node
    lyric_parser::ParseLocation location(
        node.getLineNumber(), node.getColumnNumber(), node.getFileOffset(), node.getTextSpan());
    lyric_parser::ArchetypeNode *archetypeNode;
    TU_ASSIGN_OR_RETURN (archetypeNode, state->appendNode(nodeNamespace, node.getIdValue(), location));

    tu_uint32 offset = m_archetypeNodes.size();
    auto *archetypeId = makeId(ArchetypeDescriptorType::Node, offset);
    archetypeNode = new ArchetypeNode(node, nodeNamespace, node.getIdValue(), location, archetypeId, this);
    m_archetypeNodes.push_back(archetypeNode);

    nodeTable[address] = archetypeNode;

    // load the node attrs
    for (int i = 0; i < node.numAttrs(); i++) {
        auto attr = node.getAttr(i);
        lyric_parser::ArchetypeAttr *archetypeAttr;
        TU_ASSIGN_OR_RETURN (archetypeAttr, loadAttr(archetype, attr.first, attr.second, state, nodeTable));
        archetypeNode->putAttr(archetypeAttr);
    }

    // load the node children
    for (int i = 0; i < node.numChildren(); i++) {
        auto childNode = node.getChild(i);
        lyric_parser::ArchetypeNode *child;
        TU_ASSIGN_OR_RETURN (child, loadNode(archetype, childNode, state, nodeTable));
        archetypeNode->appendChild(child);
    }

    return archetypeNode;
}

tempo_utils::Result<lyric_parser::ArchetypeNode *>
lyric_parser::ArchetypeState::load(const LyricArchetype &archetype)
{
    if (!archetype.isValid())
        return ParseStatus::forCondition(
            ParseCondition::kParseInvariant, "failed to load archetype state: invalid archetype");

    std::vector<ArchetypeNode *> nodeTable(archetype.numNodes(), nullptr);

    for (tu_uint32 i = 0; i < archetype.numPragmas(); i++) {
        auto pragma = archetype.getPragma(i);
        ArchetypeNode *pragmaNode;
        TU_ASSIGN_OR_RETURN (pragmaNode, loadNode(archetype, pragma, this, nodeTable));
        addPragma(pragmaNode);
    }

    return loadNode(archetype, archetype.getRoot(), this, nodeTable);
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
    auto entry = m_namespaceIndex.find(nsUrl);
    if (entry == m_namespaceIndex.cend())
        return nullptr;
    auto nsOffset = entry->second;
    if (0 <= nsOffset && nsOffset < m_archetypeNamespaces.size())
        return m_archetypeNamespaces.at(nsOffset);
    return nullptr;
}

tempo_utils::Result<lyric_parser::ArchetypeNamespace *>
lyric_parser::ArchetypeState::putNamespace(const tempo_utils::Url &nsUrl)
{
    auto entry = m_namespaceIndex.find(nsUrl);
    if (entry != m_namespaceIndex.cend()) {
        auto offset = entry->second;
        TU_ASSERT (0 <= offset && offset < m_archetypeNamespaces.size());
        return m_archetypeNamespaces.at(offset);
    }
    tu_uint32 offset = m_archetypeNamespaces.size();
    auto *archetypeId = makeId(ArchetypeDescriptorType::Namespace, offset);
    auto *ns = new ArchetypeNamespace(nsUrl, archetypeId, this);
    m_archetypeNamespaces.push_back(ns);
    m_namespaceIndex[nsUrl] = archetypeId->getOffset();
    return ns;
}

tempo_utils::Result<lyric_parser::ArchetypeNamespace *>
lyric_parser::ArchetypeState::putNamespace(std::string_view nsView)
{
    auto nsUrl = tempo_utils::Url::fromString(nsView);
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

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeState::getPragma(int index) const
{
    if (0 <= index && std::cmp_less(index, m_pragmaNodes.size()))
        return m_pragmaNodes.at(index);
    return nullptr;
}

void
lyric_parser::ArchetypeState::addPragma(ArchetypeNode *pragmaNode)
{
    m_pragmaNodes.push_back(pragmaNode);
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeState::replacePragma(int index, ArchetypeNode *pragma)
{
    TU_ASSERT (pragma != nullptr);
    if (0 <= index && std::cmp_less(index, m_pragmaNodes.size())) {
        auto *prev = m_pragmaNodes.at(index);
        m_pragmaNodes[index] = pragma;
        return prev;
    }
    return nullptr;
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeState::removePragma(int index)
{
    if (0 <= index && std::cmp_less(index, m_pragmaNodes.size())) {
        auto iterator = m_pragmaNodes.begin();
        std::advance(iterator, index);
        auto *pragma = *iterator;
        m_pragmaNodes.erase(iterator);
        return pragma;
    }
    return nullptr;
}

void lyric_parser::ArchetypeState::clearPragmas()
{
    m_pragmaNodes.clear();
}

std::vector<lyric_parser::ArchetypeNode *>::const_iterator
lyric_parser::ArchetypeState::pragmasBegin() const
{
    return m_pragmaNodes.cbegin();
}

std::vector<lyric_parser::ArchetypeNode *>::const_iterator
lyric_parser::ArchetypeState::pragmasEnd() const
{
    return m_pragmaNodes.cend();
}

int
lyric_parser::ArchetypeState::numPragmas() const
{
    return m_pragmaNodes.size();
}

bool
lyric_parser::ArchetypeState::hasRoot() const
{
    return m_rootNode != nullptr;
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeState::getRoot() const
{
    return m_rootNode;
}

void
lyric_parser::ArchetypeState::setRoot(ArchetypeNode *node)
{
    m_rootNode = node;
}

void
lyric_parser::ArchetypeState::clearRoot()
{
    m_rootNode = nullptr;
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
}
