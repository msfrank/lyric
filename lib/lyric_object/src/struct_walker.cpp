
#include <lyric_object/call_walker.h>
#include <lyric_object/field_walker.h>
#include <lyric_object/impl_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/struct_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::StructMember::StructMember()
    : m_structDescriptor(nullptr)
{
}

lyric_object::StructMember::StructMember(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *structDescriptor,
    tu_uint8 fieldOffset)
    : m_reader(reader),
      m_structDescriptor(structDescriptor),
      m_fieldOffset(fieldOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_structDescriptor != nullptr);
}

lyric_object::StructMember::StructMember(const StructMember &other)
    : m_reader(other.m_reader),
      m_structDescriptor(other.m_structDescriptor),
      m_fieldOffset(other.m_fieldOffset)
{
}

bool
lyric_object::StructMember::isValid() const
{
    return m_reader && m_reader->isValid() && m_structDescriptor;
}

lyric_object::AddressType
lyric_object::StructMember::memberAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *structDescriptor = static_cast<const lyo1::StructDescriptor *>(m_structDescriptor);
    if (structDescriptor->members() == nullptr)
        return AddressType::Invalid;
    if (structDescriptor->members()->size() <= m_fieldOffset)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(structDescriptor->members()->Get(m_fieldOffset));
}

lyric_object::FieldWalker
lyric_object::StructMember::getNearField() const
{
    if (!isValid())
        return {};
    auto *structDescriptor = static_cast<const lyo1::StructDescriptor *>(m_structDescriptor);
    if (structDescriptor->members() == nullptr)
        return {};
    if (structDescriptor->members()->size() <= m_fieldOffset)
        return {};
    return FieldWalker(m_reader, GET_DESCRIPTOR_OFFSET(structDescriptor->members()->Get(m_fieldOffset)));
}

lyric_object::LinkWalker
lyric_object::StructMember::getFarField() const
{
    if (!isValid())
        return {};
    auto *structDescriptor = static_cast<const lyo1::StructDescriptor *>(m_structDescriptor);
    if (structDescriptor->members() == nullptr)
        return {};
    if (structDescriptor->members()->size() <= m_fieldOffset)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(structDescriptor->members()->Get(m_fieldOffset)));
}

lyric_object::StructMethod::StructMethod()
    : m_structDescriptor(nullptr)
{
}

lyric_object::StructMethod::StructMethod(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *structDescriptor,
    tu_uint8 callOffset)
    : m_reader(reader),
      m_structDescriptor(structDescriptor),
      m_callOffset(callOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_structDescriptor != nullptr);
}

lyric_object::StructMethod::StructMethod(const StructMethod &other)
    : m_reader(other.m_reader),
      m_structDescriptor(other.m_structDescriptor),
      m_callOffset(other.m_callOffset)
{
}

bool
lyric_object::StructMethod::isValid() const
{
    return m_reader && m_reader->isValid() && m_structDescriptor;
}

lyric_object::AddressType
lyric_object::StructMethod::methodAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *structDescriptor = static_cast<const lyo1::StructDescriptor *>(m_structDescriptor);
    if (structDescriptor->methods() == nullptr)
        return AddressType::Invalid;
    if (structDescriptor->methods()->size() <= m_callOffset)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(structDescriptor->methods()->Get(m_callOffset));
}

lyric_object::CallWalker
lyric_object::StructMethod::getNearCall() const
{
    if (!isValid())
        return {};
    auto *structDescriptor = static_cast<const lyo1::StructDescriptor *>(m_structDescriptor);
    if (structDescriptor->methods() == nullptr)
        return {};
    if (structDescriptor->methods()->size() <= m_callOffset)
        return {};
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(structDescriptor->methods()->Get(m_callOffset)));
}

lyric_object::LinkWalker
lyric_object::StructMethod::getFarCall() const
{
    if (!isValid())
        return {};
    auto *structDescriptor = static_cast<const lyo1::StructDescriptor *>(m_structDescriptor);
    if (structDescriptor->methods() == nullptr)
        return {};
    if (structDescriptor->methods()->size() <= m_callOffset)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(structDescriptor->methods()->Get(m_callOffset)));
}

lyric_object::StructWalker::StructWalker()
    : m_structOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::StructWalker::StructWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 structOffset)
    : m_reader(reader),
      m_structOffset(structOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::StructWalker::StructWalker(const StructWalker &other)
    : m_reader(other.m_reader),
      m_structOffset(other.m_structOffset)
{
}

bool
lyric_object::StructWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_structOffset < m_reader->numStructs();
}

lyric_common::SymbolPath
lyric_object::StructWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return {};
    if (structDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(structDescriptor->fqsn()->str());
}

bool
lyric_object::StructWalker::isAbstract() const
{
    if (!isValid())
        return false;
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return false;
    return bool(structDescriptor->flags() & lyo1::StructFlags::Abstract);
}

bool
lyric_object::StructWalker::isDeclOnly() const
{
    if (!isValid())
        return false;
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return false;
    return bool(structDescriptor->flags() & lyo1::StructFlags::DeclOnly);
}

lyric_object::DeriveType
lyric_object::StructWalker::getDeriveType() const
{
    if (!isValid())
        return DeriveType::Any;
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return DeriveType::Any;
    if (bool(structDescriptor->flags() & lyo1::StructFlags::Final))
        return DeriveType::Final;
    if (bool(structDescriptor->flags() & lyo1::StructFlags::Sealed))
        return DeriveType::Sealed;
    return DeriveType::Any;
}

lyric_object::AccessType
lyric_object::StructWalker::getAccess() const
{
    if (!isValid())
        return AccessType::Invalid;
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return AccessType::Invalid;

    if (bool(structDescriptor->flags() & lyo1::StructFlags::Hidden))
        return AccessType::Hidden;
    return AccessType::Public;
}

bool
lyric_object::StructWalker::hasAllocator() const
{
    if (!isValid())
        return {};
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return {};
    return structDescriptor->allocator_trap() != INVALID_ADDRESS_U32;
}

tu_uint32
lyric_object::StructWalker::getAllocator() const
{
    if (!isValid())
        return {};
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return {};
    return structDescriptor->allocator_trap();
}

lyric_object::CallWalker
lyric_object::StructWalker::getConstructor() const
{
    if (!isValid())
        return {};
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return {};
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(structDescriptor->ctor_call()));
}

bool
lyric_object::StructWalker::hasSuperStruct() const
{
    if (!isValid())
        return false;
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return false;
    return structDescriptor->super_struct() != INVALID_ADDRESS_U32;
}

lyric_object::AddressType
lyric_object::StructWalker::superStructAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(structDescriptor->super_struct());
}

lyric_object::StructWalker
lyric_object::StructWalker::getNearSuperStruct() const
{
    if (!hasSuperStruct())
        return {};
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return {};
    return StructWalker(m_reader, GET_DESCRIPTOR_OFFSET(structDescriptor->super_struct()));
}

lyric_object::LinkWalker
lyric_object::StructWalker::getFarSuperStruct() const
{
    if (!hasSuperStruct())
        return {};
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(structDescriptor->super_struct()));
}

tu_uint8
lyric_object::StructWalker::numMembers() const
{
    if (!isValid())
        return 0;
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return 0;
    if (structDescriptor->members() == nullptr)
        return 0;
    return structDescriptor->members()->size();
}

lyric_object::StructMember
lyric_object::StructWalker::getMember(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return {};
    if (structDescriptor->members() == nullptr)
        return {};
    if (structDescriptor->members()->size() <= index)
        return {};
    return StructMember(m_reader, (void *) structDescriptor, index);
}

tu_uint8
lyric_object::StructWalker::numMethods() const
{
    if (!isValid())
        return 0;
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return 0;
    if (structDescriptor->methods() == nullptr)
        return 0;
    return structDescriptor->methods()->size();
}

lyric_object::StructMethod
lyric_object::StructWalker::getMethod(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return {};
    if (structDescriptor->methods() == nullptr)
        return {};
    if (structDescriptor->methods()->size() <= index)
        return {};
    return StructMethod(m_reader, (void *) structDescriptor, index);
}

tu_uint8
lyric_object::StructWalker::numImpls() const
{
    if (!isValid())
        return 0;
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return 0;
    if (structDescriptor->impls() == nullptr)
        return 0;
    return structDescriptor->impls()->size();
}

lyric_object::ImplWalker
lyric_object::StructWalker::getImpl(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return {};
    if (structDescriptor->impls() == nullptr)
        return {};
    if (structDescriptor->impls()->size() <= index)
        return {};
    return ImplWalker(m_reader, structDescriptor->impls()->Get(index));
}

tu_uint8
lyric_object::StructWalker::numSealedSubStructs() const
{
    if (!isValid())
        return 0;
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return 0;
    if (structDescriptor->sealed_subtypes() == nullptr)
        return 0;
    return structDescriptor->sealed_subtypes()->size();
}

lyric_object::TypeWalker
lyric_object::StructWalker::getSealedSubStruct(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return {};
    if (structDescriptor->sealed_subtypes() == nullptr)
        return {};
    if (structDescriptor->sealed_subtypes()->size() <= index)
        return {};
    return TypeWalker(m_reader, structDescriptor->sealed_subtypes()->Get(index));
}

lyric_object::TypeWalker
lyric_object::StructWalker::getStructType() const
{
    if (!isValid())
        return {};
    auto *structDescriptor = m_reader->getStruct(m_structOffset);
    if (structDescriptor == nullptr)
        return {};
    return TypeWalker(m_reader, structDescriptor->struct_type());
}

tu_uint32
lyric_object::StructWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_structOffset;
}
