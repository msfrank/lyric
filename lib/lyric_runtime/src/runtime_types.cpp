
#include <lyric_runtime/runtime_types.h>

lyric_common::SymbolPath
lyric_runtime::intrinsicTypeToSymbolPath(IntrinsicType intrinsic)
{
    switch (intrinsic) {
        case IntrinsicType::Nil:
            return lyric_common::SymbolPath::fromString("Nil");
        case IntrinsicType::Undef:
            return lyric_common::SymbolPath::fromString("Undef");
        case IntrinsicType::Bool:
            return lyric_common::SymbolPath::fromString("Bool");
        case IntrinsicType::Int64:
            return lyric_common::SymbolPath::fromString("Int");
        case IntrinsicType::Float64:
            return lyric_common::SymbolPath::fromString("Float");
        case IntrinsicType::Char32:
            return lyric_common::SymbolPath::fromString("Char");
        case IntrinsicType::Bytes:
            return lyric_common::SymbolPath::fromString("Bytes");
        case IntrinsicType::String:
            return lyric_common::SymbolPath::fromString("String");

        case IntrinsicType::Action:
            return lyric_common::SymbolPath::fromString("Action");
        case IntrinsicType::Binding:
            return lyric_common::SymbolPath::fromString("Binding");
        case IntrinsicType::Call:
            return lyric_common::SymbolPath::fromString("Call");
        case IntrinsicType::Class:
            return lyric_common::SymbolPath::fromString("Class");
        case IntrinsicType::Concept:
            return lyric_common::SymbolPath::fromString("Concept");
        case IntrinsicType::Enum:
            return lyric_common::SymbolPath::fromString("Enum");
        case IntrinsicType::Existential:
            return lyric_common::SymbolPath::fromString("Existential");
        case IntrinsicType::Field:
            return lyric_common::SymbolPath::fromString("Field");
        case IntrinsicType::Instance:
            return lyric_common::SymbolPath::fromString("Instance");
        case IntrinsicType::Namespace:
            return lyric_common::SymbolPath::fromString("Namespace");
        case IntrinsicType::Protocol:
            return lyric_common::SymbolPath::fromString("Protocol");
        case IntrinsicType::Struct:
            return lyric_common::SymbolPath::fromString("Struct");

        case IntrinsicType::NUM_INTRINSICS:
        default:
            return {};
    }
}
