#ifndef LYRIC_SCHEMA_DCMI_ELEMENTS_SCHEMA_H
#define LYRIC_SCHEMA_DCMI_ELEMENTS_SCHEMA_H

#include <array>

#include <tempo_utils/schema.h>

namespace lyric_schema {

    class DcmiElementsNs : public tempo_utils::SchemaNs {
    public:
        constexpr DcmiElementsNs() : tempo_utils::SchemaNs("http://purl.org/dc/elements/1.1/") {};
    };
    constexpr DcmiElementsNs kDcmiElementsNs;

    enum class DcmiElementsId {
        Contributor,   // "An entity responsible for making contributions to the resource".
        Coverage,      // "The spatial or temporal topic of the resource, the spatial applicability of the resource, or the jurisdiction under which the resource is relevant".
        Creator,       // "An entity primarily responsible for making the resource".
        Date,          // "A point or period of time associated with an event in the lifecycle of the resource".
        Description,   // "An account of the resource".
        Format,        // "The file format, physical medium, or dimensions of the resource".
        Identifier,    // "An unambiguous reference to the resource within a given context".
        Language,      // "A language of the resource".
        Publisher,     // "An entity responsible for making the resource available".
        Relation,      // "A related resource".
        Rights,        // "Information about rights held in and over the resource".
        Source,        // "A related resource from which the described resource is derived".
        Subject,       // "The topic of the resource".
        Title,         // "A name given to the resource".
        Type,          // "The nature or genre of the resource".
        NUM_IDS,
    };

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsContributorProperty(
        &kDcmiElementsNs, DcmiElementsId::Contributor, "Contributor", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsCoverageProperty(
        &kDcmiElementsNs, DcmiElementsId::Coverage, "Coverage", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsCreatorProperty(
        &kDcmiElementsNs, DcmiElementsId::Creator, "Creator", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsDateProperty(
        &kDcmiElementsNs, DcmiElementsId::Date, "Date", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsDescriptionProperty(
        &kDcmiElementsNs, DcmiElementsId::Description, "Description", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsFormatProperty(
        &kDcmiElementsNs, DcmiElementsId::Format, "Format", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsIdentifierProperty(
        &kDcmiElementsNs, DcmiElementsId::Identifier, "Identifier", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsLanguageProperty(
        &kDcmiElementsNs, DcmiElementsId::Language, "Language", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsPublisherProperty(
        &kDcmiElementsNs, DcmiElementsId::Publisher, "Publisher", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsRelationProperty(
        &kDcmiElementsNs, DcmiElementsId::Relation, "Relation", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsRightsProperty(
        &kDcmiElementsNs, DcmiElementsId::Rights, "Rights", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsSourceProperty(
        &kDcmiElementsNs, DcmiElementsId::Source, "Source", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsSubjectProperty(
        &kDcmiElementsNs, DcmiElementsId::Subject, "Subject", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsTitleProperty(
        &kDcmiElementsNs, DcmiElementsId::Title, "Title", tempo_utils::PropertyType::kString);

    constexpr tempo_utils::SchemaProperty<DcmiElementsNs,DcmiElementsId>
    kDcmiElementsTypeProperty(
        &kDcmiElementsNs, DcmiElementsId::Type, "Type", tempo_utils::PropertyType::kString);
}

#endif // LYRIC_SCHEMA_DCMI_ELEMENTS_SCHEMA_H