
#include <lyric_serde/internal/patchset_reader.h>
#include <lyric_serde/value_walker.h>

lyric_serde::ValueWalker::ValueWalker()
    : m_reader(),
      m_index(0)
{
}

lyric_serde::ValueWalker::ValueWalker(std::shared_ptr<const internal::PatchsetReader> reader, tu_uint32 index)
    : m_reader(reader),
      m_index(index)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_serde::ValueWalker::ValueWalker(const ValueWalker &other)
    : m_reader(other.m_reader),
      m_index(other.m_index)
{
}

bool
lyric_serde::ValueWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_index != kInvalidOffsetU32;
}

tu_uint32
lyric_serde::ValueWalker::getIndex() const
{
    return m_index;
}

lyric_serde::ValueType
lyric_serde::ValueWalker::getValueType() const
{
    if (!isValid())
        return ValueType::Invalid;
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr)
        return ValueType::Invalid;
    switch (value->value_type()) {
        case lps1::Value::TrueFalseNilValue: {
            auto *v = value->value_as_TrueFalseNilValue();
            if (v == nullptr)
                return ValueType::Invalid;
            return v->tfn() == lps1::TrueFalseNil::Nil? ValueType::Nil : ValueType::Bool;
        }
        case lps1::Value::Int64Value:
            return ValueType::Int64;
        case lps1::Value::Float64Value:
            return ValueType::Float64;
        case lps1::Value::UInt64Value:
            return ValueType::UInt64;
        case lps1::Value::UInt32Value:
            return ValueType::UInt32;
        case lps1::Value::UInt16Value:
            return ValueType::UInt16;
        case lps1::Value::UInt8Value:
            return ValueType::UInt8;
        case lps1::Value::StringValue:
            return ValueType::String;
        case lps1::Value::AttrValue:
            return ValueType::Attr;
        case lps1::Value::ElementValue:
            return ValueType::Element;
        default:
            return ValueType::Invalid;
    }
}

bool
lyric_serde::ValueWalker::isNil() const
{
    return getValueType() == ValueType::Nil;
}

bool
lyric_serde::ValueWalker::getBool() const
{
    if (!isValid())
        return false;
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::TrueFalseNilValue)
        return false;
    auto *v = value->value_as_TrueFalseNilValue();
    if (v == nullptr)
        return false;
    return v->tfn() == lps1::TrueFalseNil::True;
}

tu_int64
lyric_serde::ValueWalker::getInt64() const
{
    if (!isValid())
        return 0;
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::Int64Value)
        return 0;
    auto *v = value->value_as_Int64Value();
    if (v == nullptr)
        return 0;
    return v->i64();
}

double
lyric_serde::ValueWalker::getFloat64() const
{
    if (!isValid())
        return 0;
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::Float64Value)
        return 0;
    auto *v = value->value_as_Float64Value();
    if (v == nullptr)
        return 0;
    return v->f64();
}

tu_uint64
lyric_serde::ValueWalker::getUInt64() const
{
    if (!isValid())
        return 0;
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::UInt64Value)
        return 0;
    auto *v = value->value_as_UInt64Value();
    if (v == nullptr)
        return 0;
    return v->u64();
}

tu_uint32
lyric_serde::ValueWalker::getUInt32() const
{
    if (!isValid())
        return 0;
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::UInt32Value)
        return 0;
    auto *v = value->value_as_UInt32Value();
    if (v == nullptr)
        return 0;
    return v->u32();
}

tu_uint16
lyric_serde::ValueWalker::getUInt16() const
{
    if (!isValid())
        return 0;
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::UInt16Value)
        return 0;
    auto *v = value->value_as_UInt16Value();
    if (v == nullptr)
        return 0;
    return v->u16();
}

tu_uint8
lyric_serde::ValueWalker::getUInt8() const
{
    if (!isValid())
        return 0;
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::UInt8Value)
        return 0;
    auto *v = value->value_as_UInt8Value();
    if (v == nullptr)
        return 0;
    return v->u8();
}

std::string
lyric_serde::ValueWalker::getString() const
{
    if (!isValid())
        return 0;
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::StringValue)
        return {};
    auto *v = value->value_as_StringValue();
    if (v == nullptr || v->utf8() == nullptr)
        return {};
    return v->utf8()->str();
}

lyric_serde::NamespaceWalker
lyric_serde::ValueWalker::getAttrNamespace() const
{
    if (!isValid())
        return {};
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::AttrValue)
        return {};
    auto *attrValue = value->value_as_AttrValue();
    if (attrValue->ns() < 0)
        return {};
    return NamespaceWalker(m_reader, attrValue->ns());
}

lyric_serde::Resource
lyric_serde::ValueWalker::getAttrResource() const
{
    if (!isValid())
        return {0, kInvalidOffsetU32};
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::AttrValue)
        return {0, kInvalidOffsetU32};
    auto *v = value->value_as_AttrValue();
    if (v == nullptr)
        return {0, kInvalidOffsetU32};
    return {v->ns(), v->id()};
}

lyric_serde::ValueWalker
lyric_serde::ValueWalker::getAttrValue() const
{
    if (!isValid())
        return {};
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::AttrValue)
        return {};
    auto *v = value->value_as_AttrValue();
    if (v == nullptr)
        return {};
    return ValueWalker(m_reader, v->value());
}

lyric_serde::NamespaceWalker
lyric_serde::ValueWalker::getElementNamespace() const
{
    if (!isValid())
        return {};
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::ElementValue)
        return {};
    auto *elementValue = value->value_as_ElementValue();
    if (elementValue->ns() < 0)
        return {};
    return NamespaceWalker(m_reader, elementValue->ns());
}

lyric_serde::Resource
lyric_serde::ValueWalker::getElementResource() const
{
    if (!isValid())
        return {0, kInvalidOffsetU32};
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::ElementValue)
        return {0, kInvalidOffsetU32};
    auto *v = value->value_as_ElementValue();
    if (v == nullptr)
        return {0, kInvalidOffsetU32};
    return {v->ns(), v->id()};
}

lyric_serde::ValueWalker
lyric_serde::ValueWalker::getElementChild(tu_uint32 index) const
{
    if (!isValid())
        return {};
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::ElementValue)
        return {};
    auto *v = value->value_as_ElementValue();
    if (v == nullptr || v->children() == nullptr || v->children()->size() <= index)
        return {};
    return ValueWalker(m_reader, v->children()->Get(index));
}

int
lyric_serde::ValueWalker::numElementChildren() const
{
    if (!isValid())
        return 0;
    auto *value = m_reader->getValue(m_index);
    if (value == nullptr || value->value_type() != lps1::Value::ElementValue)
        return 0;
    auto *v = value->value_as_ElementValue();
    if (v == nullptr || v->children() == nullptr)
        return 0;
    return v->children()->size();
}