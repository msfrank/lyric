
#include <lyric_parser/internal/archetype_reader.h>
#include <lyric_parser/node_walker.h>

lyric_parser::NodeWalker::NodeWalker()
    : m_index(INVALID_ADDRESS_U32)
{
}

lyric_parser::NodeWalker::NodeWalker(std::shared_ptr<const internal::ArchetypeReader> reader, tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_index != INVALID_ADDRESS_U32);
}

lyric_parser::NodeWalker::NodeWalker(const NodeWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
lyric_parser::NodeWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_index != INVALID_ADDRESS_U32;
}

lyric_parser::NodeAddress
lyric_parser::NodeWalker::getAddress() const
{
    return NodeAddress(m_index);
}

lyric_parser::ParseLocation
lyric_parser::NodeWalker::getLocation() const
{
    if (!isValid())
        return {};
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    return ParseLocation(node->line_nr(), node->column_nr(), node->file_offset(), node->text_span());

}

uint32_t
lyric_parser::NodeWalker::getLineNumber() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    return node->line_nr();
}

uint32_t
lyric_parser::NodeWalker::getColumnNumber() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    return node->column_nr();
}

uint32_t
lyric_parser::NodeWalker::getFileOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    return node->file_offset();
}

uint32_t
lyric_parser::NodeWalker::getTextSpan() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    return node->text_span();
}

std::string_view
lyric_parser::NodeWalker::namespaceView() const
{
    if (!isValid())
        return {};
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    auto *ns = m_reader->getNamespace(node->node_ns());
    TU_ASSERT (ns != nullptr);
    return ns->ns_url()->string_view();
}

bool
lyric_parser::NodeWalker::isNamespace(const tempo_schema::SchemaNs &schemaNs) const
{
    if (!isValid())
        return false;
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    auto *ns = m_reader->getNamespace(node->node_ns());
    TU_ASSERT (ns != nullptr);
    auto *nsUrl = ns->ns_url();
    TU_ASSERT (nsUrl != nullptr);
    return std::string_view(schemaNs.getNs()) == nsUrl->string_view();
}

bool
lyric_parser::NodeWalker::matchesNsAndId(const char *nsUrl, tu_uint32 idValue) const
{
    if (!isValid())
        return false;
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    auto *ns = m_reader->getNamespace(node->node_ns());
    TU_ASSERT (ns != nullptr);
    auto *nsUrl__ = ns->ns_url();
    TU_ASSERT (nsUrl__ != nullptr);
    return std::string_view(nsUrl) == nsUrl__->string_view() && idValue == node->node_id();
}

const char *
lyric_parser::NodeWalker::getNsUrl() const
{
    if (!isValid())
        return nullptr;
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    auto *ns = m_reader->getNamespace(node->node_ns());
    TU_ASSERT (ns != nullptr);
    return ns->ns_url()->c_str();
}

tu_uint32
lyric_parser::NodeWalker::getIdValue() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    return node->node_id();
}

bool
lyric_parser::NodeWalker::hasAttr(const tempo_schema::AttrKey &key) const
{
    auto index = findIndexForAttr(key);
    return index != INVALID_ADDRESS_U32;
}

bool
lyric_parser::NodeWalker::hasAttr(const tempo_schema::AttrValidator &validator) const
{
    return hasAttr(validator.getKey());
}

tu_uint32
lyric_parser::NodeWalker::findIndexForAttr(const tempo_schema::AttrKey &key) const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    auto *attrs = node->node_attrs();
    if (attrs == nullptr)    // node has no attrs
        return INVALID_ADDRESS_U32;
    for (const auto attrIndex : *attrs) {
        auto *attr = m_reader->getAttr(attrIndex);
        TU_ASSERT (attr != nullptr);
        auto *ns = m_reader->getNamespace(attr->attr_ns());
        TU_ASSERT (ns != nullptr);
        auto *nsUrl = ns->ns_url();
        if (nsUrl == nullptr)
            continue;
        if (std::string_view(key.ns) == nsUrl->string_view() && key.id == attr->attr_id())
            return attrIndex;
    }
    return INVALID_ADDRESS_U32;
}

tempo_schema::AttrValue parse_attr_value(const lyi1::AttrDescriptor *attr)
{
    switch (attr->attr_value_type()) {
        case lyi1::Value::TrueFalseNilValue: {
            auto tfn = attr->attr_value_as_TrueFalseNilValue()->tfn();
            if (tfn == lyi1::TrueFalseNil::Nil)
                return tempo_schema::AttrValue(nullptr);
            return tempo_schema::AttrValue(tfn == lyi1::TrueFalseNil::True);
        }
        case lyi1::Value::Int64Value:
            return tempo_schema::AttrValue(attr->attr_value_as_Int64Value()->i64());
        case lyi1::Value::Float64Value:
            return tempo_schema::AttrValue(attr->attr_value_as_Float64Value()->f64());
        case lyi1::Value::UInt64Value:
            return tempo_schema::AttrValue(attr->attr_value_as_UInt64Value()->u64());
        case lyi1::Value::UInt32Value:
            return tempo_schema::AttrValue(attr->attr_value_as_UInt32Value()->u32());
        case lyi1::Value::UInt16Value:
            return tempo_schema::AttrValue(attr->attr_value_as_UInt16Value()->u16());
        case lyi1::Value::UInt8Value:
            return tempo_schema::AttrValue(attr->attr_value_as_UInt8Value()->u8());
        case lyi1::Value::StringValue:
            return tempo_schema::AttrValue(attr->attr_value_as_StringValue()->utf8()->c_str());
        case lyi1::Value::NodeValue:
            return tempo_schema::AttrValue(tempo_schema::AttrHandle{attr->attr_value_as_NodeValue()->node()});
        default:
            return {};
    }
}

std::pair<tempo_schema::AttrKey,tempo_schema::AttrValue>
lyric_parser::NodeWalker::getAttr(int index) const
{
    if (!isValid())
        return {};
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    auto *attrs = node->node_attrs();
    if (attrs == nullptr)    // node has no attrs
        return {};
    if (index < 0 || attrs->size() <= index)
        return {};
    tu_uint32 attr_index = attrs->Get(index);
    auto *attr = m_reader->getAttr(attr_index);
    TU_ASSERT (attr != nullptr);
    auto *ns = m_reader->getNamespace(attr->attr_ns());
    TU_ASSERT (ns != nullptr);
    auto *nsUrl = ns->ns_url();
    if (nsUrl == nullptr)
        return {};
    tempo_schema::AttrKey key{nsUrl->c_str(), attr->attr_id()};
    auto value = parse_attr_value(attr);
    return {key,value};
}

int
lyric_parser::NodeWalker::numAttrs() const
{
    if (!isValid())
        return 0;
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    auto *attrs = node->node_attrs();
    if (attrs == nullptr)    // node has no attrs
        return 0;
    return attrs->size();
}

lyric_parser::NodeWalker
lyric_parser::NodeWalker::getChild(tu_uint32 index) const
{
    if (!isValid())
        return {};
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    auto *children = node->node_children();
    if (children == nullptr)    // span has no children
        return {};
    if (children->size() <= index)
        return {};
    return NodeWalker(m_reader, children->Get(index));
}

int
lyric_parser::NodeWalker::numChildren() const
{
    if (!isValid())
        return 0;
    auto *node = m_reader->getNode(m_index);
    TU_ASSERT (node != nullptr);
    auto *children = node->node_children();
    if (children == nullptr)    // span has no children
        return 0;
    return children->size();
}

lyric_parser::NodeWalker
lyric_parser::NodeWalker::getNodeAtOffset(tu_uint32 offset) const
{
    if (!isValid())
        return {};
    if (m_reader->numNodes() <= offset)
        return {};
    return NodeWalker(m_reader, offset);
}