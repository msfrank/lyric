#ifndef LYRIC_RUNTIME_OPERAND_H
#define LYRIC_RUNTIME_OPERAND_H

#include <tempo_utils/integer_types.h>

#include "abstract_ref.h"
#include "data_cell.h"

namespace lyric_runtime {

    constexpr tu_uint32 kInvalidSmallValue      = 0x00000008;
    constexpr tu_uint64 kInvalidLargeValue      = 0x0000000000000007;

    enum class OverlayType {
        Invalid,
        SmallValue,
        LargeValue,
        Pointer,
    };

    class Operand final {
    public:
        Operand();
        Operand(const Operand &other);
        Operand(Operand &&other) noexcept;

        Operand& operator=(const Operand &other);
        Operand& operator=(Operand &&other) noexcept;

        bool isValid() const;

        OverlayType getOverlay() const;
        DataCellType getType() const;

        std::span<const tu_uint8> getBytes() const;
        size_t getSize() const;

        bool isUndef() const;
        bool isNil() const;

        bool getBool(bool &b);
        bool getI8(tu_int8 &i8);
        bool getI16(tu_int16 &i16);
        bool getI32(tu_int32 &i32);
        bool getI64(tu_int64 &i64);
        bool getU8(tu_uint8 &u8);
        bool getU16(tu_uint16 &u16);
        bool getU32(tu_uint32 &u32);
        bool getU64(tu_uint64 &u64);
        bool getC32(char32_t &c32);
        bool getF32(float &f32);
        bool getF64(double &f64);
        bool getRef(AbstractRef *&ref);
        bool getBytes(BytesRef *&bytes);
        bool getNamespace(NamespaceRef *&ns);
        bool getProtocol(ProtocolRef *&protocol);
        bool getRest(RestRef *&rest);
        bool getStatus(StatusRef *&status);
        bool getString(StringRef *&string);
        bool getDescriptor(DescriptorEntry *&descriptor);
        bool getType(TypeEntry *&type);

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
        static Operand fromRef(AbstractRef *ref);
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
        void *getPointer(tu_uint8 pointertag);
    };
}

#endif // LYRIC_RUNTIME_OPERAND_H
