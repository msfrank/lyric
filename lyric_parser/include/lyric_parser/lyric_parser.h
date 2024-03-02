#ifndef LYRIC_PARSER_LYRIC_PARSER_H
#define LYRIC_PARSER_LYRIC_PARSER_H

#include <filesystem>

#include <tempo_tracing/scope_manager.h>
#include <tempo_utils/result.h>

#include "lyric_archetype.h"

namespace lyric_parser {

    struct ParserOptions {
        std::vector<std::filesystem::path> bootDirectoryList;
        std::filesystem::path srcDirectoryPath;
    };

    class LyricParser {

    public:
        explicit LyricParser(const ParserOptions &options);
        LyricParser(const LyricParser &other);

        tempo_utils::Result<LyricArchetype> parseModule(
            std::string_view utf8,
            const tempo_utils::Url &sourceUrl,
            std::shared_ptr<tempo_tracing::TraceRecorder> recorder);

        tempo_utils::Result<LyricArchetype> parseBlock(
            std::string_view utf8,
            const tempo_utils::Url &sourceUrl,
            tempo_tracing::ScopeManager *scopeManager);

        tempo_utils::Result<LyricArchetype> parseClass(
            std::string_view utf8,
            const tempo_utils::Url &sourceUrl,
            tempo_tracing::ScopeManager *scopeManager);

        tempo_utils::Result<LyricArchetype> parseConcept(
            std::string_view utf8,
            const tempo_utils::Url &sourceUrl,
            tempo_tracing::ScopeManager *scopeManager);

        tempo_utils::Result<LyricArchetype> parseEnum(
            std::string_view utf8,
            const tempo_utils::Url &sourceUrl,
            tempo_tracing::ScopeManager *scopeManager);

        tempo_utils::Result<LyricArchetype> parseFunction(
            std::string_view utf8,
            const tempo_utils::Url &sourceUrl,
            tempo_tracing::ScopeManager *scopeManager);

        tempo_utils::Result<LyricArchetype> parseInstance(
            std::string_view utf8,
            const tempo_utils::Url &sourceUrl,
            tempo_tracing::ScopeManager *scopeManager);

        tempo_utils::Result<LyricArchetype> parseStruct(
            std::string_view utf8,
            const tempo_utils::Url &sourceUrl,
            tempo_tracing::ScopeManager *scopeManager);

    private:
        ParserOptions m_options;
    };
}

#endif // LYRIC_PARSER_LYRIC_PARSER_H