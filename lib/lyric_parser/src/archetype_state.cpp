
#include <absl/container/flat_hash_map.h>

#include <lyric_parser/archetype_attr.h>
#include <lyric_parser/archetype_namespace.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/archetype_state.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/generated/archetype.h>
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

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeState::makeSType(ModuleParser::SimpleTypeContext *ctx)
{
    TU_ASSERT (ctx != nullptr);
    auto *token = ctx->getStart();

    std::vector<std::string> parts;
    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        auto *part = ctx->Identifier(i);
        if (part == nullptr)
            continue;
        parts.push_back(part->getText());
    }
    lyric_common::SymbolPath symbolPath(parts);

    auto *symbolPathAttr = appendAttrOrThrow(kLyricAstSymbolPath, symbolPath);
    auto *stypeNode = appendNodeOrThrow(lyric_schema::kLyricAstSTypeClass, token);
    stypeNode->putAttr(symbolPathAttr);
    return stypeNode;
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeState::makePType(ModuleParser::ParametricTypeContext *ctx)
{
    TU_ASSERT (ctx != nullptr);
    auto *token = ctx->getStart();

    auto *simpleType = ctx->simpleType();
    std::vector<std::string> parts;
    for (size_t i = 0; i < simpleType->getRuleIndex(); i++) {
        auto *part = simpleType->Identifier(i);
        if (part == nullptr)
            continue;
        parts.push_back(part->getText());
    }
    lyric_common::SymbolPath symbolPath(parts);

    auto *symbolPathAttr = appendAttrOrThrow(kLyricAstSymbolPath, symbolPath);
    auto *ptypeNode = appendNodeOrThrow(lyric_schema::kLyricAstPTypeClass, token);
    ptypeNode->putAttr(symbolPathAttr);

    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        auto *param = ctx->assignableType(i);
        if (param == nullptr)
            continue;
        auto *typeNode = makeType(param);
        ptypeNode->appendChild(typeNode);
    }

    return ptypeNode;
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeState::makeUType(ModuleParser::UnionTypeContext *ctx)
{
    TU_ASSERT (ctx != nullptr);
    auto *token = ctx->getStart();

    auto *utypeNode = appendNodeOrThrow(lyric_schema::kLyricAstUTypeClass, token);

    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        auto *member = ctx->singularType(i);
        if (member == nullptr)
            continue;

        ArchetypeNode *memberNode = nullptr;
        if (member->parametricType()) {
            memberNode = makePType(member->parametricType());
        } else if (member->simpleType()) {
            memberNode = makeSType(member->simpleType());
        } else {
            throwSyntaxError(member->getStart(), "illegal union type member");
            TU_UNREACHABLE();
        }

        utypeNode->appendChild(memberNode);
    }

    return utypeNode;
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeState::makeIType(ModuleParser::IntersectionTypeContext *ctx)
{
    TU_ASSERT (ctx != nullptr);
    auto *token = ctx->getStart();

    auto *itypeNode = appendNodeOrThrow(lyric_schema::kLyricAstITypeClass, token);

    for (size_t i = 0; i < ctx->getRuleIndex(); i++) {
        auto *member = ctx->singularType(i);
        if (member == nullptr)
            continue;

        ArchetypeNode *memberNode = nullptr;
        if (member->parametricType()) {
            memberNode = makePType(member->parametricType());
        } else if (member->simpleType()) {
            memberNode = makeSType(member->simpleType());
        } else {
            throwSyntaxError(member->getStart(), "illegal intersection type member");
            TU_UNREACHABLE();
        }

        itypeNode->appendChild(memberNode);
    }

    return itypeNode;
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeState::makeType(ModuleParser::AssignableTypeContext *ctx)
{
    if (ctx->singularType()) {
        if (ctx->singularType()->parametricType())
            return makePType(ctx->singularType()->parametricType());
        return makeSType(ctx->singularType()->simpleType());
    } else if (ctx->intersectionType()) {
        return makeIType(ctx->intersectionType());
    } else if (ctx->unionType()) {
        return makeUType(ctx->unionType());
    }
    throwSyntaxError(ctx->getStart(), "illegal assignable type");
    TU_UNREACHABLE();
}

lyric_parser::ArchetypeNode *
lyric_parser::ArchetypeState::makeGeneric(
    ModuleParser::PlaceholderSpecContext *pctx,
    ModuleParser::ConstraintSpecContext *cctx)
{
    TU_ASSERT (pctx != nullptr);
    auto *token = pctx->getStart();

    auto *genericNode = appendNodeOrThrow(lyric_schema::kLyricAstGenericClass, token);

    absl::flat_hash_map<std::string,ArchetypeNode *> placeholderNodes;

    for (size_t i = 0; i < pctx->getRuleIndex(); i++) {
        if (!pctx->placeholder(i))
            continue;
        auto *p = pctx->placeholder(i);

        std::string id;
        VarianceType variance;

        if (p->covariantPlaceholder()) {
            id = p->covariantPlaceholder()->Identifier()->getText();
            variance = VarianceType::COVARIANT;
        } else if (p->contravariantPlaceholder()) {
            id = p->contravariantPlaceholder()->Identifier()->getText();
            variance = VarianceType::CONTRAVARIANT;
        } else if (p->invariantPlaceholder()) {
            id = p->invariantPlaceholder()->Identifier()->getText();
            variance = VarianceType::INVARIANT;
        } else {
            throwParseInvariant(genericNode->getAddress(), p->getStart(), "illegal placeholder");
            TU_UNREACHABLE();
        }

        auto *identifierAttr = appendAttrOrThrow(kLyricAstIdentifier, id);
        auto *varianceAttr = appendAttrOrThrow(kLyricAstVarianceType, variance);

        token = p->getStart();

        auto *placeholderNode = appendNodeOrThrow(lyric_schema::kLyricAstPlaceholderClass, token);
        placeholderNode->putAttr(identifierAttr);
        placeholderNode->putAttr(varianceAttr);
        genericNode->appendChild(placeholderNode);

        placeholderNodes[id] = placeholderNode;
    }

    if (cctx) {
        for (size_t i = 0; i < cctx->getRuleIndex(); i++) {
            if (!cctx->constraint(i))
                continue;
            auto *c = cctx->constraint(i);

            std::string id;
            BoundType bound;
            ArchetypeNode *constraintTypeNode;

            if (c->extendsConstraint()) {
                id = c->extendsConstraint()->Identifier()->getText();
                bound = BoundType::EXTENDS;
                constraintTypeNode = makeType(c->extendsConstraint()->assignableType());
            } else if (c->superConstraint()) {
                id = c->superConstraint()->Identifier()->getText();
                bound = BoundType::SUPER;
                constraintTypeNode = makeType(c->extendsConstraint()->assignableType());
            } else {
                throwParseInvariant(genericNode->getAddress(), c->getStart(), "illegal constraint");
                TU_UNREACHABLE();
            }

            auto *boundAttr = appendAttrOrThrow(kLyricAstBoundType, bound);
            auto *typeOffsetAttr = appendAttrOrThrow(kLyricAstTypeOffset, constraintTypeNode->getAddress().getAddress());

            token = c->getStart();

            auto *constraintNode = appendNodeOrThrow(lyric_schema::kLyricAstConstraintClass, token);
            constraintNode->putAttr(boundAttr);
            constraintNode->putAttr(typeOffsetAttr);

            if (!placeholderNodes.contains(id))
                throwSyntaxError(c->getStop(), "no such placeholder {} for constraint", id);
            auto *placeholderNode = placeholderNodes.at(id);
            placeholderNode->appendChild(constraintNode);
        }
    }

    return genericNode;
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
    NamespaceAddress address(m_archetypeNamespaces.size());
    auto *ns = new ArchetypeNamespace(nsUrl, address, this);
    m_archetypeNamespaces.push_back(ns);
    m_namespaceIndex[nsUrl] = address.getAddress();
    return ns;
}

tempo_utils::Result<tu_uint32>
lyric_parser::ArchetypeState::putNamespace(const char *nsString)
{
    auto nsUrl = tempo_utils::Url::fromString(nsString);
    auto putNamespaceResult = putNamespace(nsUrl);
    if (putNamespaceResult.isStatus())
        return putNamespaceResult.getStatus();
    auto *archetypeNs = putNamespaceResult.getResult();
    return archetypeNs->getAddress().getAddress();
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
lyric_parser::ArchetypeState::appendAttr(AttrId id, tempo_utils::AttrValue value)
{
    AttrAddress address(m_archetypeAttrs.size());
    auto *attr = new ArchetypeAttr(id, value, address, this);
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
lyric_parser::ArchetypeState::appendNode(tu_uint32 nodeNs, tu_uint32 nodeId, antlr4::Token *token)
{
    NodeAddress address(m_archetypeNodes.size());
    auto *node = new ArchetypeNode(nodeNs, nodeId, token, address, this);
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

static std::pair<lyi1::Value,flatbuffers::Offset<void>>
serialize_value(flatbuffers::FlatBufferBuilder &buffer, const tempo_utils::AttrValue &value)
{
    switch (value.getType()) {
        case tempo_utils::ValueType::Nil: {
            auto type = lyi1::Value::TrueFalseNilValue;
            auto offset = lyi1::CreateTrueFalseNilValue(buffer, lyi1::TrueFalseNil::Nil).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::Bool: {
            auto type = lyi1::Value::TrueFalseNilValue;
            auto tfn = value.getBool()? lyi1::TrueFalseNil::True : lyi1::TrueFalseNil::False;
            auto offset = lyi1::CreateTrueFalseNilValue(buffer, tfn).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::Int64: {
            auto type = lyi1::Value::Int64Value;
            auto offset = lyi1::CreateInt64Value(buffer, value.getInt64()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::Float64: {
            auto type = lyi1::Value::Float64Value;
            auto offset = lyi1::CreateFloat64Value(buffer, value.getFloat64()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::UInt64: {
            auto type = lyi1::Value::UInt64Value;
            auto offset = lyi1::CreateUInt64Value(buffer, value.getUInt64()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::UInt32: {
            auto type = lyi1::Value::UInt32Value;
            auto offset = lyi1::CreateUInt32Value(buffer, value.getUInt32()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::UInt16: {
            auto type = lyi1::Value::UInt16Value;
            auto offset = lyi1::CreateUInt16Value(buffer, value.getUInt16()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::UInt8: {
            auto type = lyi1::Value::UInt8Value;
            auto offset = lyi1::CreateUInt8Value(buffer, value.getUInt8()).Union();
            return {type, offset};
        }
        case tempo_utils::ValueType::String: {
            auto type = lyi1::Value::StringValue;
            auto offset = lyi1::CreateStringValue(buffer, buffer.CreateSharedString(value.stringView())).Union();
            return {type, offset};
        }
        default:
            TU_UNREACHABLE();
    }
}

tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::ArchetypeState::toArchetype() const
{
    flatbuffers::FlatBufferBuilder buffer;

    std::vector<flatbuffers::Offset<lyi1::NamespaceDescriptor>> namespaces_vector;
    std::vector<flatbuffers::Offset<lyi1::AttributeDescriptor>> attributes_vector;
    std::vector<flatbuffers::Offset<lyi1::NodeDescriptor>> nodes_vector;

    // serialize namespaces
    for (const auto *ns : m_archetypeNamespaces) {
        auto fb_nsUrl = buffer.CreateString(ns->getNsUrl().toString());
        namespaces_vector.push_back(lyi1::CreateNamespaceDescriptor(buffer, fb_nsUrl));
    }
    auto fb_namespaces = buffer.CreateVector(namespaces_vector);

    // serialize attributes
    for (const auto *attr : m_archetypeAttrs) {
        auto id = attr->getAttrId();
        auto value = attr->getAttrValue();
        auto p = serialize_value(buffer, value);

        attributes_vector.push_back(lyi1::CreateAttributeDescriptor(buffer,
            id.getAddress().getAddress(), id.getType(), p.first, p.second));
    }
    auto fb_attributes = buffer.CreateVector(attributes_vector);

    // serialize nodes
    for (const auto *node : m_archetypeNodes) {

        // serialize entry attrs
        std::vector<uint32_t> node_attrs;
        for (auto iterator = node->attrsBegin(); iterator != node->attrsEnd(); iterator++) {
            node_attrs.push_back(iterator->second.getAddress());
        }
        auto fb_node_attrs = buffer.CreateVector(node_attrs);

        // serialize entry children
        std::vector<uint32_t> node_children;
        for (auto iterator = node->childrenBegin(); iterator != node->childrenEnd(); iterator++) {
            node_children.push_back(iterator->getAddress());
        }
        auto fb_node_children = buffer.CreateVector(node_children);

        nodes_vector.push_back(lyi1::CreateNodeDescriptor(buffer,
            node->getNsOffset(), node->getTypeOffset(),
            fb_node_attrs, fb_node_children,
            node->getFileOffset(), node->getLineNumber(), node->getColumnNumber(), node->getTextSpan()));
    }
    auto fb_nodes = buffer.CreateVector(nodes_vector);

    // build archetype from buffer
    lyi1::ArchetypeBuilder archetypeBuilder(buffer);

    archetypeBuilder.add_abi(lyi1::ArchetypeVersion::Version1);
    archetypeBuilder.add_namespaces(fb_namespaces);
    archetypeBuilder.add_attributes(fb_attributes);
    archetypeBuilder.add_nodes(fb_nodes);

    // serialize archetype and mark the buffer as finished
    auto archetype = archetypeBuilder.Finish();
    buffer.Finish(archetype, lyi1::ArchetypeIdentifier());

    // copy the flatbuffer into our own byte array and instantiate archetype
    auto bytes = tempo_utils::MemoryBytes::copy(buffer.GetBufferSpan());
    return lyric_parser::LyricArchetype(bytes);
}
