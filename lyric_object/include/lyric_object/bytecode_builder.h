#ifndef LYRIC_OBJECT_BYTECODE_BUILDER_H
#define LYRIC_OBJECT_BYTECODE_BUILDER_H

#include <vector>

#include <unicode/umachine.h>

#include <tempo_utils/result.h>

#include "object_types.h"

namespace lyric_object {

    class BytecodeBuilder {

    public:
        BytecodeBuilder() = default;
        explicit BytecodeBuilder(const std::vector<tu_uint8> &bytecode);
        BytecodeBuilder(const BytecodeBuilder &other);
        BytecodeBuilder(BytecodeBuilder &&other);

        BytecodeBuilder& operator=(const BytecodeBuilder &other);
        BytecodeBuilder& operator=(BytecodeBuilder &&other) noexcept;

        std::vector<tu_uint8> getBytecode() const;
        tu_uint32 bytecodeSize() const;
        std::vector<tu_uint8>::const_iterator bytecodeBegin() const;
        std::vector<tu_uint8>::const_iterator bytecodeEnd() const;

        int currentOffset() const;

        tempo_utils::Status writeOpcode(Opcode op);
        tempo_utils::Status writeU8(tu_uint8 u8);
        tempo_utils::Status writeU16(tu_uint16 u16);
        tempo_utils::Status writeU32(tu_uint32 u32);
        tempo_utils::Status writeU64(tu_uint64 u64);

        tempo_utils::Status loadNil();
        tempo_utils::Status loadUndef();
        tempo_utils::Status loadBool(bool b);
        tempo_utils::Status loadInt(tu_int64 i64);
        tempo_utils::Status loadFloat(double dbl);
        tempo_utils::Status loadChar(UChar32 chr);

        tempo_utils::Status loadArgument(tu_uint16 index);
        tempo_utils::Status loadLocal(tu_uint16 index);
        tempo_utils::Status loadLexical(tu_uint16 index);
        tempo_utils::Status loadReceiver();

        tempo_utils::Status loadLiteral(tu_uint32 address);
        tempo_utils::Status loadField(tu_uint32 address);
        tempo_utils::Status loadStatic(tu_uint32 address);
        tempo_utils::Status loadEnum(tu_uint32 address);
        tempo_utils::Status loadInstance(tu_uint32 address);
        tempo_utils::Status loadClass(tu_uint32 address);
        tempo_utils::Status loadStruct(tu_uint32 address);
        tempo_utils::Status loadConcept(tu_uint32 address);
        tempo_utils::Status loadCall(tu_uint32 address);
        tempo_utils::Status loadType(tu_uint32 address);

        tempo_utils::Status storeArgument(tu_uint16 index);
        tempo_utils::Status storeLocal(tu_uint16 index);
        tempo_utils::Status storeLexical(tu_uint16 index);

        tempo_utils::Status storeField(tu_uint32 address);
        tempo_utils::Status storeStatic(tu_uint32 address);

        tempo_utils::Status jumpIfNil(tu_uint16 &patchOffset);
        tempo_utils::Status jumpIfNotNil(tu_uint16 &patchOffset);
        tempo_utils::Status jumpIfTrue(tu_uint16 &patchOffset);
        tempo_utils::Status jumpIfFalse(tu_uint16 &patchOffset);
        tempo_utils::Status jumpIfZero(tu_uint16 &patchOffset);
        tempo_utils::Status jumpIfNotZero(tu_uint16 &patchOffset);
        tempo_utils::Status jumpIfLessThan(tu_uint16 &patchOffset);
        tempo_utils::Status jumpIfLessOrEqual(tu_uint16 &patchOffset);
        tempo_utils::Status jumpIfGreaterThan(tu_uint16 &patchOffset);
        tempo_utils::Status jumpIfGreaterOrEqual(tu_uint16 &patchOffset);
        tempo_utils::Status jump(tu_uint16 &patchOffset);
        tempo_utils::Status jumpTo(tu_uint16 jumpLabel);

        tempo_utils::Status makeLabel(tu_uint16 &jumpLabel);
        tempo_utils::Status patch(tu_uint16 patchOffset, tu_uint16 jumpLabel);

        tempo_utils::Status popValue();
        tempo_utils::Status dupValue();
        tempo_utils::Status pickValue(tu_uint16 pickOffset);
        tempo_utils::Status dropValue(tu_uint16 dropOffset);
        tempo_utils::Status rpickValue(tu_uint16 pickOffset);
        tempo_utils::Status rdropValue(tu_uint16 dropOffset);

        tempo_utils::Status callStatic(
            tu_uint32 address,
            tu_uint16 placementSize,
            tu_uint8 flags = 0);

        tempo_utils::Status callVirtual(
            tu_uint32 address,
            tu_uint16 placementSize,
            tu_uint8 flags = 0);

        tempo_utils::Status callConcept(
            tu_uint32 address,
            tu_uint16 placementSize,
            tu_uint8 flags = 0);

        tempo_utils::Status callExistential(
            tu_uint32 address,
            tu_uint16 placementSize,
            tu_uint8 flags = 0);

        tempo_utils::Status callNew(
            tu_uint32 address,
            tu_uint16 placementSize,
            tu_uint8 newType,
            tu_uint8 flags = 0);

        tempo_utils::Status callInline(const BytecodeBuilder *inlineCode, tu_uint8 flags = 0);

        tempo_utils::Status trap(tu_uint32 address, tu_uint8 flags = 0);

    private:
        std::vector<tu_uint8> m_code;

        tempo_utils::Status makePatch(tu_uint16 &patchOffset);
    };
}

#endif // LYRIC_OBJECT_BYTECODE_BUILDER_H