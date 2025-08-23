
#include <lyric_object/call_walker.h>
#include <lyric_object/existential_walker.h>
#include <lyric_object/impl_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/template_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::ExistentialMethod::ExistentialMethod()
    : m_existentialDescriptor(nullptr)
{
}

lyric_object::ExistentialMethod::ExistentialMethod(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *existentialDescriptor,
    tu_uint8 callOffset)
    : m_reader(reader),
      m_existentialDescriptor(existentialDescriptor),
      m_callOffset(callOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_existentialDescriptor != nullptr);
}

lyric_object::ExistentialMethod::ExistentialMethod(const ExistentialMethod &other)
    : m_reader(other.m_reader),
      m_existentialDescriptor(other.m_existentialDescriptor),
      m_callOffset(other.m_callOffset)
{
}

bool
lyric_object::ExistentialMethod::isValid() const
{
    return m_reader && m_reader->isValid() && m_existentialDescriptor;
}

lyric_object::AddressType
lyric_object::ExistentialMethod::methodAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *existentialDescriptor = static_cast<const lyo1::ExistentialDescriptor *>(m_existentialDescriptor);
    if (existentialDescriptor->methods() == nullptr)
        return AddressType::Invalid;
    if (existentialDescriptor->methods()->size() <= m_callOffset)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(existentialDescriptor->methods()->Get(m_callOffset));
}

lyric_object::CallWalker
lyric_object::ExistentialMethod::getNearCall() const
{
    if (!isValid())
        return {};
    auto *existentialDescriptor = static_cast<const lyo1::ExistentialDescriptor *>(m_existentialDescriptor);
    if (existentialDescriptor->methods() == nullptr)
        return {};
    if (existentialDescriptor->methods()->size() <= m_callOffset)
        return {};
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(existentialDescriptor->methods()->Get(m_callOffset)));
}

lyric_object::LinkWalker
lyric_object::ExistentialMethod::getFarCall() const
{
    if (!isValid())
        return {};
    auto *existentialDescriptor = static_cast<const lyo1::ExistentialDescriptor *>(m_existentialDescriptor);
    if (existentialDescriptor->methods() == nullptr)
        return {};
    if (existentialDescriptor->methods()->size() <= m_callOffset)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(existentialDescriptor->methods()->Get(m_callOffset)));
}

lyric_object::ExistentialWalker::ExistentialWalker()
    : m_existentialOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::ExistentialWalker::ExistentialWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    tu_uint32 existentialOffset)
    : m_reader(reader),
      m_existentialOffset(existentialOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::ExistentialWalker::ExistentialWalker(const ExistentialWalker &other)
    : m_reader(other.m_reader),
      m_existentialOffset(other.m_existentialOffset)
{
}

bool
lyric_object::ExistentialWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_existentialOffset < m_reader->numExistentials();
}

lyric_common::SymbolPath
lyric_object::ExistentialWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return {};
    if (existentialDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(existentialDescriptor->fqsn()->str());
}

bool
lyric_object::ExistentialWalker::isDeclOnly() const
{
    if (!isValid())
        return false;
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return false;
    return bool(existentialDescriptor->flags() & lyo1::ExistentialFlags::DeclOnly);
}

lyric_object::DeriveType
lyric_object::ExistentialWalker::getDeriveType() const
{
    if (!isValid())
        return DeriveType::Any;
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return DeriveType::Any;
    if (bool(existentialDescriptor->flags() & lyo1::ExistentialFlags::Final))
        return DeriveType::Final;
    if (bool(existentialDescriptor->flags() & lyo1::ExistentialFlags::Sealed))
        return DeriveType::Sealed;
    return DeriveType::Any;
}

lyric_object::AccessType
lyric_object::ExistentialWalker::getAccess() const
{
    if (!isValid())
        return AccessType::Invalid;
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return AccessType::Invalid;

    if (bool(existentialDescriptor->flags() & lyo1::ExistentialFlags::Hidden))
        return AccessType::Hidden;
    return AccessType::Public;
}

bool
lyric_object::ExistentialWalker::hasSuperExistential() const
{
    if (!isValid())
        return false;
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return false;
    return existentialDescriptor->super_existential() != INVALID_ADDRESS_U32;
}

lyric_object::AddressType
lyric_object::ExistentialWalker::superExistentialAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(existentialDescriptor->super_existential());
}

lyric_object::ExistentialWalker
lyric_object::ExistentialWalker::getNearSuperExistential() const
{
    if (!hasSuperExistential())
        return {};
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return {};
    return ExistentialWalker(m_reader, GET_DESCRIPTOR_OFFSET(existentialDescriptor->super_existential()));
}

lyric_object::LinkWalker
lyric_object::ExistentialWalker::getFarSuperExistential() const
{
    if (!hasSuperExistential())
        return {};
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(existentialDescriptor->super_existential()));
}

bool
lyric_object::ExistentialWalker::hasTemplate() const
{
    if (!isValid())
        return false;
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return false;
    return existentialDescriptor->existential_template() != INVALID_ADDRESS_U32;
}

lyric_object::TemplateWalker
lyric_object::ExistentialWalker::getTemplate() const
{
    if (!hasTemplate())
        return {};
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return {};
    return TemplateWalker(m_reader, existentialDescriptor->existential_template());
}

lyric_object::IntrinsicType
lyric_object::ExistentialWalker::getIntrinsicType() const
{
    if (!isValid())
        return IntrinsicType::Invalid;
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return IntrinsicType::Invalid;

    switch (existentialDescriptor->intrinsic_mapping()) {
        case lyo1::IntrinsicType::Nil:          return IntrinsicType::Nil;
        case lyo1::IntrinsicType::Undef:        return IntrinsicType::Undef;
        case lyo1::IntrinsicType::Bool:         return IntrinsicType::Bool;
        case lyo1::IntrinsicType::Char:         return IntrinsicType::Char;
        case lyo1::IntrinsicType::Float64:      return IntrinsicType::Float;
        case lyo1::IntrinsicType::Int64:        return IntrinsicType::Int;
        case lyo1::IntrinsicType::String:       return IntrinsicType::String;
        case lyo1::IntrinsicType::Url:          return IntrinsicType::Url;
        case lyo1::IntrinsicType::Class:        return IntrinsicType::Class;
        case lyo1::IntrinsicType::Concept:      return IntrinsicType::Concept;
        case lyo1::IntrinsicType::Instance:     return IntrinsicType::Instance;
        case lyo1::IntrinsicType::Call:         return IntrinsicType::Call;
        case lyo1::IntrinsicType::Action:       return IntrinsicType::Action;
        case lyo1::IntrinsicType::Field:        return IntrinsicType::Field;
        case lyo1::IntrinsicType::Struct:       return IntrinsicType::Struct;
        case lyo1::IntrinsicType::Enum:         return IntrinsicType::Enum;
        case lyo1::IntrinsicType::Existential:  return IntrinsicType::Existential;
        case lyo1::IntrinsicType::Namespace:    return IntrinsicType::Namespace;
        default:
            return IntrinsicType::Invalid;
    }
}

tu_uint8
lyric_object::ExistentialWalker::numMethods() const
{
    if (!isValid())
        return 0;
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return 0;
    if (existentialDescriptor->methods() == nullptr)
        return 0;
    return existentialDescriptor->methods()->size();
}

lyric_object::ExistentialMethod
lyric_object::ExistentialWalker::getMethod(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return {};
    if (existentialDescriptor->methods() == nullptr)
        return {};
    if (existentialDescriptor->methods()->size() <= index)
        return {};
    return ExistentialMethod(m_reader, (void *) existentialDescriptor, index);
}

tu_uint8
lyric_object::ExistentialWalker::numImpls() const
{
    if (!isValid())
        return 0;
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return 0;
    if (existentialDescriptor->impls() == nullptr)
        return 0;
    return existentialDescriptor->impls()->size();
}

lyric_object::ImplWalker
lyric_object::ExistentialWalker::getImpl(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return {};
    if (existentialDescriptor->impls() == nullptr)
        return {};
    if (existentialDescriptor->impls()->size() <= index)
        return {};
    return ImplWalker(m_reader, existentialDescriptor->impls()->Get(index));
}

tu_uint8
lyric_object::ExistentialWalker::numSealedSubExistentials() const
{
    if (!isValid())
        return 0;
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return 0;
    if (existentialDescriptor->sealed_subtypes() == nullptr)
        return 0;
    return existentialDescriptor->sealed_subtypes()->size();
}

lyric_object::TypeWalker
lyric_object::ExistentialWalker::getSealedSubExistential(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return {};
    if (existentialDescriptor->sealed_subtypes() == nullptr)
        return {};
    if (existentialDescriptor->sealed_subtypes()->size() <= index)
        return {};
    return TypeWalker(m_reader, existentialDescriptor->sealed_subtypes()->Get(index));
}

lyric_object::TypeWalker
lyric_object::ExistentialWalker::getExistentialType() const
{
    if (!isValid())
        return {};
    auto *existentialDescriptor = m_reader->getExistential(m_existentialOffset);
    if (existentialDescriptor == nullptr)
        return {};
    return TypeWalker(m_reader, existentialDescriptor->existential_type());
}

tu_uint32
lyric_object::ExistentialWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_existentialOffset;
}
