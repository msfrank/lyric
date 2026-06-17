
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <absl/strings/str_split.h>

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/string_ref.h>
#include <lyric_test/operand_matchers.h>
#include <tempo_utils/memory_bytes.h>
#include <tempo_utils/unicode.h>

#include "lyric_runtime/bytes_ref.h"

lyric_test::matchers::OperandMatcher::OperandMatcher()
    : m_type(MatcherType::Invalid)
{
}

lyric_test::matchers::OperandMatcher::OperandMatcher(const lyric_runtime::Operand &cell)
    : m_type(MatcherType::Operand),
      m_cell(cell)
{
}

lyric_test::matchers::OperandMatcher::OperandMatcher(const std::string &str)
    : m_type(MatcherType::String),
      m_str(str)
{
}

lyric_test::matchers::OperandMatcher::OperandMatcher(std::shared_ptr<const tempo_utils::ImmutableBytes> bytes)
    : m_type(MatcherType::Bytes),
      m_bytes(bytes)
{
}

lyric_test::matchers::OperandMatcher::OperandMatcher(const lyric_common::SymbolUrl &symbolUrl)
    : m_type(MatcherType::Symbol),
      m_sym(symbolUrl)
{
}

lyric_test::matchers::OperandMatcher::OperandMatcher(lyric_object::LinkageSection section)
    : m_type(MatcherType::Descriptor),
      m_section(section)
{
}

lyric_test::matchers::OperandMatcher::OperandMatcher(lyric_runtime::OperandType type)
    : m_type(MatcherType::Type)
{
    m_optype = type;
}

lyric_test::matchers::OperandMatcher::OperandMatcher(const OperandMatcher &other)
    : m_type(other.m_type),
      m_section(other.m_section),
      m_cell(other.m_cell),
      m_optype(other.m_optype),
      m_str(other.m_str),
      m_url(other.m_url),
      m_bytes(other.m_bytes),
      m_sym(other.m_sym)
{
}

bool
lyric_test::matchers::OperandMatcher::MatchAndExplain(
    const lyric_runtime::Operand &cell,
    std::ostream* os) const
{
    switch (m_type) {
        case MatcherType::Invalid:
            return false;
        case MatcherType::Operand:
            return cell.isEqualTo(m_cell);
        case MatcherType::String: {
            lyric_runtime::StringRef *string;
            if (!cell.getString(string))
                return false;
            std::string str;
            if (!string->utf8Value(str))
                return false;
            return m_str == str;
        }
        case MatcherType::Bytes: {
            lyric_runtime::BytesRef *bytes;
            if (!cell.getBytes(bytes))
                return false;
            auto data = bytes->getBytesData().toVector();
            return std::equal(data.begin(), data.end(), m_bytes->getSpan().begin());
        }
        case MatcherType::Symbol: {
            lyric_runtime::BaseRef *ref;
            if (!cell.getRef(ref))
                return false;
            auto *vtable = ref->getVirtualTable();
            if (vtable == nullptr)
                return false;
            auto sym = vtable->getSymbolUrl();
            if (!m_sym.getModuleLocation().isValid())
                return sym.getSymbolPath() == m_sym.getSymbolPath();
            return m_sym == sym;
        }
        case MatcherType::Descriptor: {
            if (m_section == lyric_object::LinkageSection::Type)
                return cell.getType() == lyric_runtime::OperandType::Type;
            lyric_runtime::DescriptorEntry *descriptor;
            if (!cell.getDescriptor(descriptor))
                return false;
            return descriptor->getLinkageSection() == m_section;
        }
        case MatcherType::Type:
            return cell.getType() == m_optype;
    }
    TU_UNREACHABLE();
}

void
lyric_test::matchers::OperandMatcher::DescribeTo(std::ostream* os) const
{
    switch (m_type) {
        case MatcherType::Operand:
            *os << m_cell.toString();
            break;
        case MatcherType::String:
            *os << "cell contains string \"" << m_str << "\"";
            break;
        case MatcherType::Bytes:
            *os << "cell contains bytes of length " << m_bytes->getSize();
            break;
        case MatcherType::Symbol:
            *os << "cell contains instance of " << m_sym.toString();
            break;
        case MatcherType::Descriptor:
            *os << "cell contains " << lyric_object::linkage_section_to_name(m_section) << " descriptor";
            break;
        case MatcherType::Type: {
            switch (m_cell.getType()) {
                case lyric_runtime::OperandType::Invalid:     *os << "cell contains invalid cell"; break;
                case lyric_runtime::OperandType::Nil:         *os << "cell contains nil cell"; break;
                case lyric_runtime::OperandType::Undef:       *os << "cell contains undef cell"; break;
                case lyric_runtime::OperandType::Bool:        *os << "cell contains bool cell"; break;
                case lyric_runtime::OperandType::Int8:        *os << "cell contains i8 cell"; break;
                case lyric_runtime::OperandType::Int16:       *os << "cell contains i16 cell"; break;
                case lyric_runtime::OperandType::Int32:       *os << "cell contains i32 cell"; break;
                case lyric_runtime::OperandType::Int64:       *os << "cell contains i64 cell"; break;
                case lyric_runtime::OperandType::UInt8:       *os << "cell contains u8 cell"; break;
                case lyric_runtime::OperandType::UInt16:      *os << "cell contains u16 cell"; break;
                case lyric_runtime::OperandType::UInt32:      *os << "cell contains u32 cell"; break;
                case lyric_runtime::OperandType::UInt64:      *os << "cell contains u64 cell"; break;
                case lyric_runtime::OperandType::Float32:     *os << "cell contains f32 cell"; break;
                case lyric_runtime::OperandType::Float64:     *os << "cell contains f64 cell"; break;
                case lyric_runtime::OperandType::Char32:      *os << "cell contains chr cell"; break;
                case lyric_runtime::OperandType::Bytes:       *os << "cell contains bytes cell"; break;
                case lyric_runtime::OperandType::String:      *os << "cell contains string cell"; break;
                case lyric_runtime::OperandType::Status:      *os << "cell contains status cell"; break;
                case lyric_runtime::OperandType::Namespace:   *os << "cell contains namespace cell"; break;
                case lyric_runtime::OperandType::Protocol:    *os << "cell contains protocol cell"; break;
                case lyric_runtime::OperandType::Rest:        *os << "cell contains rest cell"; break;
                case lyric_runtime::OperandType::Ref:         *os << "cell contains ref cell"; break;
                case lyric_runtime::OperandType::Descriptor:  *os << "cell contains descriptor cell"; break;
                case lyric_runtime::OperandType::Type:        *os << "cell contains type cell"; break;
            }
            break;
        }
        default:
            *os << "!!! invalid matcher !!!";
    }
}

void
lyric_test::matchers::OperandMatcher::DescribeNegationTo(std::ostream* os) const
{
    *os << "cell does not match";
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandNil()
{
    return OperandMatcher(lyric_runtime::Operand::nil());
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandUndef()
{
    return OperandMatcher(lyric_runtime::Operand::undef());
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandBool(bool b)
{
    return OperandMatcher(lyric_runtime::Operand::fromBool(b));
}

Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandI64(tu_int64 i64)
{
    return OperandMatcher(lyric_runtime::Operand::fromI64(i64));
}

Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandI32(tu_int32 i32)
{
    return OperandMatcher(lyric_runtime::Operand::fromI32(i32));
}

Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandI16(tu_int16 i16)
{
    return OperandMatcher(lyric_runtime::Operand::fromI16(i16));
}

Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandI8(tu_int8 i8)
{
    return OperandMatcher(lyric_runtime::Operand::fromI8(i8));
}

Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandU64(tu_uint64 u64)
{
    return OperandMatcher(lyric_runtime::Operand::fromU64(u64));
}

Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandU32(tu_uint32 u32)
{
    return OperandMatcher(lyric_runtime::Operand::fromU32(u32));
}

Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandU16(tu_uint16 u16)
{
    return OperandMatcher(lyric_runtime::Operand::fromU16(u16));
}

Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandU8(tu_uint8 u8)
{
    return OperandMatcher(lyric_runtime::Operand::fromU8(u8));
}

Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandF64(double f64)
{
    return OperandMatcher(lyric_runtime::Operand::fromF64(f64));
}

Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandF32(float f32)
{
    return OperandMatcher(lyric_runtime::Operand::fromF32(f32));
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandInt(tu_int64 i64)
{
    return OperandMatcher(lyric_runtime::Operand::fromI64(i64));
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandFloat(double dbl)
{
    return OperandMatcher(lyric_runtime::Operand::fromF64(dbl));
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandChar(char32_t chr)
{
    return OperandMatcher(lyric_runtime::Operand::fromC32(chr));
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandString(std::string_view str)
{
    return OperandMatcher(std::string(str));
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandBytes(std::string_view bytes)
{
    return OperandMatcher(tempo_utils::MemoryBytes::copy(bytes));
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandBytes(std::span<const tu_uint8> bytes)
{
    return OperandMatcher(tempo_utils::MemoryBytes::copy(bytes));
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandBytes(std::shared_ptr<const tempo_utils::ImmutableBytes> bytes)
{
    return OperandMatcher(bytes);
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandRef(const lyric_common::SymbolUrl &symbolUrl)
{
    return OperandMatcher(symbolUrl);
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::OperandRef(const lyric_common::SymbolPath &symbolPath)
{
    return OperandMatcher(lyric_common::SymbolUrl(symbolPath));
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::MatchesDescriptorSection(lyric_object::LinkageSection section)
{
    return OperandMatcher(section);
}

testing::Matcher<lyric_runtime::Operand>
lyric_test::matchers::MatchesDataCellType(lyric_runtime::OperandType type)
{
    return OperandMatcher(type);
}

std::ostream&
lyric_runtime::operator<<(std::ostream& os, const Operand &cell) {
    return os << cell.toString();
}

void
lyric_runtime::PrintTo(const Operand &status, std::ostream *os)
{
    *os << status.toString();
}
