#ifndef LYRIC_ANALYZER_INTERNAL_ANALYZER_UTILS_H
#define LYRIC_ANALYZER_INTERNAL_ANALYZER_UTILS_H

#include <lyric_object/object_types.h>
#include <lyric_parser/parser_types.h>

namespace lyric_analyzer::internal {

    lyric_object::DeriveType convert_derive_type(lyric_parser::DeriveType derive);
}

#endif // LYRIC_ANALYZER_INTERNAL_ANALYZER_UTILS_H
