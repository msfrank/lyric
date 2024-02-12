#ifndef LYRIC_TEST_TEST_INSPECTOR_H
#define LYRIC_TEST_TEST_INSPECTOR_H

#include <lyric_runtime/abstract_inspector.h>

namespace lyric_test {

    class TestInspector : public lyric_runtime::AbstractInspector {
    public:
        TestInspector();

        tempo_utils::Status beforeOp(
            const lyric_object::OpCell &op,
            lyric_runtime::InterpreterState *state) override;
        tempo_utils::Status afterOp(
            const lyric_object::OpCell &op,
            lyric_runtime::InterpreterState *state) override;
        tempo_utils::Status onInterrupt(
            const lyric_runtime::DataCell &cell,
            lyric_runtime::InterpreterState *state) override;
        tempo_utils::Result<lyric_runtime::DataCell> onError(
            const lyric_object::OpCell &op,
            const tempo_utils::Status &status,
            lyric_runtime::InterpreterState *state) override;
        tempo_utils::Result<lyric_runtime::DataCell> onHalt(
            const lyric_object::OpCell &op,
            const lyric_runtime::DataCell &cell,
            lyric_runtime::InterpreterState *state) override;

        void printCallStack(lyric_runtime::InterpreterState *state) const;
        void printDataStack(lyric_runtime::InterpreterState *state, int untilFrameNr = 0) const;
        void printDataStackForFrame(lyric_runtime::InterpreterState *state, int frameNr) const;
    };
}

#endif // LYRIC_TEST_TEST_INSPECTOR_H
