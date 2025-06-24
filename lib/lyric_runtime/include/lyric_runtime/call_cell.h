#ifndef LYRIC_RUNTIME_CALL_CELL_H
#define LYRIC_RUNTIME_CALL_CELL_H

#include <tempo_utils/log_message.h>

#include <lyric_object/bytecode_iterator.h>

#include "data_cell.h"
#include "runtime_types.h"
#include "virtual_table.h"

namespace lyric_runtime {

    class CallCell {

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
            const std::vector<DataCell> &data,
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
            const std::vector<DataCell> &data,
            DataCell receiver);
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
            const std::vector<DataCell> &data);
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
        DataCell getReceiver() const;
        const VirtualTable *getVirtualTable() const;

        DataCell getArgument(int index) const;
        void setArgument(int index, const DataCell &cell);
        tu_uint16 numArguments() const;

        DataCell getLocal(int index) const;
        void setLocal(int index, const DataCell &cell);
        tu_uint16 numLocals() const;

        DataCell getLexical(int index) const;
        void setLexical(int index, const DataCell &cell);
        tu_uint16 numLexicals() const;

        DataCell getRest(int index) const;
        void setRest(int index, const DataCell &cell);
        tu_uint16 numRest() const;

        std::string toString() const;

    private:
        tu_uint32 m_callIndex;
        tu_uint32 m_callSegment;
        tu_uint32 m_procOffset;
        tu_uint32 m_returnSegment;
        lyric_object::BytecodeIterator m_returnIP;
        bool m_returnsValue;
        int m_stackGuard;
        tu_uint16 m_numArguments;
        tu_uint16 m_numRest;
        tu_uint16 m_numLocals;
        tu_uint16 m_numLexicals;
        std::vector<DataCell> m_data;
        DataCell m_receiver;
        const VirtualTable *m_vtable;
    };

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const CallCell &cell);
}

#endif // LYRIC_RUNTIME_CALL_CELL_H