#ifndef LYRIC_RUNTIME_RUNTIME_TYPES_H
#define LYRIC_RUNTIME_RUNTIME_TYPES_H

#include <lyric_common/module_location.h>
#include <lyric_object/object_types.h>

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

    enum class IntrinsicType {
        Nil,
        Undef,
        Bool,
        Char,
        Bytes,
        String,

        I64,
        I32,
        I16,
        I8,
        U64,
        U32,
        U16,
        U8,
        F64,
        F32,

        Action,
        Binding,
        Call,
        Class,
        Concept,
        Enum,
        Existential,
        Field,
        Instance,
        Namespace,
        Protocol,
        Struct,

        // must be last
        NUM_INTRINSICS,
    };

    lyric_common::SymbolPath intrinsicTypeToSymbolPath(IntrinsicType intrinsic);

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

        /** context-dependent reference to the target object. */
        tu_uint32 object = INVALID_ADDRESS_U32;

        /** context-dependent link value */
        tu_uint32 value = INVALID_ADDRESS_U32;
    };
}

#endif // LYRIC_RUNTIME_RUNTIME_TYPES_H