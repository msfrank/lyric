#ifndef LYRIC_ANALYZER_LYRIC_ANALYZER_H
#define LYRIC_ANALYZER_LYRIC_ANALYZER_H

#include <absl/container/flat_hash_map.h>

#include <lyric_assembler/assembly_state.h>
#include <lyric_common/assembly_location.h>
#include <lyric_common/symbol_url.h>
#include <lyric_parser/lyric_archetype.h>
#include <lyric_object/lyric_object.h>
#include <lyric_runtime/abstract_loader.h>
#include <tempo_tracing/trace_recorder.h>

namespace lyric_analyzer {

    struct AnalyzerOptions {
        /** */
        absl::flat_hash_map<
            lyric_common::AssemblyLocation,
            absl::flat_hash_set<lyric_common::SymbolPath>> envSymbols;

        /** */
        bool touchExternalSymbols;
    };

    class LyricAnalyzer {

    public:
        explicit LyricAnalyzer(
            std::shared_ptr<lyric_importer::ModuleCache> systemModuleCache,
            const AnalyzerOptions &options);
        LyricAnalyzer(const LyricAnalyzer &other);

        tempo_utils::Result<lyric_object::LyricObject> analyzeModule(
            const lyric_common::AssemblyLocation &location,
            const lyric_parser::LyricArchetype &archetype,
            const lyric_assembler::AssemblyStateOptions &assemblyStateOptions,
            std::shared_ptr<tempo_tracing::TraceRecorder> recorder);

    private:
        std::shared_ptr<lyric_importer::ModuleCache> m_systemModuleCache;
        AnalyzerOptions m_options;
    };
}

#endif // LYRIC_ANALYZER_LYRIC_ANALYZER_H