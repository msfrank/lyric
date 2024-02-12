#ifndef LYRIC_RUNTIME_ABSTRACT_INSPECTOR_H
#define LYRIC_RUNTIME_ABSTRACT_INSPECTOR_H

#include <lyric_object/bytecode_iterator.h>

#include "interpreter_result.h"

namespace lyric_runtime {

    class InterpreterState;

    class AbstractInspector {

    public:
        virtual ~AbstractInspector() = default;

        virtual tempo_utils::Status beforeOp(const lyric_object::OpCell &op, InterpreterState *state) = 0;

        virtual tempo_utils::Status afterOp(const lyric_object::OpCell &op, InterpreterState *state) = 0;

        virtual tempo_utils::Status onInterrupt(const DataCell &cell, InterpreterState *state) = 0;

        virtual tempo_utils::Result<DataCell> onError(
            const lyric_object::OpCell &op,
            const tempo_utils::Status &status,
            InterpreterState *state) = 0;

        virtual tempo_utils::Result<DataCell> onHalt(
            const lyric_object::OpCell &op,
            const DataCell &cell,
            InterpreterState *state) = 0;
    };
}

#endif // LYRIC_RUNTIME_ABSTRACT_INSPECTOR_H
