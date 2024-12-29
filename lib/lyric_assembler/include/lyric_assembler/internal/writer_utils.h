#ifndef LYRIC_ASSEMBLER_INTERNAL_WRITER_UTILS_H
#define LYRIC_ASSEMBLER_INTERNAL_WRITER_UTILS_H

#include <lyric_object/generated/object.h>
#include <lyric_object/object_types.h>

namespace lyric_assembler::internal {

    inline lyo1::DescriptorSection
    linkage_to_descriptor(lyric_object::LinkageSection linkage)
    {
        switch (linkage) {
            case lyric_object::LinkageSection::Action:
                return lyo1::DescriptorSection::Action;
            case lyric_object::LinkageSection::Binding:
                return lyo1::DescriptorSection::Binding;
            case lyric_object::LinkageSection::Call:
                return lyo1::DescriptorSection::Call;
            case lyric_object::LinkageSection::Class:
                return lyo1::DescriptorSection::Class;
            case lyric_object::LinkageSection::Concept:
                return lyo1::DescriptorSection::Concept;
            case lyric_object::LinkageSection::Enum:
                return lyo1::DescriptorSection::Enum;
            case lyric_object::LinkageSection::Existential:
                return lyo1::DescriptorSection::Existential;
            case lyric_object::LinkageSection::Field:
                return lyo1::DescriptorSection::Field;
            case lyric_object::LinkageSection::Instance:
                return lyo1::DescriptorSection::Instance;
            case lyric_object::LinkageSection::Literal:
                return lyo1::DescriptorSection::Literal;
            case lyric_object::LinkageSection::Namespace:
                return lyo1::DescriptorSection::Namespace;
            case lyric_object::LinkageSection::Static:
                return lyo1::DescriptorSection::Static;
            case lyric_object::LinkageSection::Struct:
                return lyo1::DescriptorSection::Struct;
            default:
                return lyo1::DescriptorSection::Invalid;
        }
    }
}

#endif // LYRIC_ASSEMBLER_INTERNAL_WRITER_UTILS_H
