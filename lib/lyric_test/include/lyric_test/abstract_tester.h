#ifndef LYRIC_TEST_ABSTRACT_TESTER_H
#define LYRIC_TEST_ABSTRACT_TESTER_H

#include <lyric_build/lyric_builder.h>
#include <tempo_tracing/tempo_spanset.h>

namespace lyric_test {

    class AbstractTester {

    public:
        virtual ~AbstractTester() = default;

        virtual std::filesystem::path getTesterDirectory() const = 0;

        virtual lyric_build::LyricBuilder *getBuilder() const = 0;

        virtual tempo_tracing::TempoSpanset getDiagnostics(
            const lyric_build::TargetComputation &computation) const = 0;
    };
}

#endif // LYRIC_TEST_ABSTRACT_TESTER_H