#ifndef LYRIC_TEST_DATA_CELL_MATCHERS_H
#define LYRIC_TEST_DATA_CELL_MATCHERS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_compiler/compiler_result.h>
#include <lyric_parser/parse_result.h>
#include <lyric_runtime/bytecode_interpreter.h>
#include <lyric_runtime/interpreter_result.h>
#include <lyric_test/test_result.h>

using ::testing::Matcher;

namespace lyric_test {

    namespace matchers {

        class DataCellMatcher {

            enum class MatcherType {
                INVALID,
                DATA_CELL,
                DATA_CELL_STRING,
                DATA_CELL_URL,
                DATA_CELL_SYMBOL,
                DATA_CELL_DESCRIPTOR,
                DATA_CELL_TYPE,
            };

        public:
            DataCellMatcher();
            DataCellMatcher(const lyric_runtime::DataCell &cell);
            DataCellMatcher(const std::string &str);
            DataCellMatcher(const tempo_utils::Url &url);
            DataCellMatcher(const lyric_common::SymbolUrl &symbolUrl);
            DataCellMatcher(lyric_object::LinkageSection section);
            DataCellMatcher(lyric_runtime::DataCellType type);
            DataCellMatcher(const DataCellMatcher &other);

            bool MatchAndExplain(const lyric_runtime::DataCell &cell, std::ostream *os) const;
            void DescribeTo(std::ostream *os) const;
            void DescribeNegationTo(std::ostream *os) const;

            using MatchesType = lyric_runtime::DataCell;
            using is_gtest_matcher = void;

        private:
            MatcherType m_type;
            lyric_runtime::DataCell m_cell;
            std::string m_str;
            tempo_utils::Url m_url;
            lyric_common::SymbolUrl m_sym;
        };

        Matcher<lyric_runtime::DataCell> DataCellNil();
        Matcher<lyric_runtime::DataCell> DataCellUndef();
        Matcher<lyric_runtime::DataCell> DataCellBool(bool b);
        Matcher<lyric_runtime::DataCell> DataCellInt(int64_t i64);
        Matcher<lyric_runtime::DataCell> DataCellFloat(double dbl);
        Matcher<lyric_runtime::DataCell> DataCellChar(UChar32 chr);
        Matcher<lyric_runtime::DataCell> DataCellString(std::string_view str);
        Matcher<lyric_runtime::DataCell> DataCellUrl(std::string_view url);
        Matcher<lyric_runtime::DataCell> DataCellUrl(const tempo_utils::Url &url);
        Matcher<lyric_runtime::DataCell> DataCellRef(const lyric_common::SymbolUrl &symbolUrl);
        Matcher<lyric_runtime::DataCell> DataCellRef(const lyric_common::SymbolPath &symbolPath);
        Matcher<lyric_runtime::DataCell> MatchesDescriptorSection(lyric_object::LinkageSection section);
        Matcher<lyric_runtime::DataCell> MatchesDataCellType(lyric_runtime::DataCellType type);
    }
}

namespace lyric_runtime {
    void PrintTo(const lyric_runtime::DataCell &cell, std::ostream *os);
    std::ostream& operator<<(std::ostream& os, const DataCell &cell);
}

#endif // LYRIC_TEST_DATA_CELL_MATCHERS_H
