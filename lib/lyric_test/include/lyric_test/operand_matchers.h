#ifndef LYRIC_TEST_OPERAND_MATCHERS_H
#define LYRIC_TEST_OPERAND_MATCHERS_H

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

        class OperandMatcher {

            enum class MatcherType {
                Invalid,
                Operand,
                String,
                Bytes,
                Symbol,
                Descriptor,
                Type,
            };

        public:
            OperandMatcher();
            OperandMatcher(const lyric_runtime::Operand &cell);
            OperandMatcher(const std::string &str);
            OperandMatcher(std::shared_ptr<const tempo_utils::ImmutableBytes> bytes);
            OperandMatcher(const lyric_common::SymbolUrl &symbolUrl);
            OperandMatcher(lyric_object::LinkageSection section);
            OperandMatcher(lyric_runtime::OperandType type);
            OperandMatcher(const OperandMatcher &other);

            bool MatchAndExplain(const lyric_runtime::Operand &cell, std::ostream *os) const;
            void DescribeTo(std::ostream *os) const;
            void DescribeNegationTo(std::ostream *os) const;

            using MatchesType = lyric_runtime::Operand;
            using is_gtest_matcher = void;

        private:
            MatcherType m_type = MatcherType::Invalid;
            lyric_object::LinkageSection m_section = lyric_object::LinkageSection::Invalid;
            lyric_runtime::Operand m_cell;
            lyric_runtime::OperandType m_optype = lyric_runtime::OperandType::Invalid;
            std::string m_str;
            tempo_utils::Url m_url;
            std::shared_ptr<const tempo_utils::ImmutableBytes> m_bytes;
            lyric_common::SymbolUrl m_sym;
        };

        Matcher<lyric_runtime::Operand> OperandNil();
        Matcher<lyric_runtime::Operand> OperandUndef();
        Matcher<lyric_runtime::Operand> OperandBool(bool b);
        Matcher<lyric_runtime::Operand> OperandInt(int64_t i64);
        Matcher<lyric_runtime::Operand> OperandFloat(double dbl);
        Matcher<lyric_runtime::Operand> OperandChar(char32_t chr);
        Matcher<lyric_runtime::Operand> OperandString(std::string_view str);
        Matcher<lyric_runtime::Operand> OperandBytes(std::string_view bytes);
        Matcher<lyric_runtime::Operand> OperandBytes(std::span<const tu_uint8> bytes);
        Matcher<lyric_runtime::Operand> OperandBytes(std::shared_ptr<const tempo_utils::ImmutableBytes> bytes);
        Matcher<lyric_runtime::Operand> OperandRef(const lyric_common::SymbolUrl &symbolUrl);
        Matcher<lyric_runtime::Operand> OperandRef(const lyric_common::SymbolPath &symbolPath);
        Matcher<lyric_runtime::Operand> MatchesDescriptorSection(lyric_object::LinkageSection section);
        Matcher<lyric_runtime::Operand> MatchesDataCellType(lyric_runtime::OperandType type);
    }
}

namespace lyric_runtime {
    void PrintTo(const Operand &cell, std::ostream *os);
    std::ostream& operator<<(std::ostream& os, const Operand &cell);
}

#endif // LYRIC_TEST_OPERAND_MATCHERS_H
