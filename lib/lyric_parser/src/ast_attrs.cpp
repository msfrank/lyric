
#include <lyric_parser/ast_attrs.h>

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
        case lyric_parser::BaseType::BINARY:
            base = lyric_parser::BaseType::BINARY;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::BaseType::OCTAL:
            base = lyric_parser::BaseType::OCTAL;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::BaseType::DECIMAL:
            base = lyric_parser::BaseType::DECIMAL;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::BaseType::HEX:
            base = lyric_parser::BaseType::HEX;
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

tempo_utils::Status
lyric_parser::BaseTypeAttr::validateAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    BaseType base;
    return value_to_base_type(value, base);
}

std::string
lyric_parser::BaseTypeAttr::toString(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return "???";

    BaseType base;
    status = value_to_base_type(value, base);
    if (status.notOk())
        return "???";

    switch (base) {
        case BaseType::BINARY:
            return "BINARY";
        case BaseType::OCTAL:
            return "OCTAL";
        case BaseType::DECIMAL:
            return "DECIMAL";
        case BaseType::HEX:
            return "HEX";
        default:
            return "???";
    }
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
        case lyric_parser::NotationType::FIXED:
            notation = lyric_parser::NotationType::FIXED;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::NotationType::SCIENTIFIC:
            notation = lyric_parser::NotationType::SCIENTIFIC;
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

tempo_utils::Status
lyric_parser::NotationTypeAttr::validateAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    NotationType notation;
    return value_to_notation_type(value, notation);
}

std::string
lyric_parser::NotationTypeAttr::toString(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return "???";

    NotationType notation;
    status = value_to_notation_type(value, notation);
    if (status.notOk())
        return "???";

    switch (notation) {
        case NotationType::FIXED:
            return "VALUE";
        case NotationType::SCIENTIFIC:
            return "SCIENTIFIC";
        default:
            return "???";
    }
}

lyric_parser::BindingTypeAttr::BindingTypeAttr(const tempo_utils::ComparableResource *resource)
    : tempo_utils::AttrSerde<BindingType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::BindingTypeAttr::writeAttr(tempo_utils::AbstractAttrWriter *writer, const BindingType &binding) const
{
    TU_ASSERT (writer != nullptr);
    return writer->putUInt32(static_cast<tu_uint32>(binding));
}

static tempo_utils::Status
value_to_binding_type(tu_int64 value, lyric_parser::BindingType &binding)
{
    switch (static_cast<lyric_parser::BindingType>(value)) {
        case lyric_parser::BindingType::DESCRIPTOR:
            binding = lyric_parser::BindingType::DESCRIPTOR;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::BindingType::VALUE:
            binding = lyric_parser::BindingType::VALUE;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::BindingType::VARIABLE:
            binding = lyric_parser::BindingType::VARIABLE;
            return tempo_utils::AttrStatus::ok();
        default:
            return tempo_utils::AttrStatus::forCondition(
                tempo_utils::AttrCondition::kConversionError, "invalid binding type");
    }
}

tempo_utils::Status
lyric_parser::BindingTypeAttr::parseAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser, BindingType &binding) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_binding_type(value, binding);
}

tempo_utils::Status
lyric_parser::BindingTypeAttr::validateAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    BindingType binding;
    return value_to_binding_type(value, binding);
}

std::string
lyric_parser::BindingTypeAttr::toString(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return "???";

    BindingType binding;
    status = value_to_binding_type(value, binding);
    if (status.notOk())
        return "???";

    switch (binding) {
        case BindingType::DESCRIPTOR:
            return "DESCRIPTOR";
        case BindingType::VALUE:
            return "VALUE";
        case BindingType::VARIABLE:
            return "VARIABLE";
        default:
            return "???";
    }
}

lyric_parser::MutationTypeAttr::MutationTypeAttr(const tempo_utils::ComparableResource *resource)
    : tempo_utils::AttrSerde<MutationType>(resource)
{
}

tempo_utils::Result<tu_uint32>
lyric_parser::MutationTypeAttr::writeAttr(tempo_utils::AbstractAttrWriter *writer, const MutationType &mutation) const
{
    TU_ASSERT (writer != nullptr);
    return writer->putUInt32(static_cast<tu_uint32>(mutation));
}

static tempo_utils::Status
value_to_mutation_type(tu_int64 value, lyric_parser::MutationType &mutation)
{
    switch (static_cast<lyric_parser::MutationType>(value)) {
        case lyric_parser::MutationType::IMMUTABLE:
            mutation = lyric_parser::MutationType::IMMUTABLE;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::MutationType::MUTABLE:
            mutation = lyric_parser::MutationType::MUTABLE;
            return tempo_utils::AttrStatus::ok();
        default:
            return tempo_utils::AttrStatus::forCondition(
                tempo_utils::AttrCondition::kConversionError, "invalid mutation type");
    }
}

tempo_utils::Status
lyric_parser::MutationTypeAttr::parseAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser, MutationType &mutation) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    return value_to_mutation_type(value, mutation);
}

tempo_utils::Status
lyric_parser::MutationTypeAttr::validateAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    MutationType mutation;
    return value_to_mutation_type(value, mutation);
}

std::string
lyric_parser::MutationTypeAttr::toString(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return "???";

    MutationType mutation;
    status = value_to_mutation_type(value, mutation);
    if (status.notOk())
        return "???";

    switch (mutation) {
        case MutationType::IMMUTABLE:
            return "IMMUTABLE";
        case MutationType::MUTABLE:
            return "MUTABLE";
        default:
            return "???";
    }
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
        case lyric_parser::AccessType::PRIVATE:
            access = lyric_parser::AccessType::PRIVATE;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::AccessType::PROTECTED:
            access = lyric_parser::AccessType::PROTECTED;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::AccessType::PUBLIC:
            access = lyric_parser::AccessType::PUBLIC;
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

tempo_utils::Status
lyric_parser::AccessTypeAttr::validateAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    AccessType access;
    return value_to_access_type(value, access);
}

std::string
lyric_parser::AccessTypeAttr::toString(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return "???";

    AccessType access;
    status = value_to_access_type(value, access);
    if (status.notOk())
        return "???";

    switch (access) {
        case AccessType::PRIVATE:
            return "PRIVATE";
        case AccessType::PROTECTED:
            return "PROTECTED";
        case AccessType::PUBLIC:
            return "PUBLIC";
        default:
            return "???";
    }
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
        case lyric_parser::BoundType::EXTENDS:
            bound = lyric_parser::BoundType::EXTENDS;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::BoundType::SUPER:
            bound = lyric_parser::BoundType::SUPER;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::BoundType::NONE:
            bound = lyric_parser::BoundType::NONE;
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

tempo_utils::Status
lyric_parser::BoundTypeAttr::validateAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    BoundType bound;
    return value_to_bound_type(value, bound);
}

std::string
lyric_parser::BoundTypeAttr::toString(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return "???";

    BoundType bound;
    status = value_to_bound_type(value, bound);
    if (status.notOk())
        return "???";

    switch (bound) {
        case BoundType::EXTENDS:
            return "EXTENDS";
        case BoundType::SUPER:
            return "SUPER";
        case BoundType::NONE:
            return "NONE";
        default:
            return "???";
    }
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
        case lyric_parser::VarianceType::COVARIANT:
            variance = lyric_parser::VarianceType::COVARIANT;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::VarianceType::CONTRAVARIANT:
            variance = lyric_parser::VarianceType::CONTRAVARIANT;
            return tempo_utils::AttrStatus::ok();
        case lyric_parser::VarianceType::INVARIANT:
            variance = lyric_parser::VarianceType::INVARIANT;
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

tempo_utils::Status
lyric_parser::VarianceTypeAttr::validateAttr(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return status;
    VarianceType variance;
    return value_to_variance_type(value, variance);
}

std::string
lyric_parser::VarianceTypeAttr::toString(tu_uint32 index, tempo_utils::AbstractAttrParser *parser) const
{
    tu_uint32 value;
    auto status = parser->getUInt32(index, value);
    if (status.notOk())
        return "???";

    VarianceType variance;
    status = value_to_variance_type(value, variance);
    if (status.notOk())
        return "???";

    switch (variance) {
        case VarianceType::COVARIANT:
            return "COVARIANT";
        case VarianceType::CONTRAVARIANT:
            return "CONTRAVARIANT";
        case VarianceType::INVARIANT:
            return "INVARIANT";
        default:
            return "???";
    }
}

const tempo_utils::StringAttr lyric_parser::kLyricAstLiteralValue(&lyric_schema::kLyricAstLiteralValueProperty);

const lyric_parser::BaseTypeAttr lyric_parser::kLyricAstBaseType(&lyric_schema::kLyricAstBaseEnumProperty);
const lyric_parser::NotationTypeAttr lyric_parser::kLyricAstNotationType(&lyric_schema::kLyricAstNotationEnumProperty);
const lyric_parser::BindingTypeAttr lyric_parser::kLyricAstBindingType(&lyric_schema::kLyricAstBindingEnumProperty);
const lyric_parser::MutationTypeAttr lyric_parser::kLyricAstMutationType(&lyric_schema::kLyricAstMutationEnumProperty);
const lyric_parser::AccessTypeAttr lyric_parser::kLyricAstAccessType(&lyric_schema::kLyricAstAccessEnumProperty);
const lyric_parser::BoundTypeAttr lyric_parser::kLyricAstBoundType(&lyric_schema::kLyricAstBoundEnumProperty);
const lyric_parser::VarianceTypeAttr lyric_parser::kLyricAstVarianceType(&lyric_schema::kLyricAstVarianceEnumProperty);

const lyric_common::AssemblyLocationAttr lyric_parser::kLyricAstAssemblyLocation(&lyric_schema::kLyricAstAssemblyLocationProperty);
const lyric_common::SymbolPathAttr lyric_parser::kLyricAstSymbolPath(&lyric_schema::kLyricAstSymbolPathProperty);
const tempo_utils::StringAttr lyric_parser::kLyricAstIdentifier(&lyric_schema::kLyricAstIdentifierProperty);
const tempo_utils::StringAttr lyric_parser::kLyricAstLabel(&lyric_schema::kLyricAstLabelProperty);

const tempo_utils::UInt32Attr lyric_parser::kLyricAstTypeOffset(&lyric_schema::kLyricAstTypeOffsetProperty);
const tempo_utils::UInt32Attr lyric_parser::kLyricAstDefaultOffset(&lyric_schema::kLyricAstDefaultOffsetProperty);
const tempo_utils::UInt32Attr lyric_parser::kLyricAstFinallyOffset(&lyric_schema::kLyricAstFinallyOffsetProperty);
const tempo_utils::UInt32Attr lyric_parser::kLyricAstRestOffset(&lyric_schema::kLyricAstRestOffsetProperty);
const tempo_utils::UInt32Attr lyric_parser::kLyricAstGenericOffset(&lyric_schema::kLyricAstGenericOffsetProperty);
const tempo_utils::UInt32Attr lyric_parser::kLyricAstImplementsOffset(&lyric_schema::kLyricAstImplementsOffsetProperty);
const tempo_utils::UInt32Attr lyric_parser::kLyricAstTypeArgumentsOffset(&lyric_schema::kLyricAstTypeArgumentsOffsetProperty);
