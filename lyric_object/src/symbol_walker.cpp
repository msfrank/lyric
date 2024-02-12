
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/symbol_walker.h>

lyric_object::SymbolWalker::SymbolWalker()
    : m_reader(),
      m_section(static_cast<tu_uint8>(lyo1::DescriptorSection::Invalid)),
      m_descriptor(INVALID_ADDRESS_U32)
{
}

lyric_object::SymbolWalker::SymbolWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    tu_uint8 section,
    tu_uint32 descriptor)
    : m_reader(reader),
      m_section(section),
      m_descriptor(descriptor)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_section != static_cast<tu_uint8>(lyo1::DescriptorSection::Invalid));
    TU_ASSERT (m_descriptor != INVALID_ADDRESS_U32);
}

lyric_object::SymbolWalker::SymbolWalker(const SymbolWalker &other)
    : m_reader(other.m_reader),
      m_section(other.m_section),
      m_descriptor(other.m_descriptor)
{
}

bool
lyric_object::SymbolWalker::isValid() const
{
    return m_reader && m_reader->isValid()
        && m_section != static_cast<tu_uint8>(lyo1::DescriptorSection::Invalid)
        && m_descriptor != INVALID_ADDRESS_U32;
}

lyric_common::SymbolPath
lyric_object::SymbolWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    return m_reader->getSymbolPath(static_cast<lyo1::DescriptorSection>(m_section), m_descriptor);
}

lyric_object::LinkageSection
lyric_object::SymbolWalker::getLinkageSection() const
{
    if (!isValid())
        return LinkageSection::Invalid;
    return internal::descriptor_to_linkage_section(static_cast<lyo1::DescriptorSection>(m_section));
}

tu_uint32
lyric_object::SymbolWalker::getLinkageIndex() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    return m_descriptor;
}