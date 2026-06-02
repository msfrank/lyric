
#include <boost/safe_numerics/checked_default.hpp>

#include <lyric_runtime/internal/i64_ops.h>
#include <lyric_runtime/interpreter_result.h>

inline bool to_i64(const lyric_runtime::DataCell &cell, tu_int64 &i64) {
    switch (cell.type) {
        case lyric_runtime::DataCellType::Int8:
            i64 = static_cast<unsigned char>(cell.data.i8);
            return true;
        case lyric_runtime::DataCellType::Int16:
            i64 = cell.data.i16;
            return true;
        case lyric_runtime::DataCellType::Int32:
            i64 = cell.data.i32;
            return true;
        case lyric_runtime::DataCellType::Int64:
            i64 = cell.data.i64;
            return true;
        default:
            return false;
    }
}

tempo_utils::Status
lyric_runtime::internal::i64_add(const DataCell &lhs, const DataCell &rhs, DataCell &result)
{
    tu_int64 l, r;
    if (!to_i64(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to i64");
    if (!to_i64(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to i64");

    auto check = boost::safe_numerics::checked::add(l, r);
    if (check.exception()) [[unlikely]] {
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            static_cast<const char *>(check));
    }
    result = DataCell(static_cast<tu_int64>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::i64_sub(const DataCell &lhs, const DataCell &rhs, DataCell &result)
{
    tu_int64 l, r;
    if (!to_i64(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to i64");
    if (!to_i64(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to i64");

    auto check = boost::safe_numerics::checked::subtract(l, r);
    if (check.exception()) [[unlikely]] {
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            static_cast<const char *>(check));
    }
    result = DataCell(static_cast<tu_int64>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::i64_mul(const DataCell &lhs, const DataCell &rhs, DataCell &result)
{
    tu_int64 l, r;
    if (!to_i64(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to i64");
    if (!to_i64(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to i64");

    auto check = boost::safe_numerics::checked::multiply(l, r);
    if (check.exception()) [[unlikely]] {
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            static_cast<const char *>(check));
    }
    result = DataCell(static_cast<tu_int64>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::i64_div(const DataCell &lhs, const DataCell &rhs, DataCell &result)
{
    tu_int64 l, r;
    if (!to_i64(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to i64");
    if (!to_i64(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to i64");

    auto check = boost::safe_numerics::checked::divide(l, r);
    if (check.exception()) [[unlikely]] {
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            static_cast<const char *>(check));
    }
    result = DataCell(static_cast<tu_int64>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::i64_cmp(const DataCell &lhs, const DataCell &rhs, DataCell &result)
{
    tu_int64 l, r;
    if (!to_i64(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to i64");
    if (!to_i64(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to i64");

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
lyric_runtime::internal::i64_neg(const DataCell &element, DataCell &result)
{
    tu_int64 e;
    if (!to_i64(element, e))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert element to i64");

    result = DataCell(-e);
    return {};
}
