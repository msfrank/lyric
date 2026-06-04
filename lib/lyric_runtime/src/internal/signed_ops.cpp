
#include <boost/safe_numerics/checked_default.hpp>

#include <lyric_runtime/internal/signed_ops.h>
#include <lyric_runtime/interpreter_result.h>

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

tempo_utils::Status
lyric_runtime::internal::i32_add(const DataCell &lhs, const DataCell &rhs, DataCell &result)
{
    tu_int32 l, r;
    if (!to_i32(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to i32");
    if (!to_i32(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to i32");

    auto check = boost::safe_numerics::checked::add(l, r);
    if (check.exception()) [[unlikely]] {
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            static_cast<const char *>(check));
    }
    result = DataCell(static_cast<tu_int32>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::i32_sub(const DataCell &lhs, const DataCell &rhs, DataCell &result)
{
    tu_int32 l, r;
    if (!to_i32(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to i32");
    if (!to_i32(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to i32");

    auto check = boost::safe_numerics::checked::subtract(l, r);
    if (check.exception()) [[unlikely]] {
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            static_cast<const char *>(check));
    }
    result = DataCell(static_cast<tu_int32>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::i32_mul(const DataCell &lhs, const DataCell &rhs, DataCell &result)
{
    tu_int32 l, r;
    if (!to_i32(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to i32");
    if (!to_i32(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to i32");

    auto check = boost::safe_numerics::checked::multiply(l, r);
    if (check.exception()) [[unlikely]] {
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            static_cast<const char *>(check));
    }
    result = DataCell(static_cast<tu_int32>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::i32_div(const DataCell &lhs, const DataCell &rhs, DataCell &result)
{
    tu_int32 l, r;
    if (!to_i32(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to i32");
    if (!to_i32(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to i32");

    auto check = boost::safe_numerics::checked::divide(l, r);
    if (check.exception()) [[unlikely]] {
        return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
            static_cast<const char *>(check));
    }
    result = DataCell(static_cast<tu_int32>(check));
    return {};
}

tempo_utils::Status
lyric_runtime::internal::i32_cmp(const DataCell &lhs, const DataCell &rhs, DataCell &result)
{
    tu_int32 l, r;
    if (!to_i32(lhs, l))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert lhs to i32");
    if (!to_i32(rhs, r))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV2,
            "cannot convert rhs to i32");

    tu_int32 cmp;
    if (l <= r) {
        cmp = l == r? 0 : -1;
    } else {
        cmp = 1;
    }
    result = DataCell(cmp);
    return {};
}

tempo_utils::Status
lyric_runtime::internal::i32_neg(const DataCell &element, DataCell &result)
{
    tu_int32 e;
    if (!to_i32(element, e))
        return InterpreterStatus::forCondition(InterpreterCondition::kInvalidDataStackV1,
            "cannot convert element to i32");

    result = DataCell(-e);
    return {};
}
