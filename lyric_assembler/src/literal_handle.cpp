
#include <lyric_assembler/literal_handle.h>

lyric_assembler::LiteralHandle::LiteralHandle()
    : m_type(lyric_runtime::LiteralCellType::INVALID)
{
}

lyric_assembler::LiteralHandle::LiteralHandle(const LiteralAddress &address)
    : m_address(address),
      m_type(lyric_runtime::LiteralCellType::NIL)
{
}

lyric_assembler::LiteralHandle::LiteralHandle(const LiteralAddress &address, bool b)
    : m_address(address),
      m_type(lyric_runtime::LiteralCellType::BOOL)
{
    m_value.b = b;
}

lyric_assembler::LiteralHandle::LiteralHandle(const LiteralAddress &address, int64_t i64)
    : m_address(address),
      m_type(lyric_runtime::LiteralCellType::I64)
{
    m_value.i64 = i64;
}

lyric_assembler::LiteralHandle::LiteralHandle(const LiteralAddress &address, double dbl)
    : m_address(address),
      m_type(lyric_runtime::LiteralCellType::DBL)
{
    m_value.dbl = dbl;
}

lyric_assembler::LiteralHandle::LiteralHandle(const LiteralAddress &address, UChar32 chr)
    : m_address(address),
      m_type(lyric_runtime::LiteralCellType::CHAR32)
{
    m_value.chr = chr;
}

lyric_assembler::LiteralHandle::LiteralHandle(const LiteralAddress &address, const std::string &str)
    : m_address(address),
      m_type(lyric_runtime::LiteralCellType::UTF8)
{
    m_str = std::make_shared<const std::string>(str);
}

lyric_assembler::LiteralAddress
lyric_assembler::LiteralHandle::LiteralHandle::getAddress() const
{
    return m_address;
}

lyric_runtime::LiteralCellType
lyric_assembler::LiteralHandle::LiteralHandle::getType() const
{
    return m_type;
}

bool
lyric_assembler::LiteralHandle::getBool() const
{
    return m_type == lyric_runtime::LiteralCellType::BOOL ? m_value.b : false;
}

int64_t
lyric_assembler::LiteralHandle::getInt64() const
{
    return m_type == lyric_runtime::LiteralCellType::I64 ? m_value.i64 : 0;
}

double
lyric_assembler::LiteralHandle::getDouble() const
{
    return m_type == lyric_runtime::LiteralCellType::DBL ? m_value.dbl : 0.0;
}

UChar32
lyric_assembler::LiteralHandle::getUChar32() const
{
    return m_type == lyric_runtime::LiteralCellType::CHAR32 ? m_value.chr : 0;
}

std::shared_ptr<const std::string>
lyric_assembler::LiteralHandle::getString() const
{
    return m_type == lyric_runtime::LiteralCellType::UTF8 ? m_str : nullptr;
}