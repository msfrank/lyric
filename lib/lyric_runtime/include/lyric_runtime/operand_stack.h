#ifndef LYRIC_RUNTIME_OPERAND_STACK_H
#define LYRIC_RUNTIME_OPERAND_STACK_H

#include <vector>

#include "operand.h"

namespace lyric_runtime {

    class OperandStack {
    public:
        explicit OperandStack(size_t stackSize = 8192);

        tempo_utils::Status pushOperand(const Operand &value);
        tempo_utils::Status popOperand(Operand &value);
        tempo_utils::Status popOperands(int count, std::vector<Operand> &values);
        tempo_utils::Status peekOperand(Operand &value, tu_int16 offset = 0) const;
        tempo_utils::Status dropOperand(tu_int16 offset = 0);

        size_t getDepth() const;
        size_t getBytesAvailable() const;
        size_t getBytesUsed() const;
        float getUtilization() const;

    private:
        std::vector<tu_uint8> m_stack;
        size_t m_last = 0;
        size_t m_depth = 0;
    };
}

#endif // LYRIC_RUNTIME_OPERAND_STACK_H
