#ifndef LYRIC_RUNTIME_BYTECODE_INTERPRETER_H
#define LYRIC_RUNTIME_BYTECODE_INTERPRETER_H

#include "abstract_inspector.h"
#include "data_cell.h"
#include "interpreter_state.h"
#include "interpreter_result.h"
#include "ref_handle.h"
#include "runtime_types.h"
#include "system_scheduler.h"

namespace lyric_runtime {

    enum class ReturnType {
        INVALID,
        VALUE,
        REF,
        EXCEPTION,
    };

    struct Return {
        ReturnType type;
        DataCell value;
        RefHandle ref;
        Return() : type(ReturnType::INVALID) {};
        Return(const DataCell &value) : type(ReturnType::VALUE), value(value) {};
        Return(const RefHandle &ref) : type(ReturnType::REF), ref(ref) {};
    };

    class BytecodeInterpreter {

    public:
        BytecodeInterpreter(std::shared_ptr<InterpreterState> state, AbstractInspector *inspector = nullptr);

        InterpreterState *interpreterState() const;
        AbstractInspector *interpreterInspector() const;

        tempo_utils::Result<Return> run();
        tempo_utils::Result<DataCell> runSubinterpreter();
        tempo_utils::Status interrupt();

        int getRecursionDepth() const;
        void incrementRecursionDepth();
        void decrementRecursionDepth();

    private:
        std::shared_ptr<InterpreterState> m_state;
        AbstractInspector *m_inspector;
        int m_recursionDepth;

        tempo_utils::Status onInterrupt(const DataCell &cell);
        tempo_utils::Result<DataCell> onError(const lyric_object::OpCell &op, const tempo_utils::Status &status);
        tempo_utils::Result<DataCell> onHalt(const lyric_object::OpCell &op, const DataCell &cell);
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
