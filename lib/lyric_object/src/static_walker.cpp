
#include <lyric_object/call_walker.h>
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/link_walker.h>
#include <lyric_object/static_walker.h>
#include <lyric_object/type_walker.h>

lyric_object::StaticWalker::StaticWalker()
    : m_staticOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::StaticWalker::StaticWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 staticOffset)
    : m_reader(reader),
      m_staticOffset(staticOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::StaticWalker::StaticWalker(const StaticWalker &other)
    : m_reader(other.m_reader),
      m_staticOffset(other.m_staticOffset)
{
}

bool
lyric_object::StaticWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_staticOffset < m_reader->numStatics();
}

lyric_common::SymbolPath
lyric_object::StaticWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *staticDescriptor = m_reader->getStatic(m_staticOffset);
    if (staticDescriptor == nullptr)
        return {};
    if (staticDescriptor->fqsn() == nullptr)
        return {};
    return lyric_common::SymbolPath::fromString(staticDescriptor->fqsn()->str());
}

bool
lyric_object::StaticWalker::isVariable() const
{
    if (!isValid())
        return false;
    auto *staticDescriptor = m_reader->getStatic(m_staticOffset);
    if (staticDescriptor == nullptr)
        return false;
    return bool(staticDescriptor->flags() & lyo1::StaticFlags::Var);
}

bool
lyric_object::StaticWalker::isDeclOnly() const
{
    if (!isValid())
        return false;
    auto *staticDescriptor = m_reader->getStatic(m_staticOffset);
    if (staticDescriptor == nullptr)
        return false;
    return bool(staticDescriptor->flags() & lyo1::StaticFlags::DeclOnly);
}

lyric_object::AccessType
lyric_object::StaticWalker::getAccess() const
{
    if (!isValid())
        return AccessType::Invalid;
    auto *staticDescriptor = m_reader->getStatic(m_staticOffset);
    if (staticDescriptor == nullptr)
        return AccessType::Invalid;

    if (bool(staticDescriptor->flags() & lyo1::StaticFlags::Hidden))
        return AccessType::Hidden;
    return AccessType::Public;
}

lyric_object::TypeWalker
lyric_object::StaticWalker::getStaticType() const
{
    if (!isValid())
        return {};
    auto *staticDescriptor = m_reader->getStatic(m_staticOffset);
    if (staticDescriptor == nullptr)
        return {};
    return TypeWalker(m_reader, staticDescriptor->static_type());
}

lyric_object::AddressType
lyric_object::StaticWalker::initializerAddressType() const
{
    if (!isValid())
        return AddressType::Invalid;
    auto *staticDescriptor = m_reader->getStatic(m_staticOffset);
    if (staticDescriptor == nullptr)
        return AddressType::Invalid;
    return GET_ADDRESS_TYPE(staticDescriptor->initializer_call());
}

lyric_object::CallWalker
lyric_object::StaticWalker::getNearInitializer() const
{
    if (!isValid())
        return {};
    auto *staticDescriptor = m_reader->getStatic(m_staticOffset);
    if (staticDescriptor == nullptr)
        return {};
    return CallWalker(m_reader, GET_DESCRIPTOR_OFFSET(staticDescriptor->initializer_call()));
}

lyric_object::LinkWalker
lyric_object::StaticWalker::getFarInitializer() const
{
    if (!isValid())
        return {};
    auto *staticDescriptor = m_reader->getStatic(m_staticOffset);
    if (staticDescriptor == nullptr)
        return {};
    return LinkWalker(m_reader, GET_LINK_OFFSET(staticDescriptor->initializer_call()));
}

tu_uint32
lyric_object::StaticWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_staticOffset;
}
