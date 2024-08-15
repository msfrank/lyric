
#include <lyric_assembler/internal/write_links.h>
#include <lyric_assembler/internal/write_object.h>

tempo_utils::Status
lyric_assembler::internal::write_links(
    ImportCache *importCache,
    flatbuffers::FlatBufferBuilder &buffer,
    LinksOffset &linksOffset)
{
    TU_ASSERT (importCache != nullptr);

    std::vector<flatbuffers::Offset<lyo1::LinkDescriptor>> links_vector;

    for (auto iterator = importCache->linksBegin(); iterator != importCache->linksEnd(); iterator++) {
        auto &requestedLink = *iterator;

        auto linkLocation = requestedLink.linkUrl.getModuleLocation();
        if (!importCache->hasImport(linkLocation))
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid link {}: no import for link location", requestedLink.linkUrl.toString());

        auto *importHandle = importCache->getImport(linkLocation);

        auto symbolPath = requestedLink.linkUrl.getSymbolPath();
        auto section = internal::linkage_to_descriptor_section(requestedLink.linkType);
        if (section == lyo1::DescriptorSection::Invalid)
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid link {}: could not parse linkage", requestedLink.linkUrl.toString());

        links_vector.push_back(lyo1::CreateLinkDescriptor(buffer,
            buffer.CreateString(symbolPath.toString()),
            section, importHandle->importIndex));
    }

    // create the links vector
    linksOffset = buffer.CreateVector(links_vector);

    return {};
}
