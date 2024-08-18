
#include <lyric_analyzer/internal/analyzer_utils.h>

lyric_object::AccessType
lyric_analyzer::internal::convert_access_type(lyric_parser::AccessType access)
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

lyric_object::DeriveType
lyric_analyzer::internal::convert_derive_type(lyric_parser::DeriveType derive)
{
    switch (derive) {
        case lyric_parser::DeriveType::Any:
            return lyric_object::DeriveType::Any;
        case lyric_parser::DeriveType::Sealed:
            return lyric_object::DeriveType::Sealed;
        case lyric_parser::DeriveType::Final:
            return lyric_object::DeriveType::Final;
        default:
            return lyric_object::DeriveType::Invalid;
    }
}
