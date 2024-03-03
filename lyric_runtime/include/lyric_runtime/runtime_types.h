#ifndef LYRIC_RUNTIME_RUNTIME_TYPES_H
#define LYRIC_RUNTIME_RUNTIME_TYPES_H

#include <lyric_common/assembly_location.h>
#include <lyric_common/symbol_url.h>
#include <lyric_object/object_types.h>
#include <tempo_utils/option_template.h>

#include "literal_cell.h"

#if defined(TARGET_OS_MAC)
#define PLUGIN_FILE_SUFFIX      "dylib"
#elif defined(TARGET_OS_WINDOWS)
#define PLUGIN_FILE_SUFFIX      "dll"
#else
#define PLUGIN_FILE_SUFFIX      "so"
#endif

namespace lyric_runtime {

    constexpr tu_uint32 INVALID_ADDRESS_U32      = 0xFFFFFFFF;
    constexpr tu_uint16 INVALID_OFFSET_U16       = 0xFFFF;
    constexpr tu_uint8 INVALID_OFFSET_U8         = 0xFF;

    // // LOAD flags
    // constexpr tu_uint8 LOAD_ARGUMENT             = 0x01;
    // constexpr tu_uint8 LOAD_LOCAL                = 0x02;
    // constexpr tu_uint8 LOAD_LEXICAL              = 0x03;
    // constexpr tu_uint8 LOAD_FIELD                = 0x04;
    // constexpr tu_uint8 LOAD_STATIC               = 0x05;
    // constexpr tu_uint8 LOAD_INSTANCE             = 0x06;
    // constexpr tu_uint8 LOAD_ENUM                 = 0x07;
    //
    // // STORE flags
    // constexpr tu_uint8 STORE_ARGUMENT            = 0x01;
    // constexpr tu_uint8 STORE_LOCAL               = 0x02;
    // constexpr tu_uint8 STORE_LEXICAL             = 0x03;
    // constexpr tu_uint8 STORE_FIELD               = 0x04;
    // constexpr tu_uint8 STORE_STATIC              = 0x05;
    //
    // // CALL flags
    // constexpr tu_uint8 CALL_RECEIVER_FOLLOWS     = 0x01;
    // constexpr tu_uint8 CALL_FORWARD_REST         = 0x02;
    // constexpr tu_uint8 CALL_FLAG_UNUSED1         = 0x04;
    // constexpr tu_uint8 CALL_FLAG_UNUSED2         = 0x08;
    // constexpr tu_uint8 CALL_HIGH_BIT             = 0x80;
    //
    // // NEW type
    // constexpr tu_uint8 NEW_CLASS                 = 0x01;
    // constexpr tu_uint8 NEW_ENUM                  = 0x02;
    // constexpr tu_uint8 NEW_INSTANCE              = 0x03;
    // constexpr tu_uint8 NEW_STRUCT                = 0x04;
    //
    // // TRAP flags
    // constexpr tu_uint8 TRAP_INDEX_FOLLOWS        = 0x01;
    //
    // // extract the CALL flags
    // constexpr tu_uint8 GET_CALL_FLAGS(tu_uint8 flags) { return flags & 0x0F; }
    //
    // // extract the NEW type
    // constexpr tu_uint8 GET_NEW_TYPE(tu_uint8 flags) { return (flags & 0x70) >> 4u; }
    //
    // // check high bit to determine whether an address is near (in the current segment) or far (in another segment)
    // constexpr bool IS_NEAR(tu_uint32 address) { return address >> 31u == 0u; }
    // constexpr bool IS_FAR(tu_uint32 address) { return address >> 31u == 1u; }
    //
    // enum class AssemblyVersion {
    //     Unknown,
    //     Version1,
    // };
    //
    // enum class HashType {
    //     Invalid,
    //     None,
    //     Sha256,
    // };
    //
    // // import flags
    // constexpr tu_uint8 IMPORT_SYSTEM_BOOSTRAP   = 0x01;
    // constexpr tu_uint8 IMPORT_API_LINKAGE       = 0x02;
    // constexpr tu_uint8 IMPORT_EXACT_LINKAGE     = 0x04;
    //
    // enum class Opcode : tu_uint8 {
    //     OP_UNKNOWN, // must not be used
    //
    //     OP_NOOP,
    //
    //     // load and store
    //     OP_NIL,
    //     OP_TRUE,
    //     OP_FALSE,
    //     OP_I64,
    //     OP_DBL,
    //     OP_CHR,
    //     OP_LITERAL,
    //     OP_STATIC,
    //     OP_SYNTHETIC,
    //     OP_DESCRIPTOR,
    //     OP_LOAD,
    //     OP_STORE,
    //
    //     // variadic args
    //     OP_VA_LOAD,
    //     OP_VA_SIZE,
    //
    //     // data stack management
    //     OP_POP,
    //     OP_DUP,
    //     OP_PICK,
    //     OP_DROP,
    //     OP_RPICK,
    //     OP_RDROP,
    //
    //     // integer math
    //     OP_I64_ADD,
    //     OP_I64_SUB,
    //     OP_I64_MUL,
    //     OP_I64_DIV,
    //     OP_I64_NEG,
    //
    //     // rational math
    //     OP_DBL_ADD,
    //     OP_DBL_SUB,
    //     OP_DBL_MUL,
    //     OP_DBL_DIV,
    //     OP_DBL_NEG,
    //
    //     // intrinsic comparisons
    //     OP_BOOL_CMP,
    //     OP_I64_CMP,
    //     OP_DBL_CMP,
    //     OP_CHR_CMP,
    //     OP_TYPE_CMP,
    //
    //     // logical operations
    //     OP_LOGICAL_AND,
    //     OP_LOGICAL_OR,
    //     OP_LOGICAL_NOT,
    //
    //     // branching
    //     OP_IF_NIL,
    //     OP_IF_NOTNIL,
    //     OP_IF_TRUE,
    //     OP_IF_FALSE,
    //     OP_IF_ZERO,
    //     OP_IF_NOTZERO,
    //     OP_IF_GT,
    //     OP_IF_GE,
    //     OP_IF_LT,
    //     OP_IF_LE,
    //     OP_JUMP,
    //
    //     // import assembly
    //     OP_IMPORT,
    //
    //     // procedure invocation
    //     OP_CALL_STATIC,
    //     OP_CALL_VIRTUAL,
    //     OP_CALL_ACTION,
    //     OP_CALL_EXTENSION,
    //     OP_TRAP,
    //     OP_RETURN,
    //
    //     // heap allocation
    //     OP_NEW,
    //
    //     // interpreter services
    //     OP_TYPE_OF,
    //     OP_INTERRUPT,
    //     OP_HALT,
    //
    //     // sentinel, must not be used
    //     LAST_
    // };
    //
    // tempo_utils::LogMessage&& operator<<(tempo_utils::LogMessage&& message, Opcode opcode);
    //
    // /**
    //  * Enumeration of symbol linkage types.
    //  */
    // enum class LinkageSection {
    //     Invalid,
    //     Type,
    //     Existential,
    //     Literal,
    //     Call,
    //     Field,
    //     Static,
    //     Action,
    //     Class,
    //     Struct,
    //     Instance,
    //     Concept,
    //     Generic,
    //     Enum,
    //     Namespace,
    // };

    /**
     * Enumeration of the result of a type comparison.
     */
    enum class TypeComparison {
        DISJOINT,       /** The types are disjoint. */
        SUPER,          /** The first operand is a supertype of the second operand. */
        EQUAL,          /** The types are equal. */
        EXTENDS,        /** The first operand is a subtype of the second operand */
    };

    struct LinkEntry {
        /** Invalid if the link has not been resolved yet, otherwise contains the linkage type. */
        lyric_object::LinkageSection linkage = lyric_object::LinkageSection::Invalid;

        /** context-dependent reference to the target assembly. */
        tu_uint32 assembly = INVALID_ADDRESS_U32;

        /** context-dependent link value */
        tu_uint32 value = INVALID_ADDRESS_U32;
    };
}

#endif // LYRIC_RUNTIME_RUNTIME_TYPES_H