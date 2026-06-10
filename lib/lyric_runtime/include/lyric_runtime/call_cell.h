#ifndef LYRIC_RUNTIME_CALL_CELL_H
#define LYRIC_RUNTIME_CALL_CELL_H

#include <tempo_utils/log_message.h>

#include <lyric_object/bytecode_iterator.h>

#include "runtime_types.h"
#include "virtual_table.h"

namespace lyric_runtime {

    class CallCell final {

    public:
        CallCell();
        CallCell(
            tu_uint32 callIndex,
            tu_uint32 callSegment,
            tu_uint32 procOffset,
            tu_uint32 returnSegment,
            lyric_object::BytecodeIterator returnIP,
            bool returnsValue,
            int stackGuard,
            tu_uint16 numArguments,
            tu_uint16 numRest,
            tu_uint16 numLocals,
            tu_uint16 numLexicals,
            const std::vector<Operand> &data,
            const VirtualTable *vtable);
        CallCell(
            tu_uint32 callIndex,
            tu_uint32 callSegment,
            tu_uint32 procOffset,
            tu_uint32 returnSegment,
            lyric_object::BytecodeIterator returnIP,
            bool returnsValue,
            int stackGuard,
            tu_uint16 numArguments,
            tu_uint16 numRest,
            tu_uint16 numLocals,
            tu_uint16 numLexicals,
            const std::vector<Operand> &data,
            Operand receiver);
        CallCell(
            tu_uint32 callIndex,
            tu_uint32 callSegment,
            tu_uint32 procOffset,
            tu_uint32 returnSegment,
            lyric_object::BytecodeIterator returnIP,
            bool returnsValue,
            int stackGuard,
            tu_uint16 numArguments,
            tu_uint16 numRest,
            tu_uint16 numLocals,
            tu_uint16 numLexicals,
            const std::vector<Operand> &data);
        CallCell(const CallCell &other);
        CallCell(CallCell &&other) noexcept;

        CallCell& operator=(const CallCell &other);
        CallCell& operator=(CallCell &&other) noexcept;

        bool isValid() const;

        tu_uint32 getCallIndex() const;
        tu_uint32 getCallSegment() const;
        tu_uint32 getProcOffset() const;
        tu_uint32 getReturnSegment() const;
        lyric_object::BytecodeIterator getReturnIP() const;
        bool returnsValue() const;
        int getStackGuard() const;
        Operand getReceiver() const;
        const VirtualTable *getVirtualTable() const;

        Operand getArgument(int index) const;
        void setArgument(int index, const Operand &cell);
        tu_uint16 numArguments() const;

        Operand getLocal(int index) const;
        void setLocal(int index, const Operand &cell);
        tu_uint16 numLocals() const;

        Operand getLexical(int index) const;
        void setLexical(int index, const Operand &cell);
        tu_uint16 numLexicals() const;

        Operand getRest(int index) const;
        void setRest(int index, const Operand &cell);
        tu_uint16 numRest() const;

        bool hasSavedIP() const;
        lyric_object::BytecodeIterator getSavedIP() const;
        void setSavedIP(const lyric_object::BytecodeIterator &ip);
        bool clearSavedIP();

        std::string toString() const;

    private:
        tu_uint32 m_callIndex;
        tu_uint32 m_callSegment;
        tu_uint32 m_procOffset;
        tu_uint32 m_returnSegment;
        lyric_object::BytecodeIterator m_returnIP;
        lyric_object::BytecodeIterator m_savedIP;
        bool m_returnsValue;
        int m_stackGuard;
        tu_uint16 m_numArguments;
        tu_uint16 m_numRest;
        tu_uint16 m_numLocals;
        tu_uint16 m_numLexicals;
        std::vector<Operand> m_data;
        Operand m_receiver;
        const VirtualTable *m_vtable;
    };

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const CallCell &cell);
}

#endif // LYRIC_RUNTIME_CALL_CELL_H