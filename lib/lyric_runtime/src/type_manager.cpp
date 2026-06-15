
#include <lyric_object/concrete_type_walker.h>
#include <lyric_runtime/base_ref.h>
#include <lyric_runtime/interpreter_result.h>
#include <lyric_runtime/protocol_ref.h>
#include <lyric_runtime/segment_manager.h>
#include <lyric_runtime/status_ref.h>
#include <lyric_runtime/type_manager.h>

lyric_runtime::TypeManager::TypeManager(
    std::vector<Operand> &&intrinsiccache,
    SegmentManager *segmentManager)
    : m_intrinsiccache(std::move(intrinsiccache)),
      m_segmentManager(segmentManager)
{
    TU_ASSERT (m_segmentManager != nullptr);
}

tempo_utils::Result<lyric_runtime::Operand>
lyric_runtime::TypeManager::typeOf(const Operand &value)
{
    switch (value.getType()) {
        case OperandType::Nil:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::Nil)];
        case OperandType::Undef:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::Undef)];
        case OperandType::Bool:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::Bool)];
        case OperandType::Int64:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::I64)];
        case OperandType::Int32:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::I32)];
        case OperandType::Int16:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::I16)];
        case OperandType::Int8:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::I8)];
        case OperandType::UInt64:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::U64)];
        case OperandType::UInt32:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::U32)];
        case OperandType::UInt16:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::U16)];
        case OperandType::UInt8:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::U8)];
        case OperandType::Float64:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::F64)];
        case OperandType::Float32:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::F32)];
        case OperandType::Char32:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::Char)];
        case OperandType::String:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::String)];
        case OperandType::Bytes:
            return m_intrinsiccache[static_cast<int>(IntrinsicType::Bytes)];

        case OperandType::Ref: {
            BaseRef *ref;
            TU_ASSERT (value.getRef(ref));
            return ref->getVirtualTable()->getType();
        }
        case OperandType::Status: {
            StatusRef *status;
            TU_ASSERT (value.getStatus(status));
            return status->getVirtualTable()->getType();
        }
        case OperandType::Protocol: {
            ProtocolRef *protocol;
            TU_ASSERT (value.getProtocol(protocol));
            return protocol->protocolType();
        }

        case OperandType::Descriptor: {
            DescriptorEntry *descriptorEntry;
            TU_ASSERT (value.getDescriptor(descriptorEntry));
            auto section = descriptorEntry->getLinkageSection();
            switch (section) {
                case lyric_object::LinkageSection::Action:
                    return m_intrinsiccache[static_cast<int>(IntrinsicType::Action)];
                case lyric_object::LinkageSection::Call:
                    return m_intrinsiccache[static_cast<int>(IntrinsicType::Call)];
                case lyric_object::LinkageSection::Class:
                    return m_intrinsiccache[static_cast<int>(IntrinsicType::Class)];
                case lyric_object::LinkageSection::Concept:
                    return m_intrinsiccache[static_cast<int>(IntrinsicType::Concept)];
                case lyric_object::LinkageSection::Enum:
                    return m_intrinsiccache[static_cast<int>(IntrinsicType::Enum)];
                case lyric_object::LinkageSection::Existential:
                    return m_intrinsiccache[static_cast<int>(IntrinsicType::Existential)];
                case lyric_object::LinkageSection::Field:
                    return m_intrinsiccache[static_cast<int>(IntrinsicType::Field)];
                case lyric_object::LinkageSection::Instance:
                    return m_intrinsiccache[static_cast<int>(IntrinsicType::Instance)];
                case lyric_object::LinkageSection::Namespace:
                    return m_intrinsiccache[static_cast<int>(IntrinsicType::Namespace)];
                case lyric_object::LinkageSection::Protocol:
                    return m_intrinsiccache[static_cast<int>(IntrinsicType::Protocol)];
                case lyric_object::LinkageSection::Struct:
                    return m_intrinsiccache[static_cast<int>(IntrinsicType::Struct)];
                default:
                    return InterpreterStatus::forCondition(InterpreterCondition::kRuntimeInvariant,
                        "{} descriptor has no type", lyric_object::linkage_section_to_name(section));
            }
        }

        case OperandType::Type:
            return InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "type descriptor has no type");
        default:
            return InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "invalid type");
    }
}

inline lyric_runtime::InterpreterStatus
resolve_type_to_descriptor(
    const lyric_runtime::Operand &type,
    lyric_runtime::Operand &descriptor,
    lyric_runtime::SegmentManager *segmentManager)
{
    lyric_runtime::TypeEntry *typeEntry;
    if (!type.getType(typeEntry))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid descriptor");

    // load the type descriptor from the specified assembly
    auto *segment = typeEntry->getSegment();
    if (segment == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid segment");

    auto object = segment->getObject();
    auto descriptorType = object.getType(typeEntry->getDescriptorIndex());
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

            auto typeCell = lyric_runtime::Operand::fromType(segment->lookupType(concreteIndex));
            return resolve_type_to_descriptor(typeCell, descriptor, segmentManager);
        }

        lyric_runtime::InterpreterStatus status;
        descriptor = segmentManager->resolveDescriptor(segment, concreteSection, concreteIndex, status);
        if (!descriptor.isValid())
            return status;
        return {};
    }

    // if the descriptor assignable type is not concrete, return invalid cell
    descriptor = lyric_runtime::Operand();
    return {};
}

inline lyric_runtime::InterpreterStatus
resolve_super_type(
    const lyric_runtime::Operand &type,
    lyric_runtime::Operand &super,
    lyric_runtime::SegmentManager *segmentManager)
{
    lyric_runtime::TypeEntry *typeEntry;
    if (!type.getType(typeEntry))
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid descriptor");

    auto *segment = typeEntry->getSegment();
    if (segment == nullptr)
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "invalid segment");
    auto object = segment->getObject();

    auto descriptorType = object.getType(typeEntry->getDescriptorIndex());
    if (!descriptorType.isValid())
        return lyric_runtime::InterpreterStatus::forCondition(
            lyric_runtime::InterpreterCondition::kRuntimeInvariant, "missing type descriptor");
    if (!descriptorType.hasSuperType()) {
        super = lyric_runtime::Operand();
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
lyric_runtime::TypeManager::compareTypes(const Operand &lhs, const Operand &rhs)
{
    Operand ld;
    Operand rd;
    Operand superType;

    TU_RETURN_IF_NOT_OK (resolve_type_to_descriptor(lhs, ld, m_segmentManager));
    if (!ld.isValid())
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid lhs descriptor");

    TU_RETURN_IF_NOT_OK (resolve_type_to_descriptor(rhs, rd, m_segmentManager));
    if (!rd.isValid())
        return InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid rhs descriptor");

    // if lhs and rhs are equal, then the comparison result is equal
    if (ld.isSameAs(rd))
        return TypeComparison::EQUAL;

    // if an ancestor of lhs equals rhs, then the comparison result is extends
    TU_RETURN_IF_NOT_OK (resolve_super_type(lhs, superType, m_segmentManager));

    while (superType.isValid()) {
        TU_RETURN_IF_NOT_OK (resolve_type_to_descriptor(superType, ld, m_segmentManager));
        if (!ld.isValid())
            return InterpreterStatus::forCondition(
                InterpreterCondition::kRuntimeInvariant, "type comparison has invalid lhs");
        if (ld.isSameAs(rd))
            return TypeComparison::EXTENDS;
        auto curr = superType;
        TU_RETURN_IF_NOT_OK (resolve_super_type(curr, superType, m_segmentManager));
    }

    // otherwise lhs is not a subtype of rhs
    return TypeComparison::DISJOINT;
}
