#ifndef LYRIC_RUNTIME_STACKFUL_COROUTINE_H
#define LYRIC_RUNTIME_STACKFUL_COROUTINE_H

#include <vector>

#include "bytecode_segment.h"
#include "call_cell.h"
#include "data_cell.h"

namespace lyric_runtime {

    class StackfulCoroutine {
    public:
        StackfulCoroutine();

        bool nextOp(lyric_object::OpCell &op);
        bool moveIP(int16_t offset);
        BytecodeSegment *peekSP() const;
        lyric_object::BytecodeIterator peekIP() const;

        void transferControl(const lyric_object::BytecodeIterator &ip, BytecodeSegment *sp = nullptr);

        CallCell& currentCallOrThrow();
        const CallCell& currentCallOrThrow() const;

        tempo_utils::Status pushCall(
            const CallCell &value,
            const lyric_object::BytecodeIterator &ip,
            BytecodeSegment *sp);
        tempo_utils::Status popCall(CallCell &value);
        tempo_utils::Status peekCall(const CallCell **valueptr, int offset = -1) const;
        tempo_utils::Status peekCall(CallCell **valueptr, int offset = -1);
        tempo_utils::Status dropCall(int offset = -1);

        bool callStackEmpty() const;
        int callStackSize() const;
        std::vector<CallCell>::const_reverse_iterator callsBegin() const;
        std::vector<CallCell>::const_reverse_iterator callsEnd() const;

        tempo_utils::Status pushData(const DataCell &value);
        tempo_utils::Status popData(DataCell &value);
        tempo_utils::Status popData(int count, std::vector<DataCell> &values);
        tempo_utils::Status peekData(const DataCell **valueptr, int offset = -1) const;
        tempo_utils::Status peekData(DataCell **valueptr, int offset = -1);
        tempo_utils::Status dropData(int offset = -1);
        tempo_utils::Status extendDataStack(int count);
        tempo_utils::Status resizeDataStack(int count);

        bool dataStackEmpty() const;
        int dataStackSize() const;
        std::vector<DataCell>::const_reverse_iterator dataBegin() const;
        std::vector<DataCell>::const_reverse_iterator dataEnd() const;

        void pushGuard(int stackGuard = -1);
        int popGuard();
        int peekGuard() const;
        bool checkGuard() const;
        int guardStackSize() const;

        void reset();

    private:
        lyric_object::BytecodeIterator m_IP;        // instruction pointer
        BytecodeSegment *m_SP;                      // current segment pointer
        std::vector<CallCell> m_callStack;          // call frame stack
        std::vector<DataCell> m_dataStack;          // data cell stack
        std::vector<int> m_guardStack;              // subinterpreter guard stack
    };
}

#endif // LYRIC_RUNTIME_STACKFUL_COROUTINE_H