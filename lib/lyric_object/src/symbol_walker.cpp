
#include <lyric_object/internal/object_reader.h>
#include <lyric_object/symbol_walker.h>

lyric_object::SymbolWalker::SymbolWalker()
    : m_reader(),
      m_symbol(nullptr)
{
}

lyric_object::SymbolWalker::SymbolWalker(
    std::shared_ptr<const internal::ObjectReader> reader,
    void *symbol)
    : m_reader(reader),
      m_symbol(symbol)
{
    TU_ASSERT (m_reader != nullptr);
    TU_ASSERT (m_symbol != nullptr);
}

lyric_object::SymbolWalker::SymbolWalker(const SymbolWalker &other)
    : m_reader(other.m_reader),
      m_symbol(other.m_symbol)
{
}

bool
lyric_object::SymbolWalker::isValid() const
{
    if (m_reader && m_reader->isValid() && m_symbol) {
        auto *symbol = static_cast<lyo1::SymbolDescriptor *>(m_symbol);
        return symbol->symbol_type() != lyo1::DescriptorSection::Invalid;
    }
    return false;
}

lyric_common::SymbolPath
lyric_object::SymbolWalker::getSymbolPath() const
{
    if (!isValid())
        return {};
    auto *symbol = static_cast<lyo1::SymbolDescriptor *>(m_symbol);
    return lyric_common::SymbolPath::fromString(symbol->fqsn()->str());
}

lyric_object::LinkageSection
lyric_object::SymbolWalker::getLinkageSection() const
{
    if (!isValid())
        return LinkageSection::Invalid;
    auto *symbol = static_cast<lyo1::SymbolDescriptor *>(m_symbol);
    return internal::descriptor_to_linkage_section(symbol->symbol_type());
}

tu_uint32
lyric_object::SymbolWalker::getLinkageIndex() const
{
    if (!isValid())
        return INVALID_ADDRESS_U32;
    auto *symbol = static_cast<lyo1::SymbolDescriptor *>(m_symbol);
    return symbol->symbol_descriptor();
}