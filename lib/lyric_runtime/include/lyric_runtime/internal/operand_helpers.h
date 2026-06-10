#ifndef LYRIC_RUNTIME_INTERNAL_OP_HELPERS_H
#define LYRIC_RUNTIME_INTERNAL_OP_HELPERS_H

#include "../interpreter_result.h"
#include "../operand.h"

namespace lyric_runtime::internal {

    template<class ValueType>
    tempo_utils::Status get_unary_operand(
        const Operand &element,
        ValueType &v)
    {
        if (!operand_to_value(element, v))
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid element operand");
        return {};
    }

    template<class ValueType>
    tempo_utils::Status get_binary_operands(
        const Operand &lhs,
        const Operand &rhs,
        ValueType &l,
        ValueType &r)
    {
        if (!operand_to_value(lhs, l))
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
                "invalid lhs operand");
        if (!operand_to_value(rhs, r))
            return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
                "invalid rhs operand");
        return {};
    }

}

#endif // LYRIC_RUNTIME_INTERNAL_OP_HELPERS_H
