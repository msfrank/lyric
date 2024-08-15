
#include <lyric_object/import_walker.h>
#include <lyric_object/internal/object_reader.h>

lyric_object::ImportWalker::ImportWalker()
    : m_importOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::ImportWalker::ImportWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 importOffset)
    : m_reader(reader),
      m_importOffset(importOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::ImportWalker::ImportWalker(const ImportWalker &other)
    : m_reader(other.m_reader),
      m_importOffset(other.m_importOffset)
{
}

bool
lyric_object::ImportWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_importOffset < m_reader->numImports();
}

bool
lyric_object::ImportWalker::isSystemBootstrap() const
{
    if (!isValid())
        return false;
    auto *importDescriptor = m_reader->getImport(m_importOffset);
    if (importDescriptor == nullptr)
        return false;
    return bool(importDescriptor->flags() & lyo1::ImportFlags::SystemBootstrap);
}

lyric_common::ModuleLocation
lyric_object::ImportWalker::getImportLocation() const
{
    if (!isValid())
        return {};
    auto *importDescriptor = m_reader->getImport(m_importOffset);
    if (importDescriptor == nullptr)
        return {};
    if (importDescriptor->import_location() == nullptr)
        return {};
    return lyric_common::ModuleLocation::fromString(importDescriptor->import_location()->str());
}

tu_uint32
lyric_object::ImportWalker::getDescriptorOffset() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_importOffset;
}
