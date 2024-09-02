#ifndef LYRIC_OBJECT_OBJECT_TYPES_H
#define LYRIC_OBJECT_OBJECT_TYPES_H

#include <absl/container/flat_hash_map.h>

#include <lyric_common/module_location.h>
#include <lyric_common/symbol_url.h>
#include <lyric_common/type_def.h>
#include <tempo_utils/option_template.h>

namespace lyric_object {

    constexpr tu_uint32 INVALID_ADDRESS_U32         = 0xFFFFFFFF;
    constexpr tu_uint16 INVALID_OFFSET_U16          = 0xFFFF;
    constexpr tu_uint8 INVALID_OFFSET_U8            = 0xFF;

    // load type enum
    constexpr tu_uint8 LOAD_ARGUMENT                = 0x01;
    constexpr tu_uint8 LOAD_LOCAL                   = 0x02;
    constexpr tu_uint8 LOAD_LEXICAL                 = 0x03;
    constexpr tu_uint8 LOAD_FIELD                   = 0x04;
    constexpr tu_uint8 LOAD_STATIC                  = 0x05;
    constexpr tu_uint8 LOAD_INSTANCE                = 0x06;
    constexpr tu_uint8 LOAD_ENUM                    = 0x07;

    // store type enum
    constexpr tu_uint8 STORE_ARGUMENT               = 0x01;
    constexpr tu_uint8 STORE_LOCAL                  = 0x02;
    constexpr tu_uint8 STORE_LEXICAL                = 0x03;
    constexpr tu_uint8 STORE_FIELD                  = 0x04;
    constexpr tu_uint8 STORE_STATIC                 = 0x05;

    // descriptor type enum

    // call flags
    constexpr tu_uint8 CALL_RECEIVER_FOLLOWS        = 0x01;
    constexpr tu_uint8 CALL_FORWARD_REST            = 0x02;
    constexpr tu_uint8 CALL_FLAG_UNUSED1            = 0x04;
    constexpr tu_uint8 CALL_FLAG_UNUSED2            = 0x08;
    constexpr tu_uint8 CALL_HIGH_BIT                = 0x80;

    // lexical type enum
    constexpr tu_uint8 LEXICAL_ARGUMENT             = 0x01;
    constexpr tu_uint8 LEXICAL_LOCAL                = 0x02;
    constexpr tu_uint8 LEXICAL_LEXICAL              = 0x03;

    // new type enum
    constexpr tu_uint8 NEW_CLASS                    = 0x01;
    constexpr tu_uint8 NEW_ENUM                     = 0x02;
    constexpr tu_uint8 NEW_INSTANCE                 = 0x03;
    constexpr tu_uint8 NEW_STRUCT                   = 0x04;

    // TRAP flags
    constexpr tu_uint8 TRAP_INDEX_FOLLOWS           = 0x01;

    // synthetic type enum
    constexpr tu_uint8 SYNTHETIC_THIS               = 0x01;

    // extract the CALL flags
    constexpr tu_uint8 GET_CALL_FLAGS(tu_uint8 flags) { return flags & 0x0F; }

    // extract the NEW type
    constexpr tu_uint8 GET_NEW_TYPE(tu_uint8 flags) { return (flags & 0x70) >> 4u; }

    // check high bit to determine whether an address is near (in the current segment) or far (in another segment)
    constexpr bool IS_NEAR(tu_uint32 address) { return address >> 31u == 0u; }
    constexpr bool IS_FAR(tu_uint32 address) { return address >> 31u == 1u; }
    constexpr bool IS_VALID(tu_uint32 address) { return address != INVALID_ADDRESS_U32; }

    /**
     * If the given address is a near address then extract and return the descriptor offset, otherwise
     * return the invalid value 0xFFFFFFFF.
     *
     * @param address The address.
     * @return The descriptor offset, or 0xFFFFFFFF.
     */
    constexpr tu_uint32 GET_DESCRIPTOR_OFFSET(tu_uint32 address) {
        return IS_NEAR(address)? address : INVALID_ADDRESS_U32;
    }

    /**
     * If the given descriptor offset is within the valid range 0 <= 2^31 then return the descriptor address,
     * otherwise return the invalid value 0xFFFFFFFF.
     */
    constexpr tu_uint32 GET_DESCRIPTOR_ADDRESS(tu_uint32 offset) {
        return offset <= std::numeric_limits<tu_int32>::max()? offset : INVALID_ADDRESS_U32;
    }

    /**
     * If the given address is a far address then extract and return the link offset, otherwise return
     * the invalid value 0xFFFFFFFF.
     *
     * @param address The address.
     * @return The link offset, or 0xFFFFFFFF.
     */
    constexpr tu_uint32 GET_LINK_OFFSET(tu_uint32 address) {
        return IS_FAR(address)? address & 0x7FFFFFFF : INVALID_ADDRESS_U32;
    }

    /**
     * If the given link offset is within the valid range 0 <= 2^31 then return the link address,
     * otherwise return the invalid value 0xFFFFFFFF.
     */
    constexpr tu_uint32 GET_LINK_ADDRESS(tu_uint32 offset) {
        return offset <= std::numeric_limits<tu_int32>::max()? offset | 0x80000000 : INVALID_ADDRESS_U32;
    }

    /**
     * The type of an address, based on the address bit-pattern.
     */
    enum class AddressType {
        Invalid,                /**< The address is invalid. */
        Near,                   /**< The addressee exists in the current segment. */
        Far,                    /**< The addressee exists in a different segment than the current segment. */
    };

    /**
     * Returns the address type for the given address.
     *
     * @param address The address.
     * @return The address type.
     */
    constexpr AddressType GET_ADDRESS_TYPE(tu_uint32 address) {
        if (address == INVALID_ADDRESS_U32) return AddressType::Invalid;
        return IS_NEAR(address)? AddressType::Near: AddressType::Far;
    }

    enum class ObjectVersion {
        Unknown,
        Version1,
    };

    enum class IntrinsicType {
        Invalid,
        Nil,
        Undef,
        Bool,
        Char,
        Float,
        Int,
        String,
        Url,
        Class,
        Concept,
        Instance,
        Call,
        Action,
        Field,
        Struct,
        Enum,
        Existential,
        Namespace,

        // must be last
        NUM_INTRINSIC_TYPES,
    };

    enum class ValueType {
        Invalid,
        Nil,
        Undef,
        Bool,
        Int64,
        Float64,
        Char,
        String,
        Url,
    };

    enum class AccessType {
        Invalid,
        Public,
        Protected,
        Private,
    };

    enum class BoundType {
        Invalid,
        None,
        Extends,
        Super,
    };

    enum class VarianceType {
        Invalid,
        Invariant,
        Covariant,
        Contravariant,
    };

    enum class PlacementType {
        Invalid,
        List,
        ListOpt,
        Named,
        NamedOpt,
        Rest,
        Ctx,
    };

    enum class DeriveType {
        Invalid,
        Any,
        Sealed,
        Final,
    };

    enum class CallMode {
        Invalid,
        Normal,
        Constructor,
        Inline,
    };

    enum class HashType {
        Invalid,
        None,
        Sha256,
    };

    // import flags
    constexpr tu_uint8 IMPORT_SYSTEM_BOOSTRAP   = 0x01;
    constexpr tu_uint8 IMPORT_API_LINKAGE       = 0x02;
    constexpr tu_uint8 IMPORT_EXACT_LINKAGE     = 0x04;

    enum class Opcode : tu_uint8 {
        OP_UNKNOWN, // must not be used

        OP_NOOP,

        // load and store
        OP_NIL,
        OP_UNDEF,
        OP_TRUE,
        OP_FALSE,
        OP_I64,
        OP_DBL,
        OP_CHR,
        OP_LITERAL,
        OP_STRING,
        OP_URL,
        OP_STATIC,
        OP_SYNTHETIC,
        OP_DESCRIPTOR,
        OP_LOAD,
        OP_STORE,

        // variadic args
        OP_VA_LOAD,
        OP_VA_SIZE,

        // data stack management
        OP_POP,
        OP_DUP,
        OP_PICK,
        OP_DROP,
        OP_RPICK,
        OP_RDROP,

        // integer math
        OP_I64_ADD,
        OP_I64_SUB,
        OP_I64_MUL,
        OP_I64_DIV,
        OP_I64_NEG,

        // rational math
        OP_DBL_ADD,
        OP_DBL_SUB,
        OP_DBL_MUL,
        OP_DBL_DIV,
        OP_DBL_NEG,

        // intrinsic comparisons
        OP_BOOL_CMP,
        OP_I64_CMP,
        OP_DBL_CMP,
        OP_CHR_CMP,
        OP_TYPE_CMP,

        // logical operations
        OP_LOGICAL_AND,
        OP_LOGICAL_OR,
        OP_LOGICAL_NOT,

        // branching
        OP_IF_NIL,
        OP_IF_NOTNIL,
        OP_IF_TRUE,
        OP_IF_FALSE,
        OP_IF_ZERO,
        OP_IF_NOTZERO,
        OP_IF_GT,
        OP_IF_GE,
        OP_IF_LT,
        OP_IF_LE,
        OP_JUMP,

        // import assembly
        OP_IMPORT,

        // procedure invocation
        OP_CALL_STATIC,
        OP_CALL_VIRTUAL,
        OP_CALL_CONCEPT,
        OP_CALL_EXISTENTIAL,
        OP_TRAP,
        OP_RETURN,

        // heap allocation
        OP_NEW,

        // interpreter services
        OP_TYPE_OF,
        OP_INTERRUPT,
        OP_HALT,
        OP_ABORT,

        // sentinel, must not be used
        LAST_
    };

    tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage&& message, Opcode opcode);

    /**
     * Enumeration of symbol linkage types.
     */
    enum class LinkageSection {
        Invalid,
        Type,
        Existential,
        Literal,
        Call,
        Field,
        Static,
        Action,
        Class,
        Struct,
        Instance,
        Concept,
        Enum,
        Namespace,
    };

    LinkageSection descriptor_to_linkage_section(tu_uint8 section);
    tu_uint8 linkage_to_descriptor_section(lyric_object::LinkageSection section);

    struct ProcHeader {
        tu_uint32 procSize = 0;
        tu_uint32 headerSize = 0;
        tu_uint16 numArguments = 0;
        tu_uint16 numLocals = 0;
        tu_uint16 numLexicals = 0;
    };

    /**
     * Describes a parameter.
     */
    struct Parameter {
        std::string name = {};                               /**< name of the parameter */
        int index = -1;                                      /**< placement index */
        PlacementType placement = PlacementType::Invalid;    /**< parameter placement */
        lyric_common::TypeDef typeDef = {};                  /**< type of the parameter */
        bool isVariable = false;                             /**< true if the parameter has variable binding, otherwise false */
    };

    /**
     * Describes a template parameter.
     */
    struct TemplateParameter {
        std::string name = {};                           /**< */
        int index = -1;                                  /**< */
        lyric_common::TypeDef typeDef = {};              /**< */
        VarianceType variance = VarianceType::Invalid;   /**< */
        BoundType bound = BoundType::Invalid;            /**< */
    };

    class LiteralIndex {
    public:
        LiteralIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit LiteralIndex(tu_uint32 u32) : u32(u32) {};
        LiteralIndex(const LiteralIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const LiteralIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    class NamespaceIndex {
    public:
        NamespaceIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit NamespaceIndex(tu_uint32 u32) : u32(u32) {};
        NamespaceIndex(const NamespaceIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const NamespaceIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    class ExistentialIndex {
    public:
        ExistentialIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit ExistentialIndex(tu_uint32 u32) : u32(u32) {};
        ExistentialIndex(const ExistentialIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const ExistentialIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    class CallIndex {
    public:
        CallIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit CallIndex(tu_uint32 u32) : u32(u32) {};
        CallIndex(const CallIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const CallIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    class FieldIndex {
    public:
        FieldIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit FieldIndex(tu_uint32 u32) : u32(u32) {};
        FieldIndex(const FieldIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const FieldIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    class ActionIndex {
    public:
        ActionIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit ActionIndex(tu_uint32 u32) : u32(u32) {};
        ActionIndex(const ActionIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const ActionIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    class ConceptIndex {
    public:
        ConceptIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit ConceptIndex(tu_uint32 u32) : u32(u32) {};
        ConceptIndex(const ConceptIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const ConceptIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    class ClassIndex {
    public:
        ClassIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit ClassIndex(tu_uint32 u32) : u32(u32) {};
        ClassIndex(const ClassIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const ClassIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    class InstanceIndex {
    public:
        InstanceIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit InstanceIndex(tu_uint32 u32) : u32(u32) {};
        InstanceIndex(const InstanceIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const InstanceIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    class EnumIndex {
    public:
        EnumIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit EnumIndex(tu_uint32 u32) : u32(u32) {};
        EnumIndex(const EnumIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const EnumIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    class StructIndex {
    public:
        StructIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit StructIndex(tu_uint32 u32) : u32(u32) {};
        StructIndex(const StructIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const StructIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    class StaticIndex {
    public:
        StaticIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit StaticIndex(tu_uint32 u32) : u32(u32) {};
        StaticIndex(const StaticIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const StaticIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    struct TemplateIndex {
    public:
        TemplateIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit TemplateIndex(tu_uint32 u32) : u32(u32) {};
        TemplateIndex(const TemplateIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const TemplateIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    class TypeIndex {
    public:
        TypeIndex() : u32(INVALID_ADDRESS_U32) {};
        explicit TypeIndex(tu_uint32 u32) : u32(u32) {};
        TypeIndex(const TypeIndex &other) : u32(other.u32) {};
        bool isValid() const { return u32 != INVALID_ADDRESS_U32; }
        tu_uint32 getIndex() const { return u32; };
        bool operator==(const TypeIndex &other) const { return u32 == other.u32; };
    private:
        tu_uint32 u32;
    };

    // forward declarations
    namespace internal {
        class ObjectReader;
    }
}

#endif // LYRIC_OBJECT_OBJECT_TYPES_H