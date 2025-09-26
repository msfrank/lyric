
#include <lyric_object/call_walker.h>
#include <lyric_object/field_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::FieldWalker::FieldWalker()
    : m_fieldOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::FieldWalker::FieldWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 fieldOffset)
    : m_reader(reader),
      m_fieldOffset(fieldOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::FieldWalker::FieldWalker(const FieldWalker &other)
    : m_reader(other.m_reader),
      m_fieldOffset(other.m_fieldOffset)
{
}

bool
lyric_object::FieldWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_fieldOffset < m_reader->numFields();
}

bool
lyric_object::FieldWalker::isVariable() const
{
    if (!isValid())
        return false;
    auto *fieldDescriptor = m_reader->getField(m_fieldOffset);
    if (fieldDescriptor == nullptr)
        return false;
    return bool(fieldDescriptor->flags() & lyo1::FieldFlags::Var);
}

bool
lyric_object::FieldWalker::isDeclOnly() const
{
    if (!isValid())
        return false;
    auto *fieldDescriptor = m_reader->getField(m_fieldOffset);
    if (fieldDescriptor == nullptr)
        return false;
    return bool(fieldDescriptor->flags() & lyo1::FieldFlags::DeclOnly);
}

lyric_object::AccessType
lyric_object::FieldWalker::getAccess() const
{
    if (!isValid())
        return AccessType::Invalid;
    auto *fieldDescriptor = m_reader->getField(m_fieldOffset);
    if (fieldDescriptor == nullptr)
        return AccessType::Invalid;

    if (bool(fieldDescriptor->flags() & lyo1::FieldFlags::Hidden))
        return AccessType::Hidden;
    return AccessType::Public;
}

lyric_common::SymbolPath
lyric_object::FieldWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *fieldDescriptor = m_reader->getField(m_fieldOffset);
    if (fieldDescriptor == nullptr)
        return {};
    if (fieldDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(fieldDescriptor->fqsn()->str());
}

lyric_object::TypeWalker
lyric_object::FieldWalker::getFieldType() const
{
    if (!isValid())
        return {};
    auto *fieldDescriptor = m_reader->getField(m_fieldOffset);
    if (fieldDescriptor == nullptr)
        return {};
    return TypeWalker(m_reader, fieldDescriptor->field_type());
}

bool
lyric_object::FieldWalker::hasInitializer() const
{
    if (!isValid())
        return false;
    auto *fieldDescriptor = m_reader->getField(m_fieldOffset);
    if (fieldDescriptor == nullptr)
        return false;
    return fieldDescriptor->initializer_call() != INVALID_ADDRESS_U32;
}

lyric_object::AddressType
lyric_object::FieldWalker::initializerAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *fieldDescriptor = m_reader->getField(m_fieldOffset);
    if (fieldDescriptor == nullptr)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(fieldDescriptor->initializer_call());
}

lyric_object::CallWalker
lyric_object::FieldWalker::getNearInitializer() const
{
    if (!isValid())
        return {};
    auto *fieldDescriptor = m_reader->getField(m_fieldOffset);
    if (fieldDescriptor == nullptr)
        return {};
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(fieldDescriptor->initializer_call()));
}

lyric_object::LinkWalker
lyric_object::FieldWalker::getFarInitializer() const
{
    if (!isValid())
        return {};
    auto *fieldDescriptor = m_reader->getField(m_fieldOffset);
    if (fieldDescriptor == nullptr)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(fieldDescriptor->initializer_call()));
}

tu_uint32
lyric_object::FieldWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_fieldOffset;
}
