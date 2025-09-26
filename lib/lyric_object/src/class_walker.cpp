
#include <lyric_object/action_walker.h>
#include <lyric_object/call_walker.h>
#include <lyric_object/class_walker.h>
#include <lyric_object/field_walker.h>
#include <lyric_object/impl_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/template_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::ClassWalker::ClassWalker()
    : m_classOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::ClassWalker::ClassWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 classOffset)
    : m_reader(std::move(reader)),
      m_classOffset(classOffset)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_classOffset != INVALID_ADDRESS_U32);
}

lyric_object::ClassWalker::ClassWalker(const ClassWalker &other)
    : m_reader(other.m_reader),
      m_classOffset(other.m_classOffset)
{
}

bool
lyric_object::ClassWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_classOffset < m_reader->numClasses();
}

lyric_common::SymbolPath
lyric_object::ClassWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return {};
    if (classDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(classDescriptor->fqsn()->str());
}

bool
lyric_object::ClassWalker::isDeclOnly() const
{
    if (!isValid())
        return false;
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return false;
    return bool(classDescriptor->flags() & lyo1::ClassFlags::DeclOnly);
}

lyric_object::DeriveType
lyric_object::ClassWalker::getDeriveType() const
{
    if (!isValid())
        return DeriveType::Any;
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return DeriveType::Any;
    if (bool(classDescriptor->flags() & lyo1::ClassFlags::Final))
        return DeriveType::Final;
    if (bool(classDescriptor->flags() & lyo1::ClassFlags::Sealed))
        return DeriveType::Sealed;
    return DeriveType::Any;
}

lyric_object::AccessType
lyric_object::ClassWalker::getAccess() const
{
    if (!isValid())
        return AccessType::Invalid;
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return AccessType::Invalid;

    if (bool(classDescriptor->flags() & lyo1::ClassFlags::Hidden))
        return AccessType::Hidden;
    return AccessType::Public;
}

bool
lyric_object::ClassWalker::hasAllocator() const
{
    if (!isValid())
        return {};
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return {};
    return classDescriptor->allocator_trap() != INVALID_ADDRESS_U32;
}

tu_uint32
lyric_object::ClassWalker::getAllocator() const
{
    if (!isValid())
        return {};
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return {};
    return classDescriptor->allocator_trap();
}

bool
lyric_object::ClassWalker::hasSuperClass() const
{
    if (!isValid())
        return false;
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return false;
    return IS_VALID(classDescriptor->super_class());
}

lyric_object::AddressType
lyric_object::ClassWalker::superClassAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(classDescriptor->super_class());
}

lyric_object::ClassWalker
lyric_object::ClassWalker::getNearSuperClass() const
{
    if (!hasSuperClass())
        return {};
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return {};
    return ClassWalker(m_reader, GET_DESCRIPTOR_OFFSET(classDescriptor->super_class()));
}

lyric_object::LinkWalker
lyric_object::ClassWalker::getFarSuperClass() const
{
    if (!hasSuperClass())
        return {};
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(classDescriptor->super_class()));
}

bool
lyric_object::ClassWalker::hasTemplate() const
{
    if (!isValid())
        return false;
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return false;
    return classDescriptor->class_template() != INVALID_ADDRESS_U32;
}

lyric_object::TemplateWalker
lyric_object::ClassWalker::getTemplate() const
{
    if (!hasTemplate())
        return {};
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return {};
    return TemplateWalker(m_reader, classDescriptor->class_template());
}

tu_uint8
lyric_object::ClassWalker::numMembers() const
{
    if (!isValid())
        return 0;
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return 0;
    if (classDescriptor->members() == nullptr)
        return 0;
    return classDescriptor->members()->size();
}

lyric_object::FieldWalker
lyric_object::ClassWalker::getMember(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return {};
    if (classDescriptor->members() == nullptr)
        return {};
    if (classDescriptor->members()->size() <= index)
        return {};
    return FieldWalker(m_reader, GET_DESCRIPTOR_OFFSET(classDescriptor->members()->Get(index)));
}

tu_uint8
lyric_object::ClassWalker::numMethods() const
{
    if (!isValid())
        return 0;
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return 0;
    if (classDescriptor->methods() == nullptr)
        return 0;
    return classDescriptor->methods()->size();
}

lyric_object::CallWalker
lyric_object::ClassWalker::getMethod(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return {};
    if (classDescriptor->methods() == nullptr)
        return {};
    if (classDescriptor->methods()->size() <= index)
        return {};
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(classDescriptor->methods()->Get(index)));
}

tu_uint8
lyric_object::ClassWalker::numImpls() const
{
    if (!isValid())
        return 0;
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return 0;
    if (classDescriptor->impls() == nullptr)
        return 0;
    return classDescriptor->impls()->size();
}

lyric_object::ImplWalker
lyric_object::ClassWalker::getImpl(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return {};
    if (classDescriptor->impls() == nullptr)
        return {};
    if (classDescriptor->impls()->size() <= index)
        return {};
    return ImplWalker(m_reader, classDescriptor->impls()->Get(index));
}

tu_uint8
lyric_object::ClassWalker::numSealedSubClasses() const
{
    if (!isValid())
        return 0;
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return 0;
    if (classDescriptor->sealed_subtypes() == nullptr)
        return 0;
    return classDescriptor->sealed_subtypes()->size();
}

lyric_object::TypeWalker
lyric_object::ClassWalker::getSealedSubClass(tu_uint8 index) const
{
    if (!isValid())
        return {};
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return {};
    if (classDescriptor->sealed_subtypes() == nullptr)
        return {};
    if (classDescriptor->sealed_subtypes()->size() <= index)
        return {};
    return TypeWalker(m_reader, classDescriptor->sealed_subtypes()->Get(index));
}

lyric_object::TypeWalker
lyric_object::ClassWalker::getClassType() const
{
    if (!isValid())
        return {};
    auto *classDescriptor = m_reader->getClass(m_classOffset);
    if (classDescriptor == nullptr)
        return {};
    return TypeWalker(m_reader, classDescriptor->class_type());
}

tu_uint32
lyric_object::ClassWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_classOffset;
}
