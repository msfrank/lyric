
#include <lyric_assembler/internal/writer_utils.h>
#include <lyric_assembler/internal/write_links.h>

tempo_utils::Status
lyric_assembler::internal::write_links(
    const std::vector<const RequestedLink> &links,
    const ObjectWriter &writer,
    const absl::flat_hash_map<lyric_common::ModuleLocation,tu_uint32> &importOffsets,
    flatbuffers::FlatBufferBuilder &buffer,
    LinksOffset &linksOffset)
{
    std::vector<flatbuffers::Offset<lyo1::LinkDescriptor>> links_vector;

    for (const auto &requestedLink : links) {

        auto linkLocation = requestedLink.linkUrl.getModuleLocation();
        auto entry = importOffsets.find(linkLocation);
        if (entry == importOffsets.cend())
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid link {}: no import for link location", requestedLink.linkUrl.toString());
        tu_uint32 importOffset = entry->second;

        auto symbolPath = requestedLink.linkUrl.getSymbolPath();
        auto section = internal::linkage_to_descriptor(requestedLink.linkType);
        if (section == lyo1::DescriptorSection::Invalid)
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid link {}: could not parse linkage", requestedLink.linkUrl.toString());

        links_vector.push_back(lyo1::CreateLinkDescriptor(buffer,
            buffer.CreateString(symbolPath.toString()), section, importOffset));
    }

    // create the links vector
    linksOffset = buffer.CreateVector(links_vector);

    return {};
}
