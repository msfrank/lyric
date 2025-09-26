
#include <lyric_object/call_walker.h>
#include <lyric_object/extension_walker.h>
#include <lyric_object/field_walker.h>
#include <lyric_object/impl_walker.h>
#include <lyric_object/instance_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/type_walker.h>

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

lyric_object::AccessType
lyric_object::InstanceWalker::getAccess() const
{
    if (!isValid())
        return AccessType::Invalid;
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return AccessType::Invalid;

    if (bool(instanceDescriptor->flags() & lyo1::InstanceFlags::Hidden))
        return AccessType::Hidden;
    return AccessType::Public;
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
lyric_object::InstanceWalker::getInitializer() const
{
    if (!isValid())
        return {};
    auto *instanceDescriptor = m_reader->getInstance(m_instanceOffset);
    if (instanceDescriptor == nullptr)
        return {};
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(instanceDescriptor->initializer_call()));
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

lyric_object::FieldWalker
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
    return FieldWalker(m_reader, GET_DESCRIPTOR_OFFSET(instanceDescriptor->members()->Get(index)));
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

lyric_object::CallWalker
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
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(instanceDescriptor->methods()->Get(index)));
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
