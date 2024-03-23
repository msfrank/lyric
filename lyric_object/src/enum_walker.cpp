
#include <lyric_object/call_walker.h>
#include <lyric_object/enum_walker.h>
#include <lyric_object/field_walker.h>
#include <lyric_object/impl_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::EnumMember::EnumMember()
    : m_enumDescriptor(nullptr)
{
}

lyric_object::EnumMember::EnumMember(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *enumDescriptor,
    tu_uint8 fieldOffset)
    : m_reader(reader),
      m_enumDescriptor(enumDescriptor),
      m_fieldOffset(fieldOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_enumDescriptor != nullptr);
}

lyric_object::EnumMember::EnumMember(const EnumMember &other)
    : m_reader(other.m_reader),
      m_enumDescriptor(other.m_enumDescriptor),
      m_fieldOffset(other.m_fieldOffset)
{
}

bool
lyric_object::EnumMember::isValid() const
{
    return m_reader && m_reader->isValid() && m_enumDescriptor;
}

lyric_object::AddressType
lyric_object::EnumMember::memberAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *enumDescriptor = static_cast<const lyo1::EnumDescriptor *>(m_enumDescriptor);
    if (enumDescriptor->members() == nullptr)
        return AddressType::Invalid;
    if (enumDescriptor->members()->size() <= m_fieldOffset)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(enumDescriptor->members()->Get(m_fieldOffset));
}

lyric_object::FieldWalker
lyric_object::EnumMember::getNearField() const
{
    if (!isValid())
        return {};
    auto *enumDescriptor = static_cast<const lyo1::EnumDescriptor *>(m_enumDescriptor);
    if (enumDescriptor->members() == nullptr)
        return {};
    if (enumDescriptor->members()->size() <= m_fieldOffset)
        return {};
    return FieldWalker(m_reader, GET_DESCRIPTOR_OFFSET(enumDescriptor->members()->Get(m_fieldOffset)));
}

lyric_object::LinkWalker
lyric_object::EnumMember::getFarField() const
{
    if (!isValid())
        return {};
    auto *enumDescriptor = static_cast<const lyo1::EnumDescriptor *>(m_enumDescriptor);
    if (enumDescriptor->members() == nullptr)
        return {};
    if (enumDescriptor->members()->size() <= m_fieldOffset)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(enumDescriptor->members()->Get(m_fieldOffset)));
}

lyric_object::EnumMethod::EnumMethod()
    : m_enumDescriptor(nullptr)
{
}

lyric_object::EnumMethod::EnumMethod(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *enumDescriptor,
    tu_uint8 callOffset)
    : m_reader(reader),
      m_enumDescriptor(enumDescriptor),
      m_callOffset(callOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_enumDescriptor != nullptr);
}

lyric_object::EnumMethod::EnumMethod(const EnumMethod &other)
    : m_reader(other.m_reader),
      m_enumDescriptor(other.m_enumDescriptor),
      m_callOffset(other.m_callOffset)
{
}

bool
lyric_object::EnumMethod::isValid() const
{
    return m_reader && m_reader->isValid() && m_enumDescriptor;
}

lyric_object::AddressType
lyric_object::EnumMethod::methodAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *enumDescriptor = static_cast<const lyo1::EnumDescriptor *>(m_enumDescriptor);
    if (enumDescriptor->methods() == nullptr)
        return AddressType::Invalid;
    if (enumDescriptor->methods()->size() <= m_callOffset)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(enumDescriptor->methods()->Get(m_callOffset));
}

lyric_object::CallWalker
lyric_object::EnumMethod::getNearCall() const
{
    if (!isValid())
        return {};
    auto *enumDescriptor = static_cast<const lyo1::EnumDescriptor *>(m_enumDescriptor);
    if (enumDescriptor->methods() == nullptr)
        return {};
    if (enumDescriptor->methods()->size() <= m_callOffset)
        return {};
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(enumDescriptor->methods()->Get(m_callOffset)));
}

lyric_object::LinkWalker
lyric_object::EnumMethod::getFarCall() const
{
    if (!isValid())
        return {};
    auto *enumDescriptor = static_cast<const lyo1::EnumDescriptor *>(m_enumDescriptor);
    if (enumDescriptor->methods() == nullptr)
        return {};
    if (enumDescriptor->methods()->size() <= m_callOffset)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(enumDescriptor->methods()->Get(m_callOffset)));
}

lyric_object::EnumWalker::EnumWalker()
    : m_enumOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::EnumWalker::EnumWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 enumOffset)
    : m_reader(reader),
      m_enumOffset(enumOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::EnumWalker::EnumWalker(const EnumWalker &other)
    : m_reader(other.m_reader),
      m_enumOffset(other.m_enumOffset)
{
}

bool
lyric_object::EnumWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_enumOffset < m_reader->numEnums();
}

lyric_common::SymbolPath
lyric_object::EnumWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return {};
    if (enumDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(enumDescriptor->fqsn()->str());
}

bool
lyric_object::EnumWalker::isAbstract() const
{
    if (!isValid())
        return false;
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return false;
    return bool(enumDescriptor->flags() & lyo1::EnumFlags::Abstract);
}

bool
lyric_object::EnumWalker::isDeclOnly() const
{
    if (!isValid())
        return false;
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return false;
    return bool(enumDescriptor->flags() & lyo1::EnumFlags::DeclOnly);
}

lyric_object::DeriveType
lyric_object::EnumWalker::getDeriveType() const
{
    if (!isValid())
        return DeriveType::Any;
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return DeriveType::Any;
    if (bool(enumDescriptor->flags() & lyo1::EnumFlags::Final))
        return DeriveType::Final;
    if (bool(enumDescriptor->flags() & lyo1::EnumFlags::Sealed))
        return DeriveType::Sealed;
    return DeriveType::Any;
}

bool
lyric_object::EnumWalker::hasAllocator() const
{
    if (!isValid())
        return {};
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return {};
    return enumDescriptor->allocator_trap() != INVALID_ADDRESS_U32;
}

tu_uint32
lyric_object::EnumWalker::getAllocator() const
{
    if (!isValid())
        return {};
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return {};
    return enumDescriptor->allocator_trap();
}

lyric_object::CallWalker
lyric_object::EnumWalker::getConstructor() const
{
    if (!isValid())
        return {};
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return {};
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(enumDescriptor->ctor_call()));
}

bool
lyric_object::EnumWalker::hasSuperEnum() const
{
    if (!isValid())
        return false;
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return false;
    return enumDescriptor->super_enum() != INVALID_ADDRESS_U32;
}

lyric_object::AddressType
lyric_object::EnumWalker::superEnumAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(enumDescriptor->super_enum());
}

lyric_object::EnumWalker
lyric_object::EnumWalker::getNearSuperEnum() const
{
    if (!hasSuperEnum())
        return {};
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return {};
    return EnumWalker(m_reader, GET_DESCRIPTOR_OFFSET(enumDescriptor->super_enum()));
}

lyric_object::LinkWalker
lyric_object::EnumWalker::getFarSuperEnum() const
{
    if (!hasSuperEnum())
        return {};
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(enumDescriptor->super_enum()));
}

tu_uint8
lyric_object::EnumWalker::numMembers() const
{
    if (!isValid())
        return 0;
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return 0;
    if (enumDescriptor->members() == nullptr)
        return 0;
    return enumDescriptor->members()->size();
}

lyric_object::EnumMember
lyric_object::EnumWalker::getMember(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return {};
    if (enumDescriptor->members() == nullptr)
        return {};
    if (enumDescriptor->members()->size() <= index)
        return {};
    return EnumMember(m_reader, (void *) enumDescriptor, index);
}

tu_uint8
lyric_object::EnumWalker::numMethods() const
{
    if (!isValid())
        return 0;
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return 0;
    if (enumDescriptor->methods() == nullptr)
        return 0;
    return enumDescriptor->methods()->size();
}

lyric_object::EnumMethod
lyric_object::EnumWalker::getMethod(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return {};
    if (enumDescriptor->methods() == nullptr)
        return {};
    if (enumDescriptor->methods()->size() <= index)
        return {};
    return EnumMethod(m_reader, (void *) enumDescriptor, index);
}

tu_uint8
lyric_object::EnumWalker::numImpls() const
{
    if (!isValid())
        return 0;
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return 0;
    if (enumDescriptor->impls() == nullptr)
        return 0;
    return enumDescriptor->impls()->size();
}

lyric_object::ImplWalker
lyric_object::EnumWalker::getImpl(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return {};
    if (enumDescriptor->impls() == nullptr)
        return {};
    if (enumDescriptor->impls()->size() <= index)
        return {};
    return ImplWalker(m_reader, enumDescriptor->impls()->Get(index));
}

tu_uint8
lyric_object::EnumWalker::numSealedSubEnums() const
{
    if (!isValid())
        return 0;
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return 0;
    if (enumDescriptor->sealed_subtypes() == nullptr)
        return 0;
    return enumDescriptor->sealed_subtypes()->size();
}

lyric_object::TypeWalker
lyric_object::EnumWalker::getSealedSubEnum(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return {};
    if (enumDescriptor->sealed_subtypes() == nullptr)
        return {};
    if (enumDescriptor->sealed_subtypes()->size() <= index)
        return {};
    return TypeWalker(m_reader, enumDescriptor->sealed_subtypes()->Get(index));
}

lyric_object::TypeWalker
lyric_object::EnumWalker::getEnumType() const
{
    if (!isValid())
        return {};
    auto *enumDescriptor = m_reader->getEnum(m_enumOffset);
    if (enumDescriptor == nullptr)
        return {};
    return TypeWalker(m_reader, enumDescriptor->enum_type());
}

tu_uint32
lyric_object::EnumWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_enumOffset;
}
