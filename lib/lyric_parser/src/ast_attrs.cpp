
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/archetype_reader.h>
#include <tempo_schema/schema_result.h>

lyric_parser::BaseTypeAttr::BaseTypeAttr(const tempo_schema::ComparableResource *resource)
    : tempo_schema::AttrSerde<BaseType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::BaseTypeAttr::writeAttr(tempo_schema::AbstractAttrWriter *writer, const BaseType &binding) const
{
    TU_ASSERT (writer != nullptr);
    return writer->putUInt32(static_cast<tu_uint32>(binding));
}

static tempo_utils::Status
value_to_base_type(tu_uint32 value, lyric_parser::BaseType &base)
{
    switch (static_cast<lyric_parser::BaseType>(value)) {
        case lyric_parser::BaseType::Binary:
            base = lyric_parser::BaseType::Binary;
            return {};
        case lyric_parser::BaseType::Octal:
            base = lyric_parser::BaseType::Octal;
            return {};
        case lyric_parser::BaseType::Decimal:
            base = lyric_parser::BaseType::Decimal;
            return {};
        case lyric_parser::BaseType::Hex:
            base = lyric_parser::BaseType::Hex;
            return {};
        default:
            return tempo_schema::SchemaStatus::forCondition(
                tempo_schema::SchemaCondition::kConversionError, "invalid base type");
    }
}

tempo_utils::Status
lyric_parser::BaseTypeAttr::parseAttr(tu_uint32 index, tempo_schema::AbstractAttrParser *parser, BaseType &binding) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_base_type(value, binding);
}

lyric_parser::NotationTypeAttr::NotationTypeAttr(const tempo_schema::ComparableResource *resource)
    : tempo_schema::AttrSerde<NotationType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::NotationTypeAttr::writeAttr(tempo_schema::AbstractAttrWriter *writer, const NotationType &binding) const
{
    TU_ASSERT (writer != nullptr);
    return writer->putUInt32(static_cast<tu_uint32>(binding));
}

static tempo_utils::Status
value_to_notation_type(tu_int64 value, lyric_parser::NotationType &notation)
{
    switch (static_cast<lyric_parser::NotationType>(value)) {
        case lyric_parser::NotationType::Fixed:
            notation = lyric_parser::NotationType::Fixed;
            return {};
        case lyric_parser::NotationType::Scientific:
            notation = lyric_parser::NotationType::Scientific;
            return {};
        default:
            return tempo_schema::SchemaStatus::forCondition(
                tempo_schema::SchemaCondition::kConversionError, "invalid notation type");
    }
}

tempo_utils::Status
lyric_parser::NotationTypeAttr::parseAttr(tu_uint32 index, tempo_schema::AbstractAttrParser *parser, NotationType &notation) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_notation_type(value, notation);
}

lyric_parser::AccessTypeAttr::AccessTypeAttr(const tempo_schema::ComparableResource *resource)
    : tempo_schema::AttrSerde<AccessType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::AccessTypeAttr::writeAttr(tempo_schema::AbstractAttrWriter *writer, const AccessType &access) const
{
    TU_ASSERT (writer != nullptr);
    return writer->putUInt32(static_cast<tu_uint32>(access));
}

static tempo_utils::Status
value_to_access_type(tu_int64 value, lyric_parser::AccessType &access)
{
    switch (static_cast<lyric_parser::AccessType>(value)) {
        case lyric_parser::AccessType::Private:
            access = lyric_parser::AccessType::Private;
            return {};
        case lyric_parser::AccessType::Protected:
            access = lyric_parser::AccessType::Protected;
            return {};
        case lyric_parser::AccessType::Public:
            access = lyric_parser::AccessType::Public;
            return {};
        default:
            return tempo_schema::SchemaStatus::forCondition(
                tempo_schema::SchemaCondition::kConversionError, "invalid access type");
    }
}

tempo_utils::Status
lyric_parser::AccessTypeAttr::parseAttr(tu_uint32 index, tempo_schema::AbstractAttrParser *parser, AccessType &access) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_access_type(value, access);
}

lyric_parser::BoundTypeAttr::BoundTypeAttr(const tempo_schema::ComparableResource *resource)
    : tempo_schema::AttrSerde<BoundType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::BoundTypeAttr::writeAttr(tempo_schema::AbstractAttrWriter *writer, const BoundType &bound) const
{
    TU_ASSERT (writer != nullptr);
    return writer->putUInt32(static_cast<tu_uint32>(bound));
}

static tempo_utils::Status
value_to_bound_type(tu_int64 value, lyric_parser::BoundType &bound)
{
    switch (static_cast<lyric_parser::BoundType>(value)) {
        case lyric_parser::BoundType::Extends:
            bound = lyric_parser::BoundType::Extends;
            return {};
        case lyric_parser::BoundType::Super:
            bound = lyric_parser::BoundType::Super;
            return {};
        case lyric_parser::BoundType::None:
            bound = lyric_parser::BoundType::None;
            return {};
        default:
            return tempo_schema::SchemaStatus::forCondition(
                tempo_schema::SchemaCondition::kConversionError, "invalid bound type");
    }
}

tempo_utils::Status
lyric_parser::BoundTypeAttr::parseAttr(tu_uint32 index, tempo_schema::AbstractAttrParser *parser, BoundType &bound) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_bound_type(value, bound);
}

lyric_parser::VarianceTypeAttr::VarianceTypeAttr(const tempo_schema::ComparableResource *resource)
    : tempo_schema::AttrSerde<VarianceType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::VarianceTypeAttr::writeAttr(tempo_schema::AbstractAttrWriter *writer, const VarianceType &variance) const
{
    TU_ASSERT (writer != nullptr);
    return writer->putUInt32(static_cast<tu_uint32>(variance));
}

static tempo_utils::Status
value_to_variance_type(tu_int64 value, lyric_parser::VarianceType &variance)
{
    switch (static_cast<lyric_parser::VarianceType>(value)) {
        case lyric_parser::VarianceType::Covariant:
            variance = lyric_parser::VarianceType::Covariant;
            return {};
        case lyric_parser::VarianceType::Contravariant:
            variance = lyric_parser::VarianceType::Contravariant;
            return {};
        case lyric_parser::VarianceType::Invariant:
            variance = lyric_parser::VarianceType::Invariant;
            return {};
        default:
            return tempo_schema::SchemaStatus::forCondition(
                tempo_schema::SchemaCondition::kConversionError, "invalid variance type");
    }
}

tempo_utils::Status
lyric_parser::VarianceTypeAttr::parseAttr(tu_uint32 index, tempo_schema::AbstractAttrParser *parser, VarianceType &variance) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_variance_type(value, variance);
}

lyric_parser::DeriveTypeAttr::DeriveTypeAttr(const tempo_schema::ComparableResource *resource)
    : tempo_schema::AttrSerde<DeriveType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::DeriveTypeAttr::writeAttr(tempo_schema::AbstractAttrWriter *writer, const DeriveType &derive) const
{
    TU_ASSERT (writer != nullptr);
    return writer->putUInt32(static_cast<tu_uint32>(derive));
}

static tempo_utils::Status
value_to_derive_type(tu_int64 value, lyric_parser::DeriveType &derive)
{
    switch (static_cast<lyric_parser::DeriveType>(value)) {
        case lyric_parser::DeriveType::Any:
            derive = lyric_parser::DeriveType::Any;
            return {};
        case lyric_parser::DeriveType::Sealed:
            derive = lyric_parser::DeriveType::Sealed;
            return {};
        case lyric_parser::DeriveType::Final:
            derive = lyric_parser::DeriveType::Final;
            return {};
        default:
            return tempo_schema::SchemaStatus::forCondition(
                tempo_schema::SchemaCondition::kConversionError, "invalid derive type");
    }
}

tempo_utils::Status
lyric_parser::DeriveTypeAttr::parseAttr(tu_uint32 index, tempo_schema::AbstractAttrParser *parser, DeriveType &derive) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_derive_type(value, derive);
}

lyric_parser::NodeAttr::NodeAttr(const tempo_schema::ComparableResource *resource)
    : StatefulAttr(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::NodeAttr::writeAttr(
    tempo_schema::AbstractAttrWriterWithState<ArchetypeState> *writer,
    ArchetypeNode * const &archetypeNode) const
{
    TU_ASSERT (writer != nullptr);
    auto *stateptr = writer->getWriterState();
    auto *resource = tempo_schema::AttrValidator::getResource();
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, stateptr->putNamespace(resource->getNsUrl()));
    AttrId id(ns, resource->getIdValue());
    ArchetypeAttr *attr;
    TU_ASSIGN_OR_RETURN (attr, stateptr->appendAttr(id, AttrValue(archetypeNode)));
    auto *archetypeId = attr->getArchetypeId();
    return writer->putHandle(tempo_schema::AttrHandle{archetypeId->getId()});
}

tempo_utils::Status
lyric_parser::NodeAttr::parseAttr(
    tu_uint32 index,
    tempo_schema::AbstractAttrParserWithState<ArchetypeState> *parser,
    ArchetypeNode * &node) const
{
    TU_ASSERT (parser != nullptr);
    tempo_schema::AttrHandle value;
    TU_RETURN_IF_NOT_OK (parser->getHandle(index, value));
    auto *readerptr = parser->getParserState();
    auto *archetypeId = readerptr->getId(value.handle);
    TU_ASSERT (archetypeId != nullptr);
    TU_ASSERT (archetypeId->getType() == ArchetypeDescriptorType::Node);
    node = readerptr->getNode(archetypeId->getOffset());
    return {};
}

tempo_utils::Status
lyric_parser::NodeAttr::parseAttr(
    tu_uint32 index,
    tempo_schema::AbstractAttrParserWithState<std::shared_ptr<const internal::ArchetypeReader>> *parser,
    NodeWalker &walker) const
{
    TU_ASSERT (parser != nullptr);
    tempo_schema::AttrHandle value;
    TU_RETURN_IF_NOT_OK (parser->getHandle(index, value));
    auto *readerptr = parser->getParserState();
    walker = NodeWalker(*readerptr, value.handle);
    return {};
}

const tempo_schema::StringAttr lyric_parser::kLyricAstLiteralValue(&lyric_schema::kLyricAstLiteralValueProperty);

const lyric_parser::BaseTypeAttr lyric_parser::kLyricAstBaseType(&lyric_schema::kLyricAstBaseEnumProperty);
const lyric_parser::NotationTypeAttr lyric_parser::kLyricAstNotationType(&lyric_schema::kLyricAstNotationEnumProperty);
const lyric_parser::AccessTypeAttr lyric_parser::kLyricAstAccessType(&lyric_schema::kLyricAstAccessEnumProperty);
const lyric_parser::BoundTypeAttr lyric_parser::kLyricAstBoundType(&lyric_schema::kLyricAstBoundEnumProperty);
const lyric_parser::VarianceTypeAttr lyric_parser::kLyricAstVarianceType(&lyric_schema::kLyricAstVarianceEnumProperty);
const lyric_parser::DeriveTypeAttr lyric_parser::kLyricAstDeriveType(&lyric_schema::kLyricAstDeriveEnumProperty);

const tempo_schema::UrlAttr lyric_parser::kLyricAstImportLocation(&lyric_schema::kLyricAstImportLocationProperty);
const lyric_common::ModuleLocationAttr lyric_parser::kLyricAstModuleLocation(&lyric_schema::kLyricAstModuleLocationProperty);
const lyric_common::SymbolPathAttr lyric_parser::kLyricAstSymbolPath(&lyric_schema::kLyricAstSymbolPathProperty);
const tempo_schema::StringAttr lyric_parser::kLyricAstIdentifier(&lyric_schema::kLyricAstIdentifierProperty);
const tempo_schema::StringAttr lyric_parser::kLyricAstLabel(&lyric_schema::kLyricAstLabelProperty);
const tempo_schema::BoolAttr lyric_parser::kLyricAstIsVariable(&lyric_schema::kLyricAstIsVariableProperty);

const lyric_parser::NodeAttr lyric_parser::kLyricAstTypeOffset(&lyric_schema::kLyricAstTypeOffsetProperty);
const lyric_parser::NodeAttr lyric_parser::kLyricAstDefaultOffset(&lyric_schema::kLyricAstDefaultOffsetProperty);
const lyric_parser::NodeAttr lyric_parser::kLyricAstFinallyOffset(&lyric_schema::kLyricAstFinallyOffsetProperty);
const lyric_parser::NodeAttr lyric_parser::kLyricAstRestOffset(&lyric_schema::kLyricAstRestOffsetProperty);
const lyric_parser::NodeAttr lyric_parser::kLyricAstGenericOffset(&lyric_schema::kLyricAstGenericOffsetProperty);
const lyric_parser::NodeAttr lyric_parser::kLyricAstTypeArgumentsOffset(&lyric_schema::kLyricAstTypeArgumentsOffsetProperty);
const lyric_parser::NodeAttr lyric_parser::kLyricAstMacroListOffset(&lyric_schema::kLyricAstMacroListOffsetProperty);
