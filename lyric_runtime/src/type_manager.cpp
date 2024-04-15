
#include <lyric_object/concrete_type_walker.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/internal/assembly_reader.h>
#include <lyric_runtime/interpreter_result.h>
#include <lyric_runtime/segment_manager.h>
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
        case DataCellType::ACTION:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Action)];
        case DataCellType::BOOL:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Bool)];
        case DataCellType::CALL:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Call)];
        case DataCellType::CHAR32:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Char)];
        case DataCellType::CLASS:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Class)];
        case DataCellType::CONCEPT:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Concept)];
        case DataCellType::DBL:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Float)];
        case DataCellType::ENUM:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Enum)];
        case DataCellType::EXISTENTIAL:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Existential)];
        case DataCellType::FIELD:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Field)];
        case DataCellType::INSTANCE:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Instance)];
        case DataCellType::I64:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Int)];
        case DataCellType::NAMESPACE:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Namespace)];
        case DataCellType::NIL:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Nil)];
        case DataCellType::STRING:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::String)];
        case DataCellType::STRUCT:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Struct)];
        case DataCellType::URL:
            return m_intrinsiccache[static_cast<int>(lyric_object::IntrinsicType::Url)];
        case DataCellType::REF:
            return value.data.ref->getVirtualTable()->getType();
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
    TU_ASSERT (type.data.descriptor.assembly != lyric_runtime::INVALID_ADDRESS_U32);
    TU_ASSERT (type.data.descriptor.value != lyric_runtime::INVALID_ADDRESS_U32);

    // load the type descriptor from the specified assembly
    auto *segment = segmentManager->getSegment(type.data.descriptor.assembly);
    if (segment == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid segment");

    auto object = segment->getObject().getObject();
    auto descriptorType = object.getType(type.data.descriptor.value);
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

            auto typeCell = lyric_runtime::DataCell::forType(type.data.descriptor.assembly, concreteIndex);
            return resolve_type_to_descriptor(typeCell, descriptor, segmentManager);
        }

        lyric_runtime::InterpreterStatus status;
        descriptor = segmentManager->resolveDescriptor(segment, concreteSection, concreteIndex, status);
        if (!descriptor.isValid())
            return status;
        return lyric_runtime::InterpreterStatus::ok();
    }

    // if the descriptor assignable type is not concrete, return invalid cell
    descriptor = lyric_runtime::DataCell();
    return lyric_runtime::InterpreterStatus::ok();
}

inline lyric_runtime::InterpreterStatus
resolve_super_type(
    const lyric_runtime::DataCell &type,
    lyric_runtime::DataCell &super,
    lyric_runtime::SegmentManager *segmentManager)
{
    TU_ASSERT (type.type == lyric_runtime::DataCellType::TYPE);

    auto *segment = segmentManager->getSegment(type.data.descriptor.assembly);
    if (segment == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid segment");
    auto object = segment->getObject().getObject();

    auto descriptorType = object.getType(type.data.descriptor.value);
    if (!descriptorType.isValid())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "missing type descriptor");
    if (!descriptorType.hasSuperType()) {
        super = lyric_runtime::DataCell();
        return lyric_runtime::InterpreterStatus::ok();
    }
    auto superType = descriptorType.getSuperType();

    lyric_runtime::InterpreterStatus status;
    super = segmentManager->resolveDescriptor(segment, lyric_object::LinkageSection::Type,
        superType.getDescriptorOffset(), status);
    if (!super.isValid())
        return status;
    return lyric_runtime::InterpreterStatus::ok();
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
