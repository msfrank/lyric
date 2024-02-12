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

        void transferControl(const lyric_object::BytecodeIterator &ip, BytecodeSegment *sp);

        void pushCall(const CallCell &value, const lyric_object::BytecodeIterator &ip, BytecodeSegment *sp);
        CallCell popCall();
        CallCell& peekCall(int offset = -1);
        void dropCall(int offset = -1);
        int callStackSize() const;
        std::vector<CallCell>::const_reverse_iterator callsBegin() const;
        std::vector<CallCell>::const_reverse_iterator callsEnd() const;

        void pushData(const DataCell &value);
        DataCell popData();
        std::vector<DataCell> popData(int count);
        DataCell& peekData(int offset = -1);
        void dropData(int offset = -1);
        int dataStackSize() const;
        void extendDataStack(int count);
        void resizeDataStack(int count);
        std::vector<DataCell>::const_reverse_iterator dataBegin() const;
        std::vector<DataCell>::const_reverse_iterator dataEnd() const;

        void pushGuard(int stackGuard = -1);
        int popGuard();
        int peekGuard();
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