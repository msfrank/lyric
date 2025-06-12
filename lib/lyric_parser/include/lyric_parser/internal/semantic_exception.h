#ifndef LYRIC_PARSER_INTERNAL_SEMANTIC_EXCEPTION_H
#define LYRIC_PARSER_INTERNAL_SEMANTIC_EXCEPTION_H

#include <exception>
#include <string_view>

#include <antlr4-runtime.h>

namespace lyric_parser::internal {

    class SemanticException : public std::exception {
    public:
        SemanticException(const antlr4::Token *token, std::string_view message) noexcept;

        size_t getLineNr() const;
        size_t getColumnNr() const;
        std::string getMessage() const;

        const char *what() const noexcept override;

    private:
        size_t m_lineNr;
        size_t m_columnNr;
        std::string m_message;
    };
}

#endif // LYRIC_PARSER_INTERNAL_SEMANTIC_EXCEPTION_H
