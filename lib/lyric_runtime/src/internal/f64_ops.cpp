
#include <boost/safe_numerics/checked_default.hpp>

#include <lyric_runtime/internal/f64_ops.h>
#include <lyric_runtime/interpreter_result.h>

inline bool to_f64(const lyric_runtime::DataCell &cell, double &f64) {
    switch (cell.type) {
        case lyric_runtime::DataCellType::Float32:
            f64 = cell.data.f32;
            return true;
        case lyric_runtime::DataCellType::Float64:
            f64 = cell.data.f64;
            return true;
        default:
            return false;
    }
}

tempo_utils::Status
lyric_runtime::internal::f64_add(const DataCell &lhs, const DataCell &rhs, DataCell &result)
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
    result = DataCell(static_cast<double>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::f64_sub(const DataCell &lhs, const DataCell &rhs, DataCell &result)
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
    result = DataCell(static_cast<double>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::f64_mul(const DataCell &lhs, const DataCell &rhs, DataCell &result)
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
    result = DataCell(static_cast<double>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::f64_div(const DataCell &lhs, const DataCell &rhs, DataCell &result)
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
    result = DataCell(static_cast<double>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::f64_cmp(const DataCell &lhs, const DataCell &rhs, DataCell &result)
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
    result = DataCell(cmp);
    return {};
}

tempo_utils::Status
lyric_runtime::internal::f64_neg(const DataCell &element, DataCell &result)
{
    double e;
    if (!to_f64(element, e))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert element to f64");

    result = DataCell(-e);
    return {};
}
