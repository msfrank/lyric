#ifndef LYRIC_BUILD_BUILD_DIAGNOSTICS_H
#define LYRIC_BUILD_BUILD_DIAGNOSTICS_H

#include <tempo_tracing/tempo_spanset.h>

#include "build_types.h"

namespace lyric_build {

    struct BuildDiagnosticsOptions {
        absl::flat_hash_set<TaskId> targets;
        bool skipUnknownTargets = false;
    };

    class BuildDiagnostics : public std::enable_shared_from_this<BuildDiagnostics> {

    public:
        static tempo_utils::Result<std::shared_ptr<BuildDiagnostics>> create(
            const tempo_tracing::TempoSpanset &spanset,
            const BuildDiagnosticsOptions &options);

        tempo_tracing::TempoSpanset getSpanset() const;

        void printDiagnostics() const;

    private:
        tempo_tracing::TempoSpanset m_spanset;
        BuildDiagnosticsOptions m_options;

        BuildDiagnostics(const tempo_tracing::TempoSpanset &spanset, const BuildDiagnosticsOptions &options);
    };
}

#endif //LYRIC_BUILD_BUILD_DIAGNOSTICS_H
