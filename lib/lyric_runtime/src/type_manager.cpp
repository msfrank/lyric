
#include <lyric_object/concrete_type_walker.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/interpreter_result.h>
#include <lyric_runtime/protocol_ref.h>
#include <lyric_runtime/segment_manager.h>
#include <lyric_runtime/status_ref.h>
#include <lyric_runtime/type_manager.h>

lyric_runtime::TypeManager::TypeManager(
    std::vector<DataCell> &&intrinsiccache,
    SegmentManager *segmentManager)
    : m_intrinsiccache(std::move(intrinsiccache)),
      m_segmentManager(segmentManager)
{
    TU_ASSERT (m_segmentManager != nullptr);
}

tempo_utils::Result<lyric_runtime::DataCell>
lyric_runtime::TypeManager::typeOf(const DataCell &value)
{
    switch (value.type) {
        case DataCellType::BOOL:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Bool)];
        case DataCellType::CHAR32:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Char)];
        case DataCellType::DBL:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Float)];
        case DataCellType::I64:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Int)];
        case DataCellType::NIL:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Nil)];
        case DataCellType::STRING:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::String)];
        case DataCellType::UNDEF:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Undef)];
        case DataCellType::URL:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Url)];
        case DataCellType::REF:
            return value.data.ref->getVirtualTable()->getType();
        case DataCellType::STATUS:
            return value.data.status->getVirtualTable()->getType();
        case DataCellType::PROTOCOL:
            return value.data.protocol->protocolType();

        case DataCellType::DESCRIPTOR: {
            auto section = value.data.descriptor->getLinkageSection();
            switch (section) {
                case lyric_object::LinkageSection::Action:
                    return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Action)];
                case lyric_object::LinkageSection::Call:
                    return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Call)];
                case lyric_object::LinkageSection::Class:
                    return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Class)];
                case lyric_object::LinkageSection::Concept:
                    return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Concept)];
                case lyric_object::LinkageSection::Enum:
                    return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Enum)];
                case lyric_object::LinkageSection::Existential:
                    return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Existential)];
                case lyric_object::LinkageSection::Field:
                    return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Field)];
                case lyric_object::LinkageSection::Instance:
                    return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Instance)];
                case lyric_object::LinkageSection::Namespace:
                    return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Namespace)];
                case lyric_object::LinkageSection::Protocol:
                    return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Protocol)];
                case lyric_object::LinkageSection::Struct:
                    return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Struct)];
                default:
                    return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                        "{} descriptor has no type", lyric_object::linkage_section_to_name(section));
            }
        }

        case DataCellType::TYPE:
            return InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "type descriptor has no type");
        default:
            return InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid type");
    }
}

inline lyric_runtime::InterpreterStatus
resolve_type_to_descriptor(
    const lyric_runtime::DataCell &type,
    lyric_runtime::DataCell &descriptor,
    lyric_runtime::SegmentManager *segmentManager)
{
    TU_ASSERT (type.type == lyric_runtime::DataCellType::TYPE);
    auto *entry = type.data.descriptor;

    // load the type descriptor from the specified assembly
    auto *segment = entry->getSegment();
    if (segment == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid segment");

    auto object = segment->getObject();
    auto descriptorType = object.getType(entry->getDescriptorIndex());
    if (!descriptorType.isValid())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "missing type descriptor");

    // the descriptor is concrete
    if (descriptorType.getTypeDefType() == lyric_common::TypeDefType::Concrete) {
        auto concreteType = descriptorType.concreteType();
        auto concreteSection = concreteType.getLinkageSection();
        auto concreteIndex = concreteType.getLinkageIndex();

        if (concreteSection == lyric_object::LinkageSection::Invalid)
            return lyric_runtime::InterpreterStatus::forCondition(
                lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid type section");

        if (concreteSection == lyric_object::LinkageSection::Type) {
            if (lyric_object::IS_FAR(concreteIndex)) {
                lyric_runtime::InterpreterStatus status;
                auto typeCell = segmentManager->resolveDescriptor(segment, concreteSection, concreteIndex, status);
                if (!typeCell.isValid())
                    return status;
                return resolve_type_to_descriptor(typeCell, descriptor, segmentManager);
            }

            auto typeCell = lyric_runtime::DataCell::forType(segment->lookupType(concreteIndex));
            return resolve_type_to_descriptor(typeCell, descriptor, segmentManager);
        }

        lyric_runtime::InterpreterStatus status;
        descriptor = segmentManager->resolveDescriptor(segment, concreteSection, concreteIndex, status);
        if (!descriptor.isValid())
            return status;
        return {};
    }

    // if the descriptor assignable type is not concrete, return invalid cell
    descriptor = lyric_runtime::DataCell();
    return {};
}

inline lyric_runtime::InterpreterStatus
resolve_super_type(
    const lyric_runtime::DataCell &type,
    lyric_runtime::DataCell &super,
    lyric_runtime::SegmentManager *segmentManager)
{
    TU_ASSERT (type.type == lyric_runtime::DataCellType::TYPE);
    auto *entry = type.data.descriptor;

    auto *segment = entry->getSegment();
    if (segment == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid segment");
    auto object = segment->getObject();

    auto descriptorType = object.getType(entry->getDescriptorIndex());
    if (!descriptorType.isValid())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "missing type descriptor");
    if (!descriptorType.hasSuperType()) {
        super = lyric_runtime::DataCell();
        return {};
    }
    auto superType = descriptorType.getSuperType();

    lyric_runtime::InterpreterStatus status;
    super = segmentManager->resolveDescriptor(segment, lyric_object::LinkageSection::Type,
        superType.getDescriptorOffset(), status);
    if (!super.isValid())
        return status;
    return {};
}

tempo_utils::Result<lyric_runtime::TypeComparison>
lyric_runtime::TypeManager::compareTypes(const DataCell &lhs, const DataCell &rhs)
{
    InterpreterStatus status;
    DataCell ld;
    DataCell rd;
    DataCell superType;

    if (lhs.type != DataCellType::TYPE)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "type comparison has invalid lhs");
    if (rhs.type != DataCellType::TYPE)
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "type comparison has invalid rhs");

    status = resolve_type_to_descriptor(lhs, ld, m_segmentManager);
    if (status.notOk())
        return status;
    if (!ld.isValid())
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid lhs descriptor");

    status = resolve_type_to_descriptor(rhs, rd, m_segmentManager);
    if (status.notOk())
        return status;
    if (!rd.isValid())
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid rhs descriptor");

    // if lhs and rhs are equal, then the comparison result is equal
    if (ld == rd)
        return TypeComparison::EQUAL;

    // if an ancestor of lhs equals rhs, then the comparison result is extends
    status = resolve_super_type(lhs, superType, m_segmentManager);
    if (status.notOk())
        return status;

    while (superType.isValid()) {
        status = resolve_type_to_descriptor(superType, ld, m_segmentManager);
        if (status.notOk())
            return status;
        if (!ld.isValid())
            return InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "type comparison has invalid lhs");
        if (ld == rd)
            return TypeComparison::EXTENDS;
        auto curr = superType;
        status = resolve_super_type(curr, superType, m_segmentManager);
        if (status.notOk())
            return status;
    }

    // otherwise lhs is not a subtype of rhs
    return TypeComparison::DISJOINT;
}
