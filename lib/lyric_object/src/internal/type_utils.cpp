
#include <lyric_object/object_result.h>
#include <lyric_object/internal/type_utils.h>

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_object::internal::load_type_symbol(
    std::shared_ptr<const ObjectReader> reader,
    const lyric_common::ModuleLocation &location,
    lyo1::TypeSection section,
    uint32_t descriptor)
{
    switch (section) {
        case lyo1::TypeSection::Existential: {
            if (lyric_object::IS_NEAR(descriptor)) {
                auto symbolPath = reader->getSymbolPath(lyo1::DescriptorSection::Existential, descriptor);
                return lyric_common::SymbolUrl(location, symbolPath);
            }
            auto symbolUrl = reader->getLinkUrl(descriptor & 0x7FFFFFFFu);
            if (!symbolUrl.isValid())
                return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
                    "link not found for descriptor");
            return symbolUrl;
        }
        case lyo1::TypeSection::Class: {
            if (lyric_object::IS_NEAR(descriptor)) {
                auto symbolPath = reader->getSymbolPath(lyo1::DescriptorSection::Class, descriptor);
                return lyric_common::SymbolUrl(location, symbolPath);
            }
            auto symbolUrl = reader->getLinkUrl(descriptor & 0x7FFFFFFFu);
            if (!symbolUrl.isValid())
                return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
                    "link not found for descriptor");
            return symbolUrl;
        }
        case lyo1::TypeSection::Concept: {
            if (lyric_object::IS_NEAR(descriptor)) {
                auto symbolPath = reader->getSymbolPath(lyo1::DescriptorSection::Concept, descriptor);
                return lyric_common::SymbolUrl(location, symbolPath);
            }
            auto symbolUrl = reader->getLinkUrl(descriptor & 0x7FFFFFFFu);
            if (!symbolUrl.isValid())
                return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
                    "link not found for descriptor");
            return symbolUrl;
        }
        case lyo1::TypeSection::Instance: {
            if (lyric_object::IS_NEAR(descriptor)) {
                auto symbolPath = reader->getSymbolPath(lyo1::DescriptorSection::Instance, descriptor);
                return lyric_common::SymbolUrl(location, symbolPath);
            }
            auto symbolUrl = reader->getLinkUrl(descriptor & 0x7FFFFFFFu);
            if (!symbolUrl.isValid())
                return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
                    "link not found for descriptor");
            return symbolUrl;
        }
        case lyo1::TypeSection::Struct: {
            if (lyric_object::IS_NEAR(descriptor)) {
                auto symbolPath = reader->getSymbolPath(lyo1::DescriptorSection::Struct, descriptor);
                return lyric_common::SymbolUrl(location, symbolPath);
            }
            auto symbolUrl = reader->getLinkUrl(descriptor & 0x7FFFFFFFu);
            if (!symbolUrl.isValid())
                return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
                    "link not found for descriptor");
            return symbolUrl;
        }
        case lyo1::TypeSection::Enum: {
            if (lyric_object::IS_NEAR(descriptor)) {
                auto symbolPath = reader->getSymbolPath(lyo1::DescriptorSection::Enum, descriptor);
                return lyric_common::SymbolUrl(location, symbolPath);
            }
            auto symbolUrl = reader->getLinkUrl(descriptor & 0x7FFFFFFFu);
            if (!symbolUrl.isValid())
                return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
                    "link not found for descriptor");
            return symbolUrl;
        }
        case lyo1::TypeSection::Type:
        default:
            return ObjectStatus::forCondition(ObjectCondition::kObjectInvariant,
                "invalid descriptor section");
    }
}
