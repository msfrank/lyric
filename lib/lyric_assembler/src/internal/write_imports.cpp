
#include <lyric_assembler/internal/write_imports.h>

tempo_utils::Status
lyric_assembler::internal::write_imports(
    const std::vector<const ImportHandle *> &imports,
    const ObjectWriter &writer,
    flatbuffers::FlatBufferBuilder &buffer,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<lyo1::ImportDescriptor>>> &importsOffset)
{
    std::vector<flatbuffers::Offset<lyo1::ImportDescriptor>> imports_vector;

    // serialize array of import descriptors
    for (const auto *importHandle : imports) {
        const auto &importLocation = importHandle->location;

        lyo1::ImportFlags flags;
        switch (importHandle->flags) {
            case ImportFlags::ExactLinkage:
                flags = lyo1::ImportFlags::ExactLinkage;
                break;
            case ImportFlags::ApiLinkage:
                flags = lyo1::ImportFlags::ApiLinkage;
                break;
            case ImportFlags::SystemBootstrap:
                flags = lyo1::ImportFlags::SystemBootstrap;
                break;
            default:
                return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                    "invalid import {}: could not parse flags", importLocation.toString());
        }
        imports_vector.push_back(lyo1::CreateImportDescriptor(buffer,
            buffer.CreateString(importLocation.toString()), lyo1::HashType::None, 0, flags));
    }

    // create the imports vector
    importsOffset = buffer.CreateVector(imports_vector);

    return {};
}