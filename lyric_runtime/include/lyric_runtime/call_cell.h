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
        CallCell(
            uint32_t callIndex,
            uint32_t callSegment,
            uint32_t procOffset,
            uint32_t returnSegment,
            lyric_object::BytecodeIterator returnIP,
            int stackGuard,
            uint16_t numArguments,
            uint16_t numRest,
            uint16_t numLocals,
            uint16_t numLexicals,
            const std::vector<DataCell> &data,
            const VirtualTable *vtable);
        CallCell(
            uint32_t callIndex,
            uint32_t callSegment,
            uint32_t procOffset,
            uint32_t returnSegment,
            lyric_object::BytecodeIterator returnIP,
            int stackGuard,
            uint16_t numArguments,
            uint16_t numRest,
            uint16_t numLocals,
            uint16_t numLexicals,
            const std::vector<DataCell> &data,
            DataCell receiver);
        CallCell(
            uint32_t callIndex,
            uint32_t callSegment,
            uint32_t procOffset,
            uint32_t returnSegment,
            lyric_object::BytecodeIterator returnIP,
            int stackGuard,
            uint16_t numArguments,
            uint16_t numRest,
            uint16_t numLocals,
            uint16_t numLexicals,
            const std::vector<DataCell> &data);
        CallCell(const CallCell &other);
        CallCell(CallCell &&other) noexcept;

        CallCell& operator=(const CallCell &other);
        CallCell& operator=(CallCell &&other) noexcept;

        uint32_t getCallIndex() const;
        uint32_t getCallSegment() const;
        uint32_t getProcOffset() const;
        uint32_t getReturnSegment() const;
        lyric_object::BytecodeIterator getReturnIP() const;
        int getStackGuard() const;
        DataCell getReceiver() const;
        const VirtualTable *getVirtualTable() const;

        DataCell getArgument(int index) const;
        void setArgument(int index, const DataCell &cell);
        uint16_t numArguments() const;

        DataCell getLocal(int index) const;
        void setLocal(int index, const DataCell &cell);
        uint16_t numLocals() const;

        DataCell getLexical(int index) const;
        void setLexical(int index, const DataCell &cell);
        uint16_t numLexicals() const;

        DataCell getRest(int index) const;
        void setRest(int index, const DataCell &cell);
        uint16_t numRest() const;

        std::string toString() const;

    private:
        uint32_t m_callIndex;
        uint32_t m_callSegment;
        uint32_t m_procOffset;
        uint32_t m_returnSegment;
        lyric_object::BytecodeIterator m_returnIP;
        int m_stackGuard;
        uint16_t m_numArguments;
        uint16_t m_numRest;
        uint16_t m_numLocals;
        uint16_t m_numLexicals;
        std::vector<DataCell> m_data;
        DataCell m_receiver;
        const VirtualTable *m_vtable;
    };

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const CallCell &cell);
}

#endif // LYRIC_RUNTIME_CALL_CELL_H