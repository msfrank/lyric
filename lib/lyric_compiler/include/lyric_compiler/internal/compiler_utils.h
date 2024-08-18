#ifndef LYRIC_COMPILER_INTERNAL_COMPILER_UTILS_H
#define LYRIC_COMPILER_INTERNAL_COMPILER_UTILS_H

#include <lyric_object/object_types.h>
#include <lyric_parser/parser_types.h>

namespace lyric_compiler::internal {

    lyric_object::AccessType convert_access_type(lyric_parser::AccessType access);

    lyric_object::DeriveType convert_derive_type(lyric_parser::DeriveType derive);
}

#endif // LYRIC_COMPILER_INTERNAL_COMPILER_UTILS_H
