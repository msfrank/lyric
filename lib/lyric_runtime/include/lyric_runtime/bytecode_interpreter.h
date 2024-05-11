#ifndef LYRIC_RUNTIME_BYTECODE_INTERPRETER_H
#define LYRIC_RUNTIME_BYTECODE_INTERPRETER_H

#include <tempo_utils/hdr_histogram.h>

#include "abstract_inspector.h"
#include "data_cell.h"
#include "interpreter_state.h"
#include "interpreter_result.h"
#include "ref_handle.h"
#include "runtime_types.h"
#include "system_scheduler.h"

namespace lyric_runtime {

    /**
     *
     */
    struct InterpreterExit {
        tempo_utils::StatusCode statusCode;         /**< Status code returned by the program. */
        DataCell mainReturn;                        /**< The return value of the main task. */
        tu_uint64 interpreterStartEpochMillis;      /**< Timestamp when the interpreter started, in milliseconds since the epoch. */
        tu_uint64 instructionCount;                 /**< Total count of instructions executed by the interpreter. */
    };

    struct InterpreterStats {
        tu_uint64 instructionCount;
        std::unique_ptr<tempo_utils::HdrHistogram> instructionsPerSlice;
        std::unique_ptr<tempo_utils::HdrHistogram> microsecondsPerPoll;
    };

    class BytecodeInterpreter {

    public:
        explicit BytecodeInterpreter(std::shared_ptr<InterpreterState> state, AbstractInspector *inspector = nullptr);

        InterpreterState *interpreterState() const;
        AbstractInspector *interpreterInspector() const;

        tempo_utils::Result<InterpreterExit> run();
        tempo_utils::Result<DataCell> runSubinterpreter();
        tempo_utils::Status interrupt();

        tu_uint16 getSliceCounter() const;
        tu_uint64 getInstructionCounter() const;

        int getRecursionDepth() const;
        void incrementRecursionDepth();
        void decrementRecursionDepth();

    private:
        std::shared_ptr<InterpreterState> m_state;
        AbstractInspector *m_inspector;
        tu_uint16 m_sliceCounter;
        tu_uint64 m_instructionCounter;
        int m_recursionDepth;

        tempo_utils::Status onInterrupt(const DataCell &cell);
        tempo_utils::Result<DataCell> onError(const lyric_object::OpCell &op, const tempo_utils::Status &status);
        tempo_utils::Result<DataCell> onHalt(const lyric_object::OpCell &op);
    };

    class RecursionLocker {

    public:
        explicit RecursionLocker(lyric_runtime::BytecodeInterpreter *interp);
        ~RecursionLocker();
        int getRecursionDepth() const;

    private:
        lyric_runtime::BytecodeInterpreter *m_interp;
    };
}

#endif // LYRIC_RUNTIME_BYTECODE_INTERPRETER_H
