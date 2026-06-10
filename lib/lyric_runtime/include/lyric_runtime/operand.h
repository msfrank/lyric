#ifndef LYRIC_RUNTIME_OPERAND_H
#define LYRIC_RUNTIME_OPERAND_H

#include <span>

#include <absl/hash/hash.h>

#include <lyric_object/object_types.h>
#include <tempo_utils/integer_types.h>

namespace lyric_runtime {

    // forward declarations
    class AbstractRef;
    class BaseRef;
    class BytesRef;
    class DescriptorEntry;
    class NamespaceRef;
    class ProtocolRef;
    class RestRef;
    class StatusRef;
    class StringRef;
    class TypeEntry;

    constexpr tu_uint32 kInvalidSmallValue      = 0x00000008;
    constexpr tu_uint64 kInvalidLargeValue      = 0x0000000000000007;

    enum class OverlayType {
        Invalid,
        SmallValue,
        LargeValue,
        Pointer,
    };

    enum class OperandType : uint8_t {
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

    class Operand final {
    public:
        Operand();
        Operand(const Operand &other);
        Operand(Operand &&other) noexcept;

        Operand& operator=(const Operand &other);
        Operand& operator=(Operand &&other) noexcept;

        bool isValid() const;

        bool isEqualTo(const Operand &other) const;
        bool isSameAs(const Operand &other) const;

        OverlayType getOverlay() const;
        OperandType getType() const;

        std::span<const tu_uint8> getBytes() const;
        size_t getSize() const;

        bool isUndef() const;
        bool isNil() const;

        bool isIntegral() const;
        bool isRational() const;
        bool isSigned() const;
        bool isUnsigned() const;
        bool isReference() const;

        bool getBool(bool &b) const;
        bool getI8(tu_int8 &i8) const;
        bool getI16(tu_int16 &i16) const;
        bool getI32(tu_int32 &i32) const;
        bool getI64(tu_int64 &i64) const;
        bool getU8(tu_uint8 &u8) const;
        bool getU16(tu_uint16 &u16) const;
        bool getU32(tu_uint32 &u32) const;
        bool getU64(tu_uint64 &u64) const;
        bool getC32(char32_t &c32) const;
        bool getF32(float &f32) const;
        bool getF64(double &f64) const;
        bool getRef(BaseRef *&ref) const;
        bool getBytes(BytesRef *&bytes) const;
        bool getNamespace(NamespaceRef *&ns) const;
        bool getProtocol(ProtocolRef *&protocol) const;
        bool getRest(RestRef *&rest) const;
        bool getStatus(StatusRef *&status) const;
        bool getString(StringRef *&string) const;
        bool getDescriptor(DescriptorEntry *&descriptor) const;
        bool getDescriptor(DescriptorEntry *&descriptor, lyric_object::LinkageSection section) const;
        bool getType(TypeEntry *&type) const;

        void setReachable() const;
        void clearReachable() const;
        bool isReachable() const;

        void hashEquality(absl::HashState state) const;
        void hashIdentity(absl::HashState state) const;

        std::string toString() const;

        static Operand fromBool(bool b);
        static Operand fromI8(tu_int8 i8);
        static Operand fromI16(tu_int16 i16);
        static Operand fromI32(tu_int32 i32);
        static Operand fromI64(tu_int64 i64);
        static Operand fromU8(tu_uint8 u8);
        static Operand fromU16(tu_uint16 u16);
        static Operand fromU32(tu_uint32 u32);
        static Operand fromU64(tu_uint64 u64);
        static Operand fromC32(char32_t c32);
        static Operand fromF32(float f32);
        static Operand fromF64(double f64);
        static Operand fromRef(BaseRef *ref);
        static Operand fromBytes(BytesRef *bytes);
        static Operand fromNamespace(NamespaceRef *ns);
        static Operand fromProtocol(ProtocolRef *protocol);
        static Operand fromRest(RestRef *rest);
        static Operand fromStatus(StatusRef *status);
        static Operand fromString(StringRef *str);
        static Operand fromDescriptor(DescriptorEntry *descriptor);
        static Operand fromType(TypeEntry *type);

        static Operand nil();
        static Operand undef();

        static Operand parse(std::span<const tu_uint8> bytes);
        static OverlayType parseRepresentation(const tu_uint8 &infoByte);
        static size_t parseSize(const tu_uint8 &infoByte);

    private:
        std::array<tu_uint8,8> m_bytes;

        explicit Operand(std::array<tu_uint8,8> bytes);

        static Operand fromPointer(void *ptr, tu_uint8 pointertag);
        void *getPointer(tu_uint8 pointertag) const;
    };

    bool operand_to_value(const Operand &op, bool &v);
    bool operand_to_value(const Operand &op, tu_int8 &v);
    bool operand_to_value(const Operand &op, tu_int16 &v);
    bool operand_to_value(const Operand &op, tu_int32 &v);
    bool operand_to_value(const Operand &op, tu_int64 &v);
    bool operand_to_value(const Operand &op, tu_uint8 &v);
    bool operand_to_value(const Operand &op, tu_uint16 &v);
    bool operand_to_value(const Operand &op, tu_uint32 &v);
    bool operand_to_value(const Operand &op, tu_uint64 &v);
    bool operand_to_value(const Operand &op, float &v);
    bool operand_to_value(const Operand &op, double &v);
    bool operand_to_value(const Operand &op, char32_t &v);

    bool operand_to_pointer(const Operand &op, BaseRef *&r);
    bool operand_to_pointer(const Operand &op, BytesRef *&r);
    bool operand_to_pointer(const Operand &op, NamespaceRef *&r);
    bool operand_to_pointer(const Operand &op, ProtocolRef *&r);
    bool operand_to_pointer(const Operand &op, RestRef *&r);
    bool operand_to_pointer(const Operand &op, StatusRef *&r);
    bool operand_to_pointer(const Operand &op, StringRef *&r);
    bool operand_to_pointer(const Operand &op, DescriptorEntry *&e);
    bool operand_to_pointer(const Operand &op, TypeEntry *&e);

    void value_to_operand(bool v, Operand &op);
    void value_to_operand(tu_int8 v, Operand &op);
    void value_to_operand(tu_int16 v, Operand &op);
    void value_to_operand(tu_int32 v, Operand &op);
    void value_to_operand(tu_int64 v, Operand &op);
    void value_to_operand(tu_uint8 v, Operand &op);
    void value_to_operand(tu_uint16 v, Operand &op);
    void value_to_operand(tu_uint32 v, Operand &op);
    void value_to_operand(tu_uint64 v, Operand &op);
    void value_to_operand(float v, Operand &op);
    void value_to_operand(double v, Operand &op);
    void value_to_operand(char32_t v, Operand &op);

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage &&message, const Operand &operand);

    struct OperandIdentity {
        Operand operand;

        OperandIdentity(Operand op): operand(std::move(op)) {}

        template <typename H>
        friend H AbslHashValue(H h, const OperandIdentity &id) {
            id.operand.hashIdentity(absl::HashState::Create(&h));
            return std::move(h);
        }

        bool operator==(const OperandIdentity &other) const {
            return operand.isSameAs(other.operand);
        }
    };

    struct OperandEquality {
        Operand operand;

        OperandEquality(Operand op): operand(std::move(op)) {}

        template <typename H>
        friend H AbslHashValue(H h, const OperandEquality &eq) {
            eq.operand.hashEquality(absl::HashState::Create(&h));
            return std::move(h);
        }

        bool operator==(const OperandEquality &other) const {
            return operand.isEqualTo(other.operand);
        }
    };
}

#endif // LYRIC_RUNTIME_OPERAND_H
