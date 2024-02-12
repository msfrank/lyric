
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <absl/strings/str_split.h>

#include <lyric_test/data_cell_matchers.h>
#include <lyric_test/test_result.h>
#include <tempo_utils/unicode.h>

lyric_test::matchers::DataCellMatcher::DataCellMatcher()
    : m_type(MatcherType::INVALID)
{
}

lyric_test::matchers::DataCellMatcher::DataCellMatcher(const lyric_runtime::DataCell &cell)
    : m_type(MatcherType::DATA_CELL),
      m_cell(cell)
{
}

lyric_test::matchers::DataCellMatcher::DataCellMatcher(const std::string &utf8)
    : m_type(MatcherType::DATA_CELL_UTF8),
      m_utf8(utf8)
{
}

lyric_test::matchers::DataCellMatcher::DataCellMatcher(lyric_object::LinkageSection section)
    : m_type(MatcherType::DATA_CELL_DESCRIPTOR)
{
    switch (section) {
        case lyric_object::LinkageSection::Type:
            m_cell.type = lyric_runtime::DataCellType::TYPE;
            break;
        case lyric_object::LinkageSection::Call:
            m_cell.type = lyric_runtime::DataCellType::CALL;
            break;
        case lyric_object::LinkageSection::Field:
            m_cell.type = lyric_runtime::DataCellType::FIELD;
            break;
        case lyric_object::LinkageSection::Enum:
            m_cell.type = lyric_runtime::DataCellType::ENUM;
            break;
        case lyric_object::LinkageSection::Action:
            m_cell.type = lyric_runtime::DataCellType::ACTION;
            break;
        case lyric_object::LinkageSection::Class:
            m_cell.type = lyric_runtime::DataCellType::CLASS;
            break;
        case lyric_object::LinkageSection::Instance:
            m_cell.type = lyric_runtime::DataCellType::INSTANCE;
            break;
        case lyric_object::LinkageSection::Struct:
            m_cell.type = lyric_runtime::DataCellType::STRUCT;
            break;
        case lyric_object::LinkageSection::Concept:
            m_cell.type = lyric_runtime::DataCellType::CONCEPT;
            break;
        case lyric_object::LinkageSection::Namespace:
            m_cell.type = lyric_runtime::DataCellType::NAMESPACE;
            break;
        default:
            TU_UNREACHABLE();
    }
}

lyric_test::matchers::DataCellMatcher::DataCellMatcher(lyric_runtime::DataCellType type)
    : m_type(MatcherType::DATA_CELL_TYPE)
{
    m_cell.type = type;
}

lyric_test::matchers::DataCellMatcher::DataCellMatcher(const DataCellMatcher &other)
    : m_type(other.m_type),
      m_cell(other.m_cell),
      m_utf8(other.m_utf8)
{
}

bool
lyric_test::matchers::DataCellMatcher::MatchAndExplain(
    const lyric_runtime::DataCell &cell,
    std::ostream* os) const
{
    switch (m_type) {
        case MatcherType::INVALID:
            return false;
        case MatcherType::DATA_CELL: {
            return cell == m_cell;
        }
        case MatcherType::DATA_CELL_UTF8: {
            if (cell.type != lyric_runtime::DataCellType::UTF8)
                return false;
            return std::string_view(cell.data.utf8.data, cell.data.utf8.size) == m_utf8;
        }
        case MatcherType::DATA_CELL_DESCRIPTOR: {
            return cell.type == m_cell.type;
        }
        case MatcherType::DATA_CELL_TYPE: {
            return cell.type == m_cell.type;
        }
    }
    TU_UNREACHABLE();
}

void
lyric_test::matchers::DataCellMatcher::DescribeTo(std::ostream* os) const
{
    switch (m_type) {
        case MatcherType::DATA_CELL:
            *os << m_cell.toString();
            break;
        case MatcherType::DATA_CELL_UTF8:
            *os << "cell contains utf8 string " << m_utf8;
            break;
        case MatcherType::DATA_CELL_DESCRIPTOR:
            switch (m_cell.type) {
                case lyric_runtime::DataCellType::TYPE:        *os << "cell contains type descriptor"; break;
                case lyric_runtime::DataCellType::CLASS:       *os << "cell contains class descriptor"; break;
                case lyric_runtime::DataCellType::STRUCT:      *os << "cell contains struct descriptor"; break;
                case lyric_runtime::DataCellType::INSTANCE:    *os << "cell contains instance descriptor"; break;
                case lyric_runtime::DataCellType::CONCEPT:     *os << "cell contains concept descriptor"; break;
                case lyric_runtime::DataCellType::CALL:        *os << "cell contains call descriptor"; break;
                case lyric_runtime::DataCellType::FIELD:       *os << "cell contains field descriptor"; break;
                case lyric_runtime::DataCellType::ACTION:      *os << "cell contains action descriptor"; break;
                case lyric_runtime::DataCellType::NAMESPACE:   *os << "cell contains action descriptor"; break;
                default:
                    TU_UNREACHABLE();
            }
            break;
        case MatcherType::DATA_CELL_TYPE: {
            switch (m_cell.type) {
                case lyric_runtime::DataCellType::INVALID:     *os << "cell contains invalid cell"; break;
                case lyric_runtime::DataCellType::NIL:         *os << "cell contains nil cell"; break;
                case lyric_runtime::DataCellType::PRESENT:     *os << "cell contains present cell"; break;
                case lyric_runtime::DataCellType::BOOL:        *os << "cell contains bool cell"; break;
                case lyric_runtime::DataCellType::I64:         *os << "cell contains i64 cell"; break;
                case lyric_runtime::DataCellType::DBL:         *os << "cell contains dbl cell"; break;
                case lyric_runtime::DataCellType::CHAR32:      *os << "cell contains chr cell"; break;
                case lyric_runtime::DataCellType::UTF8:        *os << "cell contains utf8 cell"; break;
                case lyric_runtime::DataCellType::REF:         *os << "cell contains ref cell"; break;
                case lyric_runtime::DataCellType::TYPE:        *os << "cell contains type cell"; break;
                case lyric_runtime::DataCellType::CLASS:       *os << "cell contains class cell"; break;
                case lyric_runtime::DataCellType::STRUCT:      *os << "cell contains struct cell"; break;
                case lyric_runtime::DataCellType::INSTANCE:    *os << "cell contains instance cell"; break;
                case lyric_runtime::DataCellType::CONCEPT:     *os << "cell contains concept cell"; break;
                case lyric_runtime::DataCellType::CALL:        *os << "cell contains call cell"; break;
                case lyric_runtime::DataCellType::FIELD:       *os << "cell contains field cell"; break;
                case lyric_runtime::DataCellType::ENUM:        *os << "cell contains enum cell"; break;
                case lyric_runtime::DataCellType::ACTION:      *os << "cell contains action cell"; break;
                case lyric_runtime::DataCellType::EXISTENTIAL: *os << "cell contains existential cell"; break;
                case lyric_runtime::DataCellType::NAMESPACE:   *os << "cell contains namespace cell"; break;
            }
            break;
        }
        default:
            *os << "!!! invalid matcher !!!";
    }
}

void
lyric_test::matchers::DataCellMatcher::DescribeNegationTo(std::ostream* os) const
{
    *os << "cell does not match";
}

testing::Matcher<lyric_runtime::DataCell>
lyric_test::matchers::DataCellNil()
{
    return DataCellMatcher(lyric_runtime::DataCell::nil());
}

testing::Matcher<lyric_runtime::DataCell>
lyric_test::matchers::DataCellBool(bool b)
{
    return DataCellMatcher(lyric_runtime::DataCell(b));
}

testing::Matcher<lyric_runtime::DataCell>
lyric_test::matchers::DataCellInt(int64_t i64)
{
    return DataCellMatcher(lyric_runtime::DataCell(i64));
}

testing::Matcher<lyric_runtime::DataCell>
lyric_test::matchers::DataCellFloat(double dbl)
{
    return DataCellMatcher(lyric_runtime::DataCell(dbl));
}

testing::Matcher<lyric_runtime::DataCell>
lyric_test::matchers::DataCellChar(UChar32 chr)
{
    return DataCellMatcher(lyric_runtime::DataCell(chr));
}

testing::Matcher<lyric_runtime::DataCell>
lyric_test::matchers::DataCellUtf8(const std::string &utf8)
{
    return DataCellMatcher(utf8);
}

testing::Matcher<lyric_runtime::DataCell>
lyric_test::matchers::MatchesDescriptorSection(const lyric_object::LinkageSection section)
{
    return DataCellMatcher(section);
}

testing::Matcher<lyric_runtime::DataCell>
lyric_test::matchers::MatchesDataCellType(lyric_runtime::DataCellType type)
{
    return DataCellMatcher(type);
}

std::ostream&
lyric_runtime::operator<<(std::ostream& os, const DataCell &cell) {
    return os << cell.toString();
}

void
lyric_runtime::PrintTo(const DataCell &status, std::ostream *os)
{
    *os << status.toString();
}
