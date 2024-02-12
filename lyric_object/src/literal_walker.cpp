
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/literal_walker.h>
#include <tempo_utils/big_endian.h>

lyric_object::LiteralWalker::LiteralWalker()
    : m_literalOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::LiteralWalker::LiteralWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    tu_uint32 literalOffset)
    : m_reader(reader),
      m_literalOffset(literalOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::LiteralWalker::LiteralWalker(const LiteralWalker &other)
    : m_reader(other.m_reader),
      m_literalOffset(other.m_literalOffset)
{
}

bool
lyric_object::LiteralWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_literalOffset < m_reader->numLiterals();
}

lyric_object::ValueType
lyric_object::LiteralWalker::getValueType() const
{
    if (!isValid())
        return ValueType::Invalid;
    auto *literalDescriptor = m_reader->getLiteral(m_literalOffset);
    if (literalDescriptor == nullptr)
        return ValueType::Invalid;

    switch (literalDescriptor->literal_value_type()) {
        case lyo1::Value::TrueFalseNilValue:
            if (literalDescriptor->literal_value_as_TrueFalseNilValue()->tfn() == lyo1::TrueFalseNil::Nil)
                return ValueType::Nil;
            return ValueType::Bool;
        case lyo1::Value::CharValue:
            return ValueType::Char;
        case lyo1::Value::Int64Value:
            return ValueType::Int64;
        case lyo1::Value::Float64Value:
            return ValueType::Float64;
        case lyo1::Value::StringValue:
            return ValueType::String;
        default:
            return ValueType::Invalid;
    }
}

bool
lyric_object::LiteralWalker::boolValue() const
{
    if (!isValid())
        return false;
    auto *literalDescriptor = m_reader->getLiteral(m_literalOffset);
    if (literalDescriptor == nullptr)
        return false;
    if (literalDescriptor->literal_value_type() != lyo1::Value::TrueFalseNilValue)
        return false;
    return literalDescriptor->literal_value_as_TrueFalseNilValue()->tfn() == lyo1::TrueFalseNil::True;
}

UChar32
lyric_object::LiteralWalker::charValue() const
{
    if (!isValid())
        return 0;
    auto *literalDescriptor = m_reader->getLiteral(m_literalOffset);
    if (literalDescriptor == nullptr)
        return 0;
    if (literalDescriptor->literal_value_type() != lyo1::Value::CharValue)
        return 0;
    return literalDescriptor->literal_value_as_CharValue()->chr();
}

tu_int64
lyric_object::LiteralWalker::int64Value() const
{
    if (!isValid())
        return 0;
    auto *literalDescriptor = m_reader->getLiteral(m_literalOffset);
    if (literalDescriptor == nullptr)
        return 0;
    if (literalDescriptor->literal_value_type() != lyo1::Value::Int64Value)
        return 0;
    return literalDescriptor->literal_value_as_Int64Value()->i64();
}

double
lyric_object::LiteralWalker::float64Value() const
{
    if (!isValid())
        return 0.0;
    auto *literalDescriptor = m_reader->getLiteral(m_literalOffset);
    if (literalDescriptor == nullptr)
        return 0.0;
    if (literalDescriptor->literal_value_type() != lyo1::Value::Float64Value)
        return 0.0;
    return literalDescriptor->literal_value_as_Float64Value()->dbl();
}

std::string_view
lyric_object::LiteralWalker::stringValue() const
{
    if (!isValid())
        return {};
    auto *literalDescriptor = m_reader->getLiteral(m_literalOffset);
    if (literalDescriptor == nullptr)
        return {};
    if (literalDescriptor->literal_value_type() != lyo1::Value::StringValue)
        return {};
    return literalDescriptor->literal_value_as_StringValue()->str()->string_view();
}

tu_uint32 lyric_object::LiteralWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_literalOffset;
}
