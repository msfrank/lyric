#ifndef LYRIC_ASSEMBLER_CODE_BUILDER_H
#define LYRIC_ASSEMBLER_CODE_BUILDER_H

#include <vector>

#include "assembler_types.h"
#include "object_state.h"

namespace lyric_assembler {

    class CodeBuilder {
    public:
        CodeBuilder() = default;
        explicit CodeBuilder(const std::vector<tu_uint8> &bytecode);
        CodeBuilder(const CodeBuilder &other);

        std::vector<tu_uint8> getBytecode() const;
        tu_uint32 bytecodeSize() const;
        std::vector<tu_uint8>::const_iterator bytecodeBegin() const;
        std::vector<tu_uint8>::const_iterator bytecodeEnd() const;

        int currentOffset() const;

        tempo_utils::Status writeOpcode(lyric_object::Opcode op);
        tempo_utils::Status writeU8(tu_uint8 u8);
        tempo_utils::Status writeU16(tu_uint16 u16);
        tempo_utils::Status writeU32(tu_uint32 u32);
        tempo_utils::Status writeU64(uint64_t u64);

        tempo_utils::Status loadNil();
        tempo_utils::Status loadUndef();
        tempo_utils::Status loadBool(bool b);
        tempo_utils::Status loadInt(int64_t i64);
        tempo_utils::Status loadFloat(double dbl);
        tempo_utils::Status loadChar(UChar32 chr);
        tempo_utils::Status loadString(const LiteralAddress &stringAddress);
        tempo_utils::Status loadUrl(const LiteralAddress &urlAddress);
        tempo_utils::Status loadLiteral(const LiteralAddress &literalAddress);
        tempo_utils::Status loadArgument(const ArgumentOffset &argumentOffset);
        tempo_utils::Status loadLocal(const LocalOffset &localOffset);
        tempo_utils::Status loadLexical(const LexicalOffset &lexicalOffset);
        tempo_utils::Status loadField(const FieldAddress &fieldAddress);
        tempo_utils::Status loadStatic(const StaticAddress &staticAddress);
        tempo_utils::Status loadEnum(const EnumAddress &enumAddress);
        tempo_utils::Status loadInstance(const InstanceAddress &instanceAddress);
        tempo_utils::Status loadSynthetic(SyntheticType syntheticType);
        tempo_utils::Status loadClass(const ClassAddress &classAddress);
        tempo_utils::Status loadStruct(const StructAddress &structAddress);
        tempo_utils::Status loadConcept(const ConceptAddress &conceptAddress);
        tempo_utils::Status loadCall(const CallAddress &callAddress);
        tempo_utils::Status loadExistential(const ExistentialAddress &existentialAddress);
        tempo_utils::Status loadType(const TypeAddress &typeAddress);

        tempo_utils::Status storeArgument(const ArgumentOffset &argumentOffset);
        tempo_utils::Status storeLocal(const LocalOffset &localOffset);
        tempo_utils::Status storeLexical(const LexicalOffset &lexicalOffset);
        tempo_utils::Status storeField(const FieldAddress &fieldAddress);
        tempo_utils::Status storeStatic(const StaticAddress &staticAddress);

        tempo_utils::Result<PatchOffset> jumpIfNil();
        tempo_utils::Result<PatchOffset> jumpIfNotNil();
        tempo_utils::Result<PatchOffset> jumpIfTrue();
        tempo_utils::Result<PatchOffset> jumpIfFalse();
        tempo_utils::Result<PatchOffset> jumpIfZero();
        tempo_utils::Result<PatchOffset> jumpIfNotZero();
        tempo_utils::Result<PatchOffset> jumpIfLessThan();
        tempo_utils::Result<PatchOffset> jumpIfLessOrEqual();
        tempo_utils::Result<PatchOffset> jumpIfGreaterThan();
        tempo_utils::Result<PatchOffset> jumpIfGreaterOrEqual();
        tempo_utils::Result<PatchOffset> jump();
        tempo_utils::Status jump(const JumpLabel &label);

        tempo_utils::Result<JumpLabel> makeLabel();
        tempo_utils::Status patch(const PatchOffset &dst, const JumpLabel &src);

        tempo_utils::Status popValue();
        tempo_utils::Status dupValue();
        tempo_utils::Status pickValue(tu_uint16 pickOffset);
        tempo_utils::Status dropValue(tu_uint16 dropOffset);
        tempo_utils::Status rpickValue(tu_uint16 pickOffset);
        tempo_utils::Status rdropValue(tu_uint16 dropOffset);

        tempo_utils::Status callStatic(
            const CallAddress &address,
            tu_uint16 placementSize,
            tu_uint8 flags = 0);

        tempo_utils::Status callVirtual(
            const CallAddress &address,
            tu_uint16 placementSize,
            tu_uint8 flags = 0);

//        tempo_utils::Status callExtension(
//            const ActionAddress &address,
//            tu_uint16 placementSize,
//            tu_uint8 flags = 0);

        tempo_utils::Status callConcept(
            const ActionAddress &address,
            tu_uint16 placementSize,
            tu_uint8 flags = 0);

        tempo_utils::Status callExistential(
            const CallAddress &address,
            tu_uint16 placementSize,
            tu_uint8 flags = 0);

        tempo_utils::Status callNew(
            tu_uint32 address,
            tu_uint16 placementSize,
            tu_uint8 newType,
            tu_uint8 flags = 0);

        tempo_utils::Status callInline(const CodeBuilder *inlineCode, tu_uint8 flags = 0);

        tempo_utils::Status trap(tu_uint32 address, tu_uint8 flags = 0);

    private:
        std::vector<tu_uint8> m_code;

        tempo_utils::Result<PatchOffset> makePatch();
    };
}

#endif // LYRIC_ASSEMBLER_CODE_BUILDER_H
