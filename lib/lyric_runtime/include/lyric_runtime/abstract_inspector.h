#ifndef LYRIC_RUNTIME_ABSTRACT_INSPECTOR_H
#define LYRIC_RUNTIME_ABSTRACT_INSPECTOR_H

#include <lyric_object/bytecode_iterator.h>
#include <tempo_utils/status.h>
#include <tempo_utils/result.h>

#include "operand.h"

namespace lyric_runtime {

    class BytecodeInterpreter;
    class InterpreterState;

    class AbstractInspector {

    public:
        virtual ~AbstractInspector() = default;

        virtual tempo_utils::Status beforeOp(
            const lyric_object::OpCell &op,
            BytecodeInterpreter *interp,
            InterpreterState *state) = 0;

        virtual tempo_utils::Status afterOp(
            const lyric_object::OpCell &op,
            BytecodeInterpreter *interp,
            InterpreterState *state) = 0;

        virtual tempo_utils::Status onInterrupt(
            const Operand &cell,
            BytecodeInterpreter *interp,
            InterpreterState *state) = 0;

        virtual tempo_utils::Result<Operand> onError(
            const lyric_object::OpCell &op,
            const tempo_utils::Status &status,
            BytecodeInterpreter *interp,
            InterpreterState *state) = 0;

        virtual tempo_utils::Result<Operand> onHalt(
            const lyric_object::OpCell &op,
            const Operand &cell,
            BytecodeInterpreter *interp,
            InterpreterState *state) = 0;
    };
}

#endif // LYRIC_RUNTIME_ABSTRACT_INSPECTOR_H
