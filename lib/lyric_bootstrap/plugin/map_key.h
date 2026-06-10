#ifndef ZURI_CORE_MAP_KEY_H
#define ZURI_CORE_MAP_KEY_H

#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/bytes_ref.h>
#include <lyric_runtime/namespace_ref.h>
#include <lyric_runtime/protocol_ref.h>
#include <lyric_runtime/rest_ref.h>
#include <lyric_runtime/status_ref.h>
#include <lyric_runtime/string_ref.h>

struct MapKey {
    const lyric_runtime::Operand &cell;
};

template <typename H>
H AbslHashValue(H h, const MapKey &key)
{
    const auto &operand = key.cell;
    auto optype = operand.getType();

    switch (optype) {
        case lyric_runtime::OperandType::Invalid:
        case lyric_runtime::OperandType::Nil:
        case lyric_runtime::OperandType::Undef:
            return H::combine(std::move(h), optype);

        case lyric_runtime::OperandType::Bool: {
            bool b;
            TU_ASSERT (operand.getBool(b));
            return H::combine(std::move(h), b, optype);
        }
        case lyric_runtime::OperandType::Int8: {
            tu_int8 i8;
            TU_ASSERT (operand.getI8(i8));
            return H::combine(std::move(h), i8, optype);
        }
        case lyric_runtime::OperandType::Int16: {
            tu_int16 i16;
            TU_ASSERT (operand.getI16(i16));
            return H::combine(std::move(h), i16, optype);
        }
        case lyric_runtime::OperandType::Int32: {
            tu_int32 i32;
            TU_ASSERT (operand.getI32(i32));
            return H::combine(std::move(h), i32, optype);
        }
        case lyric_runtime::OperandType::Int64: {
            tu_int64 i64;
            TU_ASSERT (operand.getI64(i64));
            return H::combine(std::move(h), i64, optype);
        }
        case lyric_runtime::OperandType::UInt8: {
            tu_uint8 u8;
            TU_ASSERT (operand.getU8(u8));
            return H::combine(std::move(h), u8, optype);
        }
        case lyric_runtime::OperandType::UInt16: {
            tu_uint16 u16;
            TU_ASSERT (operand.getU16(u16));
            return H::combine(std::move(h), u16, optype);
        }
        case lyric_runtime::OperandType::UInt32: {
            tu_uint32 u32;
            TU_ASSERT (operand.getU32(u32));
            return H::combine(std::move(h), u32, optype);
        }
        case lyric_runtime::OperandType::UInt64: {
            tu_uint64 u64;
            TU_ASSERT (operand.getU64(u64));
            return H::combine(std::move(h), u64, optype);
        }
        case lyric_runtime::OperandType::Float32: {
            float f32;
            TU_ASSERT (operand.getF32(f32));
            return H::combine(std::move(h), f32, optype);
        }
        case lyric_runtime::OperandType::Float64: {
            double f64;
            TU_ASSERT (operand.getF64(f64));
            return H::combine(std::move(h), f64, optype);
        }
        case lyric_runtime::OperandType::Char32: {
            char32_t chr;
            TU_ASSERT (operand.getC32(chr));
            return H::combine(std::move(h), chr, optype);
        }


        case lyric_runtime::OperandType::Descriptor: {
            lyric_runtime::DescriptorEntry *descriptor;
            TU_ASSERT (operand.getDescriptor(descriptor));
            return H::combine(std::move(h),
                descriptor->getSegmentIndex(),
                descriptor->getDescriptorIndex());
        }
        case lyric_runtime::OperandType::Ref: {
            lyric_runtime::BaseRef *ref;
            TU_ASSERT (operand.getRef(ref));
            ref->hashValue(absl::HashState::Create(&h));
            return std::move(h);
        }
        case lyric_runtime::OperandType::Bytes: {
            lyric_runtime::BytesRef *bytes;
            TU_ASSERT (operand.getBytes(bytes));
            bytes->hashValue(absl::HashState::Create(&h));
            return std::move(h);
        }
        case lyric_runtime::OperandType::Namespace: {
            lyric_runtime::NamespaceRef *ns;
            TU_ASSERT (operand.getNamespace(ns));
            ns->hashValue(absl::HashState::Create(&h));
            return std::move(h);
        }
        case lyric_runtime::OperandType::Protocol: {
            lyric_runtime::ProtocolRef *protocol;
            TU_ASSERT (operand.getProtocol(protocol));
            protocol->hashValue(absl::HashState::Create(&h));
            return std::move(h);
        }
        case lyric_runtime::OperandType::Rest: {
            lyric_runtime::RestRef *rest;
            TU_ASSERT (operand.getRest(rest));
            rest->hashValue(absl::HashState::Create(&h));
            return std::move(h);
        }
        case lyric_runtime::OperandType::String: {
            lyric_runtime::StringRef *str;
            TU_ASSERT (operand.getString(str));
            str->hashValue(absl::HashState::Create(&h));
            return std::move(h);
        }
        case lyric_runtime::OperandType::Status: {
            lyric_runtime::StatusRef *status;
            TU_ASSERT (operand.getStatus(status));
            status->hashValue(absl::HashState::Create(&h));
            return std::move(h);
        }
        case lyric_runtime::OperandType::Type: {
            lyric_runtime::TypeEntry *type;
            TU_ASSERT (operand.getType(type));
            return H::combine(std::move(h),
                type->getSegmentIndex(),
                type->getDescriptorIndex());
        }

        default:
            TU_UNREACHABLE();
    }
}

#endif // ZURI_CORE_MAP_KEY_H