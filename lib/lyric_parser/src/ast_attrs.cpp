
#include <lyric_parser/ast_attrs.h>
#include <lyric_parser/internal/archetype_reader.h>

lyric_parser::BaseTypeAttr::BaseTypeAttr(const tempo_utils::ComparableResource *resource)
    : tempo_utils::AttrSerde<BaseType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::BaseTypeAttr::writeAttr(tempo_utils::AbstractAttrWriter *writer, const BaseType &binding) const
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
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::BaseType::Octal:
            base = lyric_parser::BaseType::Octal;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::BaseType::Decimal:
            base = lyric_parser::BaseType::Decimal;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::BaseType::Hex:
            base = lyric_parser::BaseType::Hex;
            return tempo_utils::AttrStatus::ok();
        default:
            return tempo_utils::AttrStatus::forCondition(
                tempo_utils::AttrCondition::kConversionError, "invalid base type");
    }
}

tempo_utils::Status
lyric_parser::BaseTypeAttr::parseAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser, BaseType &binding) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_base_type(value, binding);
}

lyric_parser::NotationTypeAttr::NotationTypeAttr(const tempo_utils::ComparableResource *resource)
    : tempo_utils::AttrSerde<NotationType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::NotationTypeAttr::writeAttr(tempo_utils::AbstractAttrWriter *writer, const NotationType &binding) const
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
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::NotationType::Scientific:
            notation = lyric_parser::NotationType::Scientific;
            return tempo_utils::AttrStatus::ok();
        default:
            return tempo_utils::AttrStatus::forCondition(
                tempo_utils::AttrCondition::kConversionError, "invalid notation type");
    }
}

tempo_utils::Status
lyric_parser::NotationTypeAttr::parseAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser, NotationType &notation) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_notation_type(value, notation);
}

lyric_parser::AccessTypeAttr::AccessTypeAttr(const tempo_utils::ComparableResource *resource)
    : tempo_utils::AttrSerde<AccessType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::AccessTypeAttr::writeAttr(tempo_utils::AbstractAttrWriter *writer, const AccessType &access) const
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
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::AccessType::Protected:
            access = lyric_parser::AccessType::Protected;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::AccessType::Public:
            access = lyric_parser::AccessType::Public;
            return tempo_utils::AttrStatus::ok();
        default:
            return tempo_utils::AttrStatus::forCondition(
                tempo_utils::AttrCondition::kConversionError, "invalid access type");
    }
}

tempo_utils::Status
lyric_parser::AccessTypeAttr::parseAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser, AccessType &access) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_access_type(value, access);
}

lyric_parser::BoundTypeAttr::BoundTypeAttr(const tempo_utils::ComparableResource *resource)
    : tempo_utils::AttrSerde<BoundType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::BoundTypeAttr::writeAttr(tempo_utils::AbstractAttrWriter *writer, const BoundType &bound) const
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
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::BoundType::Super:
            bound = lyric_parser::BoundType::Super;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::BoundType::None:
            bound = lyric_parser::BoundType::None;
            return tempo_utils::AttrStatus::ok();
        default:
            return tempo_utils::AttrStatus::forCondition(
                tempo_utils::AttrCondition::kConversionError, "invalid bound type");
    }
}

tempo_utils::Status
lyric_parser::BoundTypeAttr::parseAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser, BoundType &bound) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_bound_type(value, bound);
}

lyric_parser::VarianceTypeAttr::VarianceTypeAttr(const tempo_utils::ComparableResource *resource)
    : tempo_utils::AttrSerde<VarianceType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::VarianceTypeAttr::writeAttr(tempo_utils::AbstractAttrWriter *writer, const VarianceType &variance) const
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
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::VarianceType::Contravariant:
            variance = lyric_parser::VarianceType::Contravariant;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::VarianceType::Invariant:
            variance = lyric_parser::VarianceType::Invariant;
            return tempo_utils::AttrStatus::ok();
        default:
            return tempo_utils::AttrStatus::forCondition(
                tempo_utils::AttrCondition::kConversionError, "invalid variance type");
    }
}

tempo_utils::Status
lyric_parser::VarianceTypeAttr::parseAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser, VarianceType &variance) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_variance_type(value, variance);
}

lyric_parser::NodeAttr::NodeAttr(const tempo_utils::ComparableResource *resource)
    : StatefulAttr(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::NodeAttr::writeAttr(
    tempo_utils::AbstractAttrWriterWithState<ArchetypeState> *writer,
    ArchetypeNode * const &archetypeNode) const
{
    TU_ASSERT (writer != nullptr);
    auto *stateptr = writer->getWriterState();
    auto *resource = tempo_utils::AttrValidator::getResource();
    ArchetypeNamespace *ns;
    TU_ASSIGN_OR_RETURN (ns, stateptr->putNamespace(resource->getNsUrl()));
    AttrId id(ns, resource->getIdValue());
    ArchetypeAttr *attr;
    TU_ASSIGN_OR_RETURN (attr, stateptr->appendAttr(id, AttrValue(archetypeNode)));
    auto *archetypeId = attr->getArchetypeId();
    return writer->putHandle(tempo_utils::AttrHandle{archetypeId->getId()});
}

tempo_utils::Status
lyric_parser::NodeAttr::parseAttr(
    tu_uint32 index,
    tempo_utils::AbstractAttrParserWithState<ArchetypeState> *parser,
    ArchetypeNode * &node) const
{
    TU_ASSERT (parser != nullptr);
    tempo_utils::AttrHandle value;
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
    tempo_utils::AbstractAttrParserWithState<std::shared_ptr<const internal::ArchetypeReader>> *parser,
    NodeWalker &walker) const
{
    TU_ASSERT (parser != nullptr);
    tempo_utils::AttrHandle value;
    TU_RETURN_IF_NOT_OK (parser->getHandle(index, value));
    auto *readerptr = parser->getParserState();
    walker = NodeWalker(*readerptr, value.handle);
    return {};
}

const tempo_utils::StringAttr lyric_parser::kLyricAstLiteralValue(&lyric_schema::kLyricAstLiteralValueProperty);

const lyric_parser::BaseTypeAttr lyric_parser::kLyricAstBaseType(&lyric_schema::kLyricAstBaseEnumProperty);
const lyric_parser::NotationTypeAttr lyric_parser::kLyricAstNotationType(&lyric_schema::kLyricAstNotationEnumProperty);
const lyric_parser::AccessTypeAttr lyric_parser::kLyricAstAccessType(&lyric_schema::kLyricAstAccessEnumProperty);
const lyric_parser::BoundTypeAttr lyric_parser::kLyricAstBoundType(&lyric_schema::kLyricAstBoundEnumProperty);
const lyric_parser::VarianceTypeAttr lyric_parser::kLyricAstVarianceType(&lyric_schema::kLyricAstVarianceEnumProperty);

const lyric_common::ModuleLocationAttr lyric_parser::kLyricAstModuleLocation(&lyric_schema::kLyricAstModuleLocationProperty);
const lyric_common::SymbolPathAttr lyric_parser::kLyricAstSymbolPath(&lyric_schema::kLyricAstSymbolPathProperty);
const tempo_utils::StringAttr lyric_parser::kLyricAstIdentifier(&lyric_schema::kLyricAstIdentifierProperty);
const tempo_utils::StringAttr lyric_parser::kLyricAstLabel(&lyric_schema::kLyricAstLabelProperty);
const tempo_utils::BoolAttr lyric_parser::kLyricAstIsVariable(&lyric_schema::kLyricAstIsVariableProperty);

const lyric_parser::NodeAttr lyric_parser::kLyricAstTypeOffset(&lyric_schema::kLyricAstTypeOffsetProperty);
const lyric_parser::NodeAttr lyric_parser::kLyricAstDefaultOffset(&lyric_schema::kLyricAstDefaultOffsetProperty);
const lyric_parser::NodeAttr lyric_parser::kLyricAstFinallyOffset(&lyric_schema::kLyricAstFinallyOffsetProperty);
const lyric_parser::NodeAttr lyric_parser::kLyricAstRestOffset(&lyric_schema::kLyricAstRestOffsetProperty);
const lyric_parser::NodeAttr lyric_parser::kLyricAstGenericOffset(&lyric_schema::kLyricAstGenericOffsetProperty);
const lyric_parser::NodeAttr lyric_parser::kLyricAstImplementsOffset(&lyric_schema::kLyricAstImplementsOffsetProperty);
const lyric_parser::NodeAttr lyric_parser::kLyricAstTypeArgumentsOffset(&lyric_schema::kLyricAstTypeArgumentsOffsetProperty);
