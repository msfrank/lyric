#ifndef LYRIC_RUNTIME_INTERPRETER_RESULT_H
#define LYRIC_RUNTIME_INTERPRETER_RESULT_H

#include <lyric_common/symbol_url.h>
#include <tempo_utils/log_stream.h>
#include <tempo_utils/result.h>
#include <tempo_utils/status.h>

#include "data_cell.h"

namespace lyric_runtime {

    constexpr tempo_utils::SchemaNs kLyricRuntimeInterpreterStatusNs("dev.zuri.ns:lyric-runtime-interpreter-status-1");

    enum class InterpreterCondition {
        kInvalidOperandAddressV1,
        kInvalidOperandFlagsAddressV1,
        kInvalidOperandFlagsAddressV2,
        kInvalidOperandJumpV1,
        kInvalidOperandOffsetV1,
        kInvalidOperandFlagsAddressPlacementV1,
        kInvalidOperandFlagsAddressPlacementV2,
        kInvalidOperandFlagsAddressPlacementV3,
        kInvalidOperandFlagsOffsetPlacementV1,
        kInvalidOperandFlagsOffsetPlacementV2,
        kInvalidOperandFlagsOffsetPlacementV3,
        kInvalidOperandTypeV1,
        kInvalidOperandImmediateI64V1,
        kInvalidOperandImmediateDblV1,
        kInvalidOperandImmediateChrV1,
        kInvalidDataStackV1,
        kInvalidDataStackV2,
        kInvalidDataStackValue,
        kInvalidReceiver,
        kRuntimeInvariant,
        kMissingAssembly,
        kMissingSymbol,
        kExceededMaximumRecursion,
        kInterrupted,
        kAborted,
    };

    class InterpreterStatus : public tempo_utils::TypedStatus<InterpreterCondition> {
    public:
        using TypedStatus::TypedStatus;
        static InterpreterStatus ok();
        static bool convert(InterpreterStatus &dstStatus, const tempo_utils::Status &srcStatus);
    private:
        InterpreterStatus(tempo_utils::StatusCode statusCode, std::shared_ptr<const tempo_utils::Detail> detail);

    public:
        /**
         *
         * @tparam Args
         * @param condition
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename... Args>
        static InterpreterStatus forCondition(
            InterpreterCondition condition,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return InterpreterStatus(condition, message);
        }
        /**
         *
         * @tparam Args
         * @param condition
         * @param messageFmt
         * @param messageArgs
         * @return
         */
        template <typename... Args>
        static InterpreterStatus forCondition(
            InterpreterCondition condition,
            tempo_utils::TraceId traceId,
            tempo_utils::SpanId spanId,
            fmt::string_view messageFmt = {},
            Args... messageArgs)
        {
            auto message = fmt::vformat(messageFmt, fmt::make_format_args(messageArgs...));
            return InterpreterStatus(condition, message, traceId, spanId);
        }
    };
}

namespace tempo_utils {

    template<>
    struct StatusTraits<lyric_runtime::InterpreterStatus> {
        using ConditionType = lyric_runtime::InterpreterCondition;
        static bool convert(lyric_runtime::InterpreterStatus &dstStatus, const tempo_utils::Status &srcStatus)
        {
            return lyric_runtime::InterpreterStatus::convert(dstStatus, srcStatus);
        }
    };

    template<>
    struct ConditionTraits<lyric_runtime::InterpreterCondition> {
        using StatusType = lyric_runtime::InterpreterStatus;
        static constexpr const char *condition_namespace() { return lyric_runtime::kLyricRuntimeInterpreterStatusNs.getNs(); }
        static constexpr StatusCode make_status_code(lyric_runtime::InterpreterCondition condition)
        {
            switch (condition) {
                case lyric_runtime::InterpreterCondition::kInvalidOperandAddressV1:
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsAddressV1:
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsAddressV2:
                case lyric_runtime::InterpreterCondition::kInvalidOperandJumpV1:
                case lyric_runtime::InterpreterCondition::kInvalidOperandOffsetV1:
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsAddressPlacementV1:
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsAddressPlacementV2:
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsAddressPlacementV3:
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsOffsetPlacementV1:
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsOffsetPlacementV2:
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsOffsetPlacementV3:
                case lyric_runtime::InterpreterCondition::kInvalidOperandTypeV1:
                case lyric_runtime::InterpreterCondition::kInvalidOperandImmediateI64V1:
                case lyric_runtime::InterpreterCondition::kInvalidOperandImmediateDblV1:
                case lyric_runtime::InterpreterCondition::kInvalidOperandImmediateChrV1:
                case lyric_runtime::InterpreterCondition::kInvalidDataStackV1:
                case lyric_runtime::InterpreterCondition::kInvalidDataStackV2:
                case lyric_runtime::InterpreterCondition::kInvalidDataStackValue:
                case lyric_runtime::InterpreterCondition::kInvalidReceiver:
                case lyric_runtime::InterpreterCondition::kAborted:
                    return tempo_utils::StatusCode::kAborted;
                case lyric_runtime::InterpreterCondition::kRuntimeInvariant:
                    return tempo_utils::StatusCode::kInternal;
                case lyric_runtime::InterpreterCondition::kMissingAssembly:
                case lyric_runtime::InterpreterCondition::kMissingSymbol:
                    return tempo_utils::StatusCode::kFailedPrecondition;
                case lyric_runtime::InterpreterCondition::kInterrupted:
                    return tempo_utils::StatusCode::kCancelled;
                default:
                    return tempo_utils::StatusCode::kUnknown;
            }
        };
        static constexpr const char *make_error_message(lyric_runtime::InterpreterCondition condition)
        {
            switch (condition) {
                case lyric_runtime::InterpreterCondition::kInvalidOperandAddressV1:
                    return "Invalid operand (address_u32.address)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsAddressV1:
                    return "Invalid operand (flags_u8_address_u32.flags)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsAddressV2:
                    return "Invalid operand (flags_u8_address_u32.address)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandJumpV1:
                    return "Invalid operand (jump_i16.jump)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandOffsetV1:
                    return "Invalid operand (offset_u16.offset)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsAddressPlacementV1:
                    return "Invalid operand (flags_u8_address_u32_placement_u16.flags)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsAddressPlacementV2:
                    return "Invalid operand (flags_u8_address_u32_placement_u16.address)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsAddressPlacementV3:
                    return "Invalid operand (flags_u8_address_u32_placement_u16.placement)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsOffsetPlacementV1:
                    return "Invalid operand (flags_u8_offset_u16_placement_u16.flags)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsOffsetPlacementV2:
                    return "Invalid operand (flags_u8_offset_u16_placement_u16.offset)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandFlagsOffsetPlacementV3:
                    return "Invalid operand (flags_u8_offset_u16_placement_u16.placement)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandTypeV1:
                    return "Invalid operand (type_u8.type)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandImmediateI64V1:
                    return "Invalid operand (immediate_i64.i64)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandImmediateDblV1:
                    return "Invalid operand (immediate_dbl.dbl)";
                case lyric_runtime::InterpreterCondition::kInvalidOperandImmediateChrV1:
                    return "Invalid operand (immediate_chr.chr)";
                case lyric_runtime::InterpreterCondition::kInvalidDataStackV1:
                    return "Invalid data stack value 1";
                case lyric_runtime::InterpreterCondition::kInvalidDataStackV2:
                    return "Invalid data stack value 2";
                case lyric_runtime::InterpreterCondition::kInvalidDataStackValue:
                    return "Invalid data stack value";
                case lyric_runtime::InterpreterCondition::kInvalidReceiver:
                    return "Invalid receiver";
                case lyric_runtime::InterpreterCondition::kRuntimeInvariant:
                    return "Runtime invariant";
                case lyric_runtime::InterpreterCondition::kMissingAssembly:
                    return "Missing assembly";
                case lyric_runtime::InterpreterCondition::kMissingSymbol:
                    return "Missing symbol";
                case lyric_runtime::InterpreterCondition::kInterrupted:
                    return "Interrupted";
                case lyric_runtime::InterpreterCondition::kAborted:
                    return "Aborted";
                default:
                    return "INVALID";
            }
        }
    };
}

#endif // LYRIC_RUNTIME_INTERPRETER_RESULT_H
