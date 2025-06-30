
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/symbol_walker.h>

lyric_object::SymbolWalker::SymbolWalker()
    : m_symbolOffset(INVALID_ADDRESS_U32)
{
}

lyric_object::SymbolWalker::SymbolWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    tu_uint32 symbolOffset)
    : m_reader(reader),
      m_symbolOffset(symbolOffset)
{
    TU_ASSERT (m_reader != nullptr);
}

lyric_object::SymbolWalker::SymbolWalker(const SymbolWalker &other)
    : m_reader(other.m_reader),
      m_symbolOffset(other.m_symbolOffset)
{
}

bool
lyric_object::SymbolWalker::isValid() const
{
    return m_reader && m_reader->isValid() && m_symbolOffset < m_reader->numSymbols();
}

lyric_common::SymbolPath
lyric_object::SymbolWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *symbolDescriptor = m_reader->getSymbol(m_symbolOffset);
    if (symbolDescriptor == nullptr)
        return {};
    return m_reader->getSymbolPath(symbolDescriptor->symbol_type(), symbolDescriptor->symbol_descriptor());
}

lyric_object::LinkageSection
lyric_object::SymbolWalker::getLinkageSection() const
{
    if (!isValid())
        return LinkageSection::Invalid;
    auto *symbolDescriptor = m_reader->getSymbol(m_symbolOffset);
    if (symbolDescriptor == nullptr)
        return LinkageSection::Invalid;
    return internal::descriptor_to_linkage_section(symbolDescriptor->symbol_type());
}

tu_uint32
lyric_object::SymbolWalker::getLinkageIndex() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    auto *symbolDescriptor = m_reader->getSymbol(m_symbolOffset);
    if (symbolDescriptor == nullptr)
        return INVALID_ADDRESS_U32;
    return symbolDescriptor->symbol_descriptor();
}

tu_uint32
lyric_object::SymbolWalker::getDescriptorOffset() const
{
    return m_symbolOffset;
}