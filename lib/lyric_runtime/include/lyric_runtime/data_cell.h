#ifndef LYRIC_RUNTIME_DATA_CELL_H
#define LYRIC_RUNTIME_DATA_CELL_H

#include <tempo_utils/log_message.h>

#include "runtime_types.h"

namespace lyric_runtime {

    // forward declarations
    class BaseRef;
    class BytesRef;
    class DescriptorEntry;
    class NamespaceRef;
    class ProtocolRef;
    class RestRef;
    class StatusRef;
    class StringRef;
    class TypeEntry;

    enum class DataCellType : uint8_t {
        Invalid,
        Nil,
        Undef,
        Bool,
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Float32,
        Float64,
        Char32,
        Descriptor,
        Type,
        Ref,
        Bytes,
        Namespace,
        Protocol,
        Status,
        String,
        Rest,
    };

    struct DataCell final {

        DataCell();
        explicit DataCell(bool b);
        explicit DataCell(tu_int8 i8);
        explicit DataCell(tu_int16 i16);
        explicit DataCell(tu_int32 i32);
        explicit DataCell(tu_int64 i64);
        explicit DataCell(tu_uint8 u8);
        explicit DataCell(tu_uint16 u16);
        explicit DataCell(tu_uint32 u32);
        explicit DataCell(tu_uint64 u64);
        explicit DataCell(float f32);
        explicit DataCell(double f64);
        explicit DataCell(char32_t char32);
        DataCell(const DataCell &other);
        DataCell(DataCell &&other) noexcept;
        ~DataCell() = default;

        DataCell& operator=(const DataCell &other);
        DataCell& operator=(DataCell &&other) noexcept;

        bool isValid() const;
        bool isIntegral() const;
        bool isRational() const;
        bool isIntrinsic() const;
        bool isDescriptor() const;
        bool isReference() const;

        lyric_object::LinkageSection getLinkage() const;

        std::string toString() const;

        static DataCell nil();
        static DataCell undef();

        static DataCell forBytes(BytesRef *bytes);
        static DataCell forDescriptor(DescriptorEntry *descriptor);
        static DataCell forNamespace(NamespaceRef *ns);
        static DataCell forProtocol(ProtocolRef *protocol);
        static DataCell forRef(BaseRef *ref);
        static DataCell forRest(RestRef *rest);
        static DataCell forStatus(StatusRef *status);
        static DataCell forString(StringRef *str);
        static DataCell forType(TypeEntry *type);

        DataCellType type;
        union {
            bool b;
            tu_int8 i8;
            tu_int16 i16;
            tu_int32 i32;
            tu_int64 i64;
            tu_uint8 u8;
            tu_uint16 u16;
            tu_uint32 u32;
            tu_uint64 u64;
            float f32;
            double f64;
            char32_t chr;
            DescriptorEntry *descriptor;
            TypeEntry *type;
            BaseRef *ref;
            BytesRef *bytes;
            NamespaceRef *ns;
            ProtocolRef *protocol;
            StatusRef *status;
            StringRef *str;
            RestRef *rest;
        } data;
    };

    bool operator==(const DataCell &lhs, const DataCell &rhs);
    bool operator!=(const DataCell &lhs, const DataCell &rhs);

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const DataCell &cell);
}

#endif // LYRIC_RUNTIME_DATA_CELL_H
