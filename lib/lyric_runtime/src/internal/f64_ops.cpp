
#include <boost/safe_numerics/checked_default.hpp>

#include <lyric_runtime/internal/f64_ops.h>
#include <lyric_runtime/interpreter_result.h>

inline bool to_f64(const lyric_runtime::Operand &cell, double &f64) {
    switch (cell.getType()) {
        case lyric_runtime::OperandType::Float32: {
            float f32;
            if (!cell.getF32(f32))
                return false;
            f64 = f32;
            return true;
        }
        case lyric_runtime::OperandType::Float64:
            return cell.getF64(f64);
        default:
            return false;
    }
}

tempo_utils::Status
lyric_runtime::internal::f64_add(const Operand &lhs, const Operand &rhs, Operand &result)
{
    double l, r;
    if (!to_f64(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to f64");
    if (!to_f64(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to f64");

    auto check = boost::safe_numerics::checked::add(l, r);
    if (check.exception()) [[unlikely]] {
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            static_cast<const char *>(check));
    }
    result = Operand::fromF64(static_cast<double>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::f64_sub(const Operand &lhs, const Operand &rhs, Operand &result)
{
    double l, r;
    if (!to_f64(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to f64");
    if (!to_f64(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to f64");

    auto check = boost::safe_numerics::checked::subtract(l, r);
    if (check.exception()) [[unlikely]] {
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            static_cast<const char *>(check));
    }
    result = Operand::fromF64(static_cast<double>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::f64_mul(const Operand &lhs, const Operand &rhs, Operand &result)
{
    double l, r;
    if (!to_f64(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to f64");
    if (!to_f64(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to f64");

    auto check = boost::safe_numerics::checked::multiply(l, r);
    if (check.exception()) [[unlikely]] {
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            static_cast<const char *>(check));
    }
    result = Operand::fromF64(static_cast<double>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::f64_div(const Operand &lhs, const Operand &rhs, Operand &result)
{
    double l, r;
    if (!to_f64(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to f64");
    if (!to_f64(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to f64");

    auto check = boost::safe_numerics::checked::divide(l, r);
    if (check.exception()) [[unlikely]] {
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            static_cast<const char *>(check));
    }
    result = Operand::fromF64(static_cast<double>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::f64_cmp(const Operand &lhs, const Operand &rhs, Operand &result)
{
    double l, r;
    if (!to_f64(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to f64");
    if (!to_f64(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to f64");

    tu_int64 cmp;
    if (l <= r) {
        cmp = l == r? 0 : -1;
    } else {
        cmp = 1;
    }
    result = Operand::fromF64(cmp);
    return {};
}

tempo_utils::Status
lyric_runtime::internal::f64_neg(const Operand &element, Operand &result)
{
    double e;
    if (!to_f64(element, e))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert element to f64");

    result = Operand::fromF64(-e);
    return {};
}
