
#include <lyric_object/call_walker.h>
#include <lyric_object/extension_walker.h>
#include <lyric_object/field_walker.h>
#include <lyric_object/impl_walker.h>
#include <lyric_object/instance_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::InstanceMember::InstanceMember()
    : m_instanceDescriptor(nullptr)
{
}

lyric_object::InstanceMember::InstanceMember(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *instanceDescriptor,
    tu_uint8 fieldOffset)
    : m_reader(reader),
      m_instanceDescriptor(instanceDescriptor),
      m_fieldOffset(fieldOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_instanceDescriptor != nullptr);
}

lyric_object::InstanceMember::InstanceMember(const InstanceMember &other)
    : m_reader(other.m_reader),
      m_instanceDescriptor(other.m_instanceDescriptor),
      m_fieldOffset(other.m_fieldOffset)
{
}

bool
lyric_object::InstanceMember::isValid() const
{
    return m_reader && m_reader->isValid() && m_instanceDescriptor;
}

//std::string
//lyric_object::InstanceMember::getName() const
//{
//    if (!isValid())
//        return {};
//    auto *instanceDescriptor = static_cast<const lyo1::InstanceDescriptor *>(m_instanceDescriptor);
//    if (instanceDescriptor->members() == nullptr)
//        return {};
//    if (instanceDescriptor->members()->size() <= m_fieldOffset)
//        return {};
//    auto *instanceMember = instanceDescriptor->members()->Get(m_fieldOffset);
//    if (instanceDescriptor->names() == nullptr)
//        return {};
//    if (instanceDescriptor->names()->size() <= instanceMember->name_offset())
//        return {};
//    return instanceDescriptor->names()->Get(instanceMember->name_offset())->str();
//}

lyric_object::AddressType
lyric_object::InstanceMember::memberAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *instanceDescriptor = static_cast<const lyo1::InstanceDescriptor *>(m_instanceDescriptor);
    if (instanceDescriptor->members() == nullptr)
        return AddressType::Invalid;
    if (instanceDescriptor->members()->size() <= m_fieldOffset)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(instanceDescriptor->members()->Get(m_fieldOffset));
}

lyric_object::FieldWalker
lyric_object::InstanceMember::getNearField() const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = static_cast<const lyo1::InstanceDescriptor *>(m_instanceDescriptor);
    if (instanceDescriptor->members() == nullptr)
        return {};
    if (instanceDescriptor->members()->size() <= m_fieldOffset)
        return {};
    return FieldWalker(m_reader, GET_DESCRIPTOR_OFFSET(instanceDescriptor->members()->Get(m_fieldOffset)));
}

lyric_object::LinkWalker
lyric_object::InstanceMember::getFarField() const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = static_cast<const lyo1::InstanceDescriptor *>(m_instanceDescriptor);
    if (instanceDescriptor->members() == nullptr)
        return {};
    if (instanceDescriptor->members()->size() <= m_fieldOffset)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(instanceDescriptor->members()->Get(m_fieldOffset)));
}

lyric_object::InstanceMethod::InstanceMethod()
    : m_instanceDescriptor(nullptr)
{
}

lyric_object::InstanceMethod::InstanceMethod(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *instanceDescriptor,
    tu_uint8 callOffset)
    : m_reader(reader),
      m_instanceDescriptor(instanceDescriptor),
      m_callOffset(callOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_instanceDescriptor != nullptr);
}

lyric_object::InstanceMethod::InstanceMethod(const InstanceMethod &other)
    : m_reader(other.m_reader),
      m_instanceDescriptor(other.m_instanceDescriptor),
      m_callOffset(other.m_callOffset)
{
}

bool
lyric_object::InstanceMethod::isValid() const
{
    return m_reader && m_reader->isValid() && m_instanceDescriptor;
}

//std::string
//lyric_object::InstanceMethod::getName() const
//{
//    if (!isValid())
//        return {};
//    auto *instanceDescriptor = static_cast<const lyo1::InstanceDescriptor *>(m_instanceDescriptor);
//    if (instanceDescriptor->methods() == nullptr)
//        return {};
//    if (instanceDescriptor->methods()->size() <= m_callOffset)
//        return {};
//    auto *instanceMethod = instanceDescriptor->methods()->Get(m_callOffset);
//    if (instanceDescriptor->names() == nullptr)
//        return {};
//    if (instanceDescriptor->names()->size() <= instanceMethod->name_offset())
//        return {};
//    return instanceDescriptor->names()->Get(instanceMethod->name_offset())->str();
//}

lyric_object::AddressType
lyric_object::InstanceMethod::methodAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *instanceDescriptor = static_cast<const lyo1::InstanceDescriptor *>(m_instanceDescriptor);
    if (instanceDescriptor->methods() == nullptr)
        return AddressType::Invalid;
    if (instanceDescriptor->methods()->size() <= m_callOffset)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(instanceDescriptor->methods()->Get(m_callOffset));
}

lyric_object::CallWalker
lyric_object::InstanceMethod::getNearCall() const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = static_cast<const lyo1::InstanceDescriptor *>(m_instanceDescriptor);
    if (instanceDescriptor->methods() == nullptr)
        return {};
    if (instanceDescriptor->methods()->size() <= m_callOffset)
        return {};
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(instanceDescriptor->methods()->Get(m_callOffset)));
}

lyric_object::LinkWalker
lyric_object::InstanceMethod::getFarCall() const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = static_cast<const lyo1::InstanceDescriptor *>(m_instanceDescriptor);
    if (instanceDescriptor->methods() == nullptr)
        return {};
    if (instanceDescriptor->methods()->size() <= m_callOffset)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(instanceDescriptor->methods()->Get(m_callOffset)));
}

//lyric_object::InstanceImpl::InstanceImpl()
//    : m_instanceDescriptor(nullptr)
//{
//}
//
//lyric_object::InstanceImpl::InstanceImpl(
//    std::shared_ptr<const internal::ObjectReader> reader,
//    void *instanceDescriptor,
//    tu_uint8 implOffset)
//    : m_reader(reader),
//      m_instanceDescriptor(instanceDescriptor),
//      m_implOffset(implOffset)
//{
//    TU_ASSERT (m_reader != nullptr);
//    TU_ASSERT (m_instanceDescriptor != nullptr);
//}
//
//lyric_object::InstanceImpl::InstanceImpl(const InstanceImpl &other)
//    : m_reader(other.m_reader),
//      m_instanceDescriptor(other.m_instanceDescriptor),
//      m_implOffset(other.m_implOffset)
//{
//}
//
//bool
//lyric_object::InstanceImpl::isValid() const
//{
//    return m_reader && m_reader->isValid() && m_instanceDescriptor;
//}
//
//lyric_object::TypeWalker
//lyric_object::InstanceImpl::getImplType() const
//{
//    if (!isValid())
//        return {};
//    auto *instanceDescriptor = static_cast<const lyo1::InstanceDescriptor *>(m_instanceDescriptor);
//    if (instanceDescriptor->impls() == nullptr)
//        return {};
//    if (instanceDescriptor->impls()->size() <= m_implOffset)
//        return {};
//    auto *instanceImpl = instanceDescriptor->impls()->Get(m_implOffset);
//    return TypeWalker(m_reader, instanceImpl->impl_type());
//}
//
//lyric_object::ExtensionWalker
//lyric_object::InstanceImpl::getExtension(tu_uint8 index) const
//{
//    if (!isValid())
//        return {};
//    auto *instanceDescriptor = static_cast<const lyo1::InstanceDescriptor *>(m_instanceDescriptor);
//    if (instanceDescriptor->impls() == nullptr)
//        return {};
//    if (instanceDescriptor->impls()->size() <= m_implOffset)
//        return {};
//    auto *instanceImpl = instanceDescriptor->impls()->Get(m_implOffset);
//    if (instanceImpl->extensions() == nullptr)
//        return {};
//    if (instanceImpl->extensions()->size() <= index)
//        return {};
//    return ExtensionWalker(m_reader, (void *) instanceDescriptor, m_implOffset, index);
//}
//
//tu_uint8
//lyric_object::InstanceImpl::numExtensions() const
//{
//    if (!isValid())
//        return {};
//    auto *instanceDescriptor = static_cast<const lyo1::InstanceDescriptor *>(m_instanceDescriptor);
//    if (instanceDescriptor->impls() == nullptr)
//        return {};
//    if (instanceDescriptor->impls()->size() <= m_implOffset)
//        return {};
//    auto *instanceImpl = instanceDescriptor->impls()->Get(m_implOffset);
//    if (instanceImpl->extensions() == nullptr)
//        return {};
//    return instanceImpl->extensions()->size();
//}

lyric_object::InstanceWalker::InstanceWalker()
    : m_instanceOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::InstanceWalker::InstanceWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    tu_uint32 instanceOffset)
    : m_reader(reader),
      m_instanceOffset(instanceOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::InstanceWalker::InstanceWalker(const InstanceWalker &other)
    : m_reader(other.m_reader),
      m_instanceOffset(other.m_instanceOffset)
{
}

bool
lyric_object::InstanceWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_instanceOffset < m_reader->numInstances();
}

lyric_common::SymbolPath
lyric_object::InstanceWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return {};
    if (instanceDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(instanceDescriptor->fqsn()->str());
}

bool
lyric_object::InstanceWalker::isAbstract() const
{
    if (!isValid())
        return false;
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return false;
    return bool(instanceDescriptor->flags() & lyo1::InstanceFlags::Abstract);
}

bool
lyric_object::InstanceWalker::isDeclOnly() const
{
    if (!isValid())
        return false;
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return false;
    return bool(instanceDescriptor->flags() & lyo1::InstanceFlags::DeclOnly);
}

lyric_object::DeriveType
lyric_object::InstanceWalker::getDeriveType() const
{
    if (!isValid())
        return DeriveType::Any;
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return DeriveType::Any;
    if (bool(instanceDescriptor->flags() & lyo1::InstanceFlags::Final))
        return DeriveType::Final;
    if (bool(instanceDescriptor->flags() & lyo1::InstanceFlags::Sealed))
        return DeriveType::Sealed;
    return DeriveType::Any;
}

bool
lyric_object::InstanceWalker::hasAllocator() const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return {};
    return instanceDescriptor->allocator_trap() != INVALID_ADDRESS_U32;
}

tu_uint32
lyric_object::InstanceWalker::getAllocator() const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return {};
    return instanceDescriptor->allocator_trap();
}

lyric_object::CallWalker
lyric_object::InstanceWalker::getConstructor() const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return {};
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(instanceDescriptor->ctor_call()));
}

bool
lyric_object::InstanceWalker::hasSuperInstance() const
{
    if (!isValid())
        return false;
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return false;
    return instanceDescriptor->super_instance() != INVALID_ADDRESS_U32;
}

lyric_object::AddressType
lyric_object::InstanceWalker::superInstanceAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(instanceDescriptor->super_instance());
}

lyric_object::InstanceWalker
lyric_object::InstanceWalker::getNearSuperInstance() const
{
    if (!hasSuperInstance())
        return {};
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return {};
    return InstanceWalker(m_reader, GET_DESCRIPTOR_OFFSET(instanceDescriptor->super_instance()));
}

lyric_object::LinkWalker
lyric_object::InstanceWalker::getFarSuperInstance() const
{
    if (!hasSuperInstance())
        return {};
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(instanceDescriptor->super_instance()));
}

tu_uint8
lyric_object::InstanceWalker::numMembers() const
{
    if (!isValid())
        return 0;
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return 0;
    if (instanceDescriptor->members() == nullptr)
        return 0;
    return instanceDescriptor->members()->size();
}

lyric_object::InstanceMember
lyric_object::InstanceWalker::getMember(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return {};
    if (instanceDescriptor->members() == nullptr)
        return {};
    if (instanceDescriptor->members()->size() <= index)
        return {};
    return InstanceMember(m_reader, (void *) instanceDescriptor, index);
}

tu_uint8
lyric_object::InstanceWalker::numMethods() const
{
    if (!isValid())
        return 0;
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return 0;
    if (instanceDescriptor->methods() == nullptr)
        return 0;
    return instanceDescriptor->methods()->size();
}

lyric_object::InstanceMethod
lyric_object::InstanceWalker::getMethod(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return {};
    if (instanceDescriptor->methods() == nullptr)
        return {};
    if (instanceDescriptor->methods()->size() <= index)
        return {};
    return InstanceMethod(m_reader, (void *) instanceDescriptor, index);
}

tu_uint8
lyric_object::InstanceWalker::numImpls() const
{
    if (!isValid())
        return 0;
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return 0;
    if (instanceDescriptor->impls() == nullptr)
        return 0;
    return instanceDescriptor->impls()->size();
}

lyric_object::ImplWalker
lyric_object::InstanceWalker::getImpl(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return {};
    if (instanceDescriptor->impls() == nullptr)
        return {};
    if (instanceDescriptor->impls()->size() <= index)
        return {};
    return ImplWalker(m_reader, instanceDescriptor->impls()->Get(index));
}

tu_uint8
lyric_object::InstanceWalker::numSealedSubInstances() const
{
    if (!isValid())
        return 0;
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return 0;
    if (instanceDescriptor->sealed_subtypes() == nullptr)
        return 0;
    return instanceDescriptor->sealed_subtypes()->size();
}

lyric_object::TypeWalker
lyric_object::InstanceWalker::getSealedSubInstance(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return {};
    if (instanceDescriptor->sealed_subtypes() == nullptr)
        return {};
    if (instanceDescriptor->sealed_subtypes()->size() <= index)
        return {};
    return TypeWalker(m_reader, instanceDescriptor->sealed_subtypes()->Get(index));
}

lyric_object::TypeWalker
lyric_object::InstanceWalker::getInstanceType() const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return {};
    return TypeWalker(m_reader, instanceDescriptor->instance_type());
}

tu_uint32 lyric_object::InstanceWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_instanceOffset;
}
