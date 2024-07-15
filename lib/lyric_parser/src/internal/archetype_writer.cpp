
#include <lyric_parser/archetype_attr.h>
#include <lyric_parser/archetype_namespace.h>
#include <lyric_parser/archetype_node.h>
#include <lyric_parser/internal/archetype_writer.h>
#include <tempo_utils/memory_bytes.h>

lyric_parser::internal::ArchetypeWriter::ArchetypeWriter(const ArchetypeState *state)
    : m_state(state)
{
    TU_ASSERT (m_state != nullptr);
    m_addressTable.assign(m_state->numIds(), INVALID_ADDRESS_U32);
}

tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::internal::ArchetypeWriter::createArchetype(const ArchetypeState *state)
{
    ArchetypeWriter writer(state);

    auto *root = state->getNode(0);
    if (root == nullptr)
        return lyric_parser::ParseStatus::forCondition(
            ParseCondition::kParseInvariant, "missing root node");

    TU_RETURN_IF_STATUS (writer.writeNode(root));
    return writer.writeArchetype();
}

tempo_utils::Result<lyric_parser::NamespaceAddress>
lyric_parser::internal::ArchetypeWriter::writeNamespace(const ArchetypeNamespace *ns)
{
    TU_ASSERT (ns != nullptr);
    auto *archetypeId = ns->getArchetypeId();
    TU_ASSERT (archetypeId->getType() == ArchetypeDescriptorType::Namespace);
    auto id = archetypeId->getId();

    if (m_addressTable[id] != INVALID_ADDRESS_U32)
        return NamespaceAddress(m_addressTable[id]);

    tu_uint32 address = m_namespacesVector.size();
    auto fb_nsUrl = m_buffer.CreateString(ns->getNsUrl().toString());
    m_namespacesVector.push_back(lyi1::CreateNamespaceDescriptor(m_buffer, fb_nsUrl));
    m_addressTable[id] = address;

    return NamespaceAddress(address);
}

tempo_utils::Result<std::pair<lyi1::Value,flatbuffers::Offset<void>>>
lyric_parser::internal::ArchetypeWriter::writeValue(const lyric_parser::AttrValue &value)
{
    if (value.isNode()) {
        NodeAddress nodeAddress;
        TU_ASSIGN_OR_RETURN (nodeAddress, writeNode(value.getNode()));
        auto type = lyi1::Value::NodeValue;
        auto offset = lyi1::CreateNodeValue(m_buffer, nodeAddress.getAddress()).Union();
        return std::pair{type, offset};
    }

    auto literal = value.getLiteral();
    switch (literal.getType()) {
        case tempo_utils::ValueType::Nil: {
            auto type = lyi1::Value::TrueFalseNilValue;
            auto offset = lyi1::CreateTrueFalseNilValue(m_buffer, lyi1::TrueFalseNil::Nil).Union();
            return std::pair{type, offset};
        }
        case tempo_utils::ValueType::Bool: {
            auto type = lyi1::Value::TrueFalseNilValue;
            auto tfn = literal.getBool()? lyi1::TrueFalseNil::True : lyi1::TrueFalseNil::False;
            auto offset = lyi1::CreateTrueFalseNilValue(m_buffer, tfn).Union();
            return std::pair{type, offset};
        }
        case tempo_utils::ValueType::Int64: {
            auto type = lyi1::Value::Int64Value;
            auto offset = lyi1::CreateInt64Value(m_buffer, literal.getInt64()).Union();
            return std::pair{type, offset};
        }
        case tempo_utils::ValueType::Float64: {
            auto type = lyi1::Value::Float64Value;
            auto offset = lyi1::CreateFloat64Value(m_buffer, literal.getFloat64()).Union();
            return std::pair{type, offset};
        }
        case tempo_utils::ValueType::UInt64: {
            auto type = lyi1::Value::UInt64Value;
            auto offset = lyi1::CreateUInt64Value(m_buffer, literal.getUInt64()).Union();
            return std::pair{type, offset};
        }
        case tempo_utils::ValueType::UInt32: {
            auto type = lyi1::Value::UInt32Value;
            auto offset = lyi1::CreateUInt32Value(m_buffer, literal.getUInt32()).Union();
            return std::pair{type, offset};
        }
        case tempo_utils::ValueType::UInt16: {
            auto type = lyi1::Value::UInt16Value;
            auto offset = lyi1::CreateUInt16Value(m_buffer, literal.getUInt16()).Union();
            return std::pair{type, offset};
        }
        case tempo_utils::ValueType::UInt8: {
            auto type = lyi1::Value::UInt8Value;
            auto offset = lyi1::CreateUInt8Value(m_buffer, literal.getUInt8()).Union();
            return std::pair{type, offset};
        }
        case tempo_utils::ValueType::String: {
            auto type = lyi1::Value::StringValue;
            auto str = m_buffer.CreateSharedString(literal.stringView());
            auto offset = lyi1::CreateStringValue(m_buffer, str).Union();
            return std::pair{type, offset};
        }
        default:
            return lyric_parser::ParseStatus::forCondition(
                ParseCondition::kParseInvariant, "invalid attr value");
    }
}

tempo_utils::Result<lyric_parser::AttrAddress>
lyric_parser::internal::ArchetypeWriter::writeAttr(const ArchetypeAttr *attr)
{
    TU_ASSERT (attr != nullptr);
    auto *archetypeId = attr->getArchetypeId();
    TU_ASSERT (archetypeId->getType() == ArchetypeDescriptorType::Attr);
    auto id = archetypeId->getId();

    if (m_addressTable[id] != INVALID_ADDRESS_U32)
        return AttrAddress(m_addressTable[id]);

    // allocate space for a new attr
    tu_uint32 address = m_attrsVector.size();
    m_addressTable[id] = address;
    m_attrsVector.emplace_back(0);

    auto attrId = attr->getAttrId();
    NamespaceAddress nsAddress;
    TU_ASSIGN_OR_RETURN (nsAddress, writeNamespace(attrId.getNamespace()));

    auto attrValue = attr->getAttrValue();
    std::pair<lyi1::Value,flatbuffers::Offset<void>> valueAndOffset;
    TU_ASSIGN_OR_RETURN (valueAndOffset, writeValue(attrValue));

    m_attrsVector[address] = lyi1::CreateAttrDescriptor(m_buffer,
        nsAddress.getAddress(), attrId.getType(), valueAndOffset.first, valueAndOffset.second);

    return AttrAddress(address);
}

tempo_utils::Result<lyric_parser::NodeAddress>
lyric_parser::internal::ArchetypeWriter::writeNode(const ArchetypeNode *node)
{
    TU_ASSERT (node != nullptr);
    auto *archetypeId = node->getArchetypeId();
    TU_ASSERT (archetypeId->getType() == ArchetypeDescriptorType::Node);
    auto id = archetypeId->getId();

    if (m_addressTable[id] != INVALID_ADDRESS_U32)
        return NodeAddress(m_addressTable[id]);

    // allocate space for a new node
    tu_uint32 address = m_nodesVector.size();
    m_addressTable[id] = address;
    m_nodesVector.emplace_back(0);

    // serialize namespace
    NamespaceAddress nsAddress;
    TU_ASSIGN_OR_RETURN (nsAddress, writeNamespace(node->getNamespace()));

    // serialize entry attrs
    std::vector<tu_uint32> node_attrs;
    for (auto iterator = node->attrsBegin(); iterator != node->attrsEnd(); iterator++) {
        AttrAddress attrAddress;
        TU_ASSIGN_OR_RETURN (attrAddress, writeAttr(iterator->second));
        node_attrs.push_back(attrAddress.getAddress());
    }
    auto fb_node_attrs = m_buffer.CreateVector(node_attrs);

    // serialize entry children
    std::vector<tu_uint32> node_children;
    for (auto iterator = node->childrenBegin(); iterator != node->childrenEnd(); iterator++) {
        NodeAddress childAddress;
        TU_ASSIGN_OR_RETURN (childAddress, writeNode(*iterator));
        node_children.push_back(childAddress.getAddress());
    }
    auto fb_node_children = m_buffer.CreateVector(node_children);

    auto location = node->getLocation();

    m_nodesVector[address] = lyi1::CreateNodeDescriptor(m_buffer,
        nsAddress.getAddress(), node->getTypeOffset(),
        fb_node_attrs, fb_node_children,
        location.fileOffset, location.lineNumber, location.columnNumber, location.textSpan);

    return NodeAddress(address);
}

tempo_utils::Result<lyric_parser::LyricArchetype>
lyric_parser::internal::ArchetypeWriter::writeArchetype()
{
    auto fb_namespaces = m_buffer.CreateVector(m_namespacesVector);
    auto fb_attrs = m_buffer.CreateVector(m_attrsVector);
    auto fb_nodes = m_buffer.CreateVector(m_nodesVector);

    // build archetype from buffer
    lyi1::ArchetypeBuilder archetypeBuilder(m_buffer);

    archetypeBuilder.add_abi(lyi1::ArchetypeVersion::Version1);
    archetypeBuilder.add_namespaces(fb_namespaces);
    archetypeBuilder.add_attrs(fb_attrs);
    archetypeBuilder.add_nodes(fb_nodes);

    // serialize archetype and mark the buffer as finished
    auto archetype = archetypeBuilder.Finish();
    m_buffer.Finish(archetype, lyi1::ArchetypeIdentifier());

    // copy the flatbuffer into our own byte array and instantiate archetype
    auto bytes = tempo_utils::MemoryBytes::copy(m_buffer.GetBufferSpan());
    return lyric_parser::LyricArchetype(bytes);
}
