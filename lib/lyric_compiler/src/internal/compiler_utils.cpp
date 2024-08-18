
#include <lyric_compiler/internal/compiler_utils.h>

lyric_object::AccessType
lyric_compiler::internal::convert_access_type(lyric_parser::AccessType access)
{
    switch (access) {
        case lyric_parser::AccessType::Public:
            return lyric_object::AccessType::Public;
        case lyric_parser::AccessType::Protected:
            return lyric_object::AccessType::Protected;
        case lyric_parser::AccessType::Private:
            return lyric_object::AccessType::Private;
        default:
            return lyric_object::AccessType::Invalid;
    }
}
