
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/virtual_table.h>
#include <tempo_utils/log_stream.h>

lyric_runtime::VirtualMember::VirtualMember()
    : m_segment(nullptr),
      m_memberIndex(INVALID_ADDRESS_U32),
      m_layoutOffset(INVALID_ADDRESS_U32)
{
}

lyric_runtime::VirtualMember::VirtualMember(
    BytecodeSegment *segment,
    uint32_t memberIndex,
    uint32_t layoutOffset)
    : m_segment(segment),
      m_memberIndex(memberIndex),
      m_layoutOffset(layoutOffset)
{
    TU_ASSERT (m_segment != nullptr);
}

lyric_runtime::BytecodeSegment *
lyric_runtime::VirtualMember::getSegment() const
{
    return m_segment;
}

uint32_t
lyric_runtime::VirtualMember::getMemberIndex() const
{
    return m_memberIndex;
}

uint32_t
lyric_runtime::VirtualMember::getLayoutOffset() const
{
    return m_layoutOffset;
}

lyric_runtime::VirtualMethod::VirtualMethod()
    : m_segment(nullptr),
      m_callIndex(INVALID_ADDRESS_U32),
      m_procOffset(INVALID_ADDRESS_U32),
      m_returnsValue(false)
{
}

lyric_runtime::VirtualMethod::VirtualMethod(
    BytecodeSegment *segment,
    uint32_t callIndex,
    uint32_t procOffset,
    bool returnsValue)
    : m_segment(segment),
      m_callIndex(callIndex),
      m_procOffset(procOffset),
      m_returnsValue(returnsValue)
{
    TU_ASSERT (m_segment != nullptr);
}

lyric_runtime::BytecodeSegment *
lyric_runtime::VirtualMethod::getSegment() const
{
    return m_segment;
}

uint32_t
lyric_runtime::VirtualMethod::getCallIndex() const
{
    return m_callIndex;
}

uint32_t
lyric_runtime::VirtualMethod::getProcOffset() const
{
    return m_procOffset;
}

bool
lyric_runtime::VirtualMethod::returnsValue() const
{
    return m_returnsValue;
}

lyric_runtime::ImplTable::ImplTable(
    BytecodeSegment *segment,
    const DataCell &descriptor,
    const DataCell &type,
    absl::flat_hash_map<DataCell,VirtualMethod> &methods)
    : m_segment(segment),
      m_descriptor(descriptor),
      m_type(type),
      m_methods(std::move(methods))
{
    TU_ASSERT (m_segment != nullptr);
    TU_ASSERT (m_descriptor.isValid());
    TU_ASSERT (m_type.isValid());
}

lyric_runtime::BytecodeSegment *
lyric_runtime::ImplTable::getSegment() const
{
    return m_segment;
}

lyric_runtime::DataCell
lyric_runtime::ImplTable::getDescriptor() const
{
    return m_descriptor;
}

lyric_runtime::DataCell
lyric_runtime::ImplTable::getType() const
{
    return m_type;
}

lyric_common::SymbolUrl
lyric_runtime::ImplTable::getSymbolUrl() const
{
    auto objectLocation = m_segment->getObjectLocation();
    auto object = m_segment->getObject();

    lyric_object::LinkageSection section;
    switch (m_descriptor.type) {
        case DataCellType::CLASS:
            section = lyric_object::LinkageSection::Class;
            break;
        case DataCellType::CONCEPT:
            section = lyric_object::LinkageSection::Concept;
            break;
        case DataCellType::ENUM:
            section = lyric_object::LinkageSection::Enum;
            break;
        case DataCellType::INSTANCE:
            section = lyric_object::LinkageSection::Instance;
            break;
        case DataCellType::STRUCT:
            section = lyric_object::LinkageSection::Struct;
            break;
        default:
            return {};
    }

    auto symbolPath = object.getSymbolPath(section, getDescriptorIndex());
    return lyric_common::SymbolUrl(objectLocation, symbolPath);
}

lyric_object::LinkageSection
lyric_runtime::ImplTable::getLinkageSection() const
{
    switch (m_descriptor.type) {
        case DataCellType::CLASS:
            return lyric_object::LinkageSection::Class;
        case DataCellType::CONCEPT:
            return lyric_object::LinkageSection::Concept;
        case DataCellType::ENUM:
            return lyric_object::LinkageSection::Enum;
        case DataCellType::INSTANCE:
            return lyric_object::LinkageSection::Instance;
        case DataCellType::STRUCT:
            return lyric_object::LinkageSection::Struct;
        default:
            return lyric_object::LinkageSection::Invalid;
    }
}

uint32_t
lyric_runtime::ImplTable::getDescriptorIndex() const
{
    return m_descriptor.data.descriptor->getDescriptorIndex();
}

const lyric_runtime::VirtualMethod *
lyric_runtime::ImplTable::getMethod(const DataCell &descriptor) const
{
    if (m_methods.contains(descriptor)) {
        const auto &method = m_methods.at(descriptor);
        return &method;     // return pointer to the virtual method entry
    }
    return nullptr;
}

lyric_runtime::ExistentialTable::ExistentialTable(
    BytecodeSegment *segment,
    const DataCell &descriptor,
    const DataCell &type,
    const ExistentialTable *parentTable,
    absl::flat_hash_map<DataCell,VirtualMethod> &methods,
    absl::flat_hash_map<DataCell,ImplTable> &impls)
    : m_segment(segment),
      m_descriptor(descriptor),
      m_type(type),
      m_parent(parentTable),
      m_methods(std::move(methods)),
      m_impls(std::move(impls))
{
    TU_ASSERT (m_segment != nullptr);
    TU_ASSERT (m_descriptor.isValid());
    TU_ASSERT (m_type.isValid());
}

lyric_runtime::BytecodeSegment *
lyric_runtime::ExistentialTable::getSegment() const
{
    return m_segment;
}

lyric_runtime::DataCell
lyric_runtime::ExistentialTable::getDescriptor() const
{
    return m_descriptor;
}

lyric_runtime::DataCell
lyric_runtime::ExistentialTable::getType() const
{
    return m_type;
}

const lyric_runtime::ExistentialTable *
lyric_runtime::ExistentialTable::getParent() const
{
    return m_parent;
}

lyric_common::SymbolUrl
lyric_runtime::ExistentialTable::getSymbolUrl() const
{
    auto objectLocation = m_segment->getObjectLocation();
    auto object = m_segment->getObject();

    lyric_object::LinkageSection section;
    switch (m_descriptor.type) {
        case DataCellType::EXISTENTIAL:
            section = lyric_object::LinkageSection::Existential;
            break;
        default:
            return {};
    }

    auto symbolPath = object.getSymbolPath(section, getDescriptorIndex());
    return lyric_common::SymbolUrl(objectLocation, symbolPath);
}

lyric_object::LinkageSection
lyric_runtime::ExistentialTable::getLinkageSection() const
{
    switch (m_descriptor.type) {
        case DataCellType::EXISTENTIAL:
            return lyric_object::LinkageSection::Existential;
        default:
            return lyric_object::LinkageSection::Invalid;
    }
}

uint32_t
lyric_runtime::ExistentialTable::getDescriptorIndex() const
{
    return m_descriptor.data.descriptor->getDescriptorIndex();
}

const lyric_runtime::VirtualMethod *
lyric_runtime::ExistentialTable::getMethod(const DataCell &descriptor) const
{
    for (auto *curr = this; curr != nullptr; curr = curr->m_parent) {
        if (curr->m_methods.contains(descriptor)) {
            const auto &method = curr->m_methods.at(descriptor);
            return &method;     // return pointer to the virtual method entry
        }
    }
    return nullptr;
}

const lyric_runtime::VirtualMethod *
lyric_runtime::ExistentialTable::getExtension(const DataCell &conceptDescriptor, const DataCell &callDescriptor) const
{
    if (!m_impls.contains(conceptDescriptor))
        return nullptr;

    const auto &impl = m_impls.at(conceptDescriptor);
    return impl.getMethod(callDescriptor);
}

lyric_runtime::ConceptTable::ConceptTable(
    BytecodeSegment *segment,
    const DataCell &descriptor,
    const DataCell &type,
    const ConceptTable *parentTable,
    absl::flat_hash_map<DataCell,ImplTable> &impls)
    : m_segment(segment),
      m_descriptor(descriptor),
      m_type(type),
      m_parent(parentTable),
      m_impls(std::move(impls))
{
    TU_ASSERT (m_segment != nullptr);
    TU_ASSERT (m_descriptor.isValid());
    TU_ASSERT (m_type.isValid());
}

lyric_runtime::BytecodeSegment *
lyric_runtime::ConceptTable::getSegment() const
{
    return m_segment;
}

lyric_runtime::DataCell
lyric_runtime::ConceptTable::getDescriptor() const
{
    return m_descriptor;
}

lyric_runtime::DataCell
lyric_runtime::ConceptTable::getType() const
{
    return m_type;
}

const lyric_runtime::ConceptTable *
lyric_runtime::ConceptTable::getParent() const
{
    return m_parent;
}

lyric_common::SymbolUrl
lyric_runtime::ConceptTable::getSymbolUrl() const
{
    auto objectLocation = m_segment->getObjectLocation();
    auto object = m_segment->getObject();

    lyric_object::LinkageSection section;
    switch (m_descriptor.type) {
        case DataCellType::CONCEPT:
            section = lyric_object::LinkageSection::Concept;
            break;
        default:
            return {};
    }

    auto symbolPath = object.getSymbolPath(section, getDescriptorIndex());
    return lyric_common::SymbolUrl(objectLocation, symbolPath);
}

lyric_object::LinkageSection
lyric_runtime::ConceptTable::getLinkageSection() const
{
    switch (m_descriptor.type) {
        case DataCellType::CONCEPT:
            return lyric_object::LinkageSection::Concept;
        default:
            return lyric_object::LinkageSection::Invalid;
    }
}

uint32_t
lyric_runtime::ConceptTable::getDescriptorIndex() const
{
    return m_descriptor.data.descriptor->getDescriptorIndex();
}

const lyric_runtime::VirtualMethod *
lyric_runtime::ConceptTable::getExtension(const DataCell &conceptDescriptor, const DataCell &callDescriptor) const
{
    if (!m_impls.contains(conceptDescriptor))
        return nullptr;

    const auto &impl = m_impls.at(conceptDescriptor);
    return impl.getMethod(callDescriptor);
}

lyric_runtime::VirtualTable::VirtualTable(
    BytecodeSegment *segment,
    const DataCell &descriptor,
    const DataCell &type,
    const VirtualTable *parentTable,
    NativeFunc allocator,
    const VirtualMethod &ctor,
    absl::flat_hash_map<DataCell,VirtualMember> &members,
    absl::flat_hash_map<DataCell,VirtualMethod> &methods,
    absl::flat_hash_map<DataCell,ImplTable> &impls)
    : m_segment(segment),
      m_descriptor(descriptor),
      m_type(type),
      m_parent(parentTable),
      m_allocator(allocator),
      m_ctor(ctor),
      m_members(std::move(members)),
      m_methods(std::move(methods)),
      m_impls(std::move(impls))
{
    TU_ASSERT (m_segment != nullptr);
    TU_ASSERT (m_descriptor.isValid());
    TU_ASSERT (m_type.isValid());
}

lyric_runtime::BytecodeSegment *
lyric_runtime::VirtualTable::getSegment() const
{
    return m_segment;
}

lyric_common::SymbolUrl
lyric_runtime::VirtualTable::getSymbolUrl() const
{
    auto objectLocation = m_segment->getObjectLocation();
    auto object = m_segment->getObject();

    lyric_object::LinkageSection section;
    switch (m_descriptor.type) {
        case DataCellType::CLASS:
            section = lyric_object::LinkageSection::Class;
            break;
        case DataCellType::ENUM:
            section = lyric_object::LinkageSection::Enum;
            break;
        case DataCellType::INSTANCE:
            section = lyric_object::LinkageSection::Instance;
            break;
        case DataCellType::STRUCT:
            section = lyric_object::LinkageSection::Struct;
            break;
        default:
            return {};
    }

    auto symbolPath = object.getSymbolPath(section, getDescriptorIndex());
    return lyric_common::SymbolUrl(objectLocation, symbolPath);
}

lyric_runtime::DataCell
lyric_runtime::VirtualTable::getDescriptor() const
{
    return m_descriptor;
}

lyric_object::LinkageSection
lyric_runtime::VirtualTable::getLinkageSection() const
{
    switch (m_descriptor.type) {
        case DataCellType::CLASS:
            return lyric_object::LinkageSection::Class;
        case DataCellType::ENUM:
            return lyric_object::LinkageSection::Enum;
        case DataCellType::INSTANCE:
            return lyric_object::LinkageSection::Instance;
        case DataCellType::STRUCT:
            return lyric_object::LinkageSection::Struct;
        default:
            return lyric_object::LinkageSection::Invalid;
    }
}

uint32_t
lyric_runtime::VirtualTable::getDescriptorIndex() const
{
    return m_descriptor.data.descriptor->getDescriptorIndex();
}

lyric_runtime::DataCell
lyric_runtime::VirtualTable::getType() const
{
    return m_type;
}

const lyric_runtime::VirtualTable *
lyric_runtime::VirtualTable::getParent() const
{
    return m_parent;
}

uint32_t
lyric_runtime::VirtualTable::getLayoutStart() const
{
    uint32_t size = 0;
    for (auto *curr = m_parent; curr != nullptr; curr = curr->m_parent) {
        size += curr->m_members.size();
    }
    return size;
}

uint32_t
lyric_runtime::VirtualTable::getLayoutTotal() const
{
    uint32_t size = 0;
    for (auto *curr = this; curr != nullptr; curr = curr->m_parent) {
        size += curr->m_members.size();
    }
    return size;
}

lyric_runtime::NativeFunc
lyric_runtime::VirtualTable::getAllocator() const
{
    return m_allocator;
}

const lyric_runtime::VirtualMethod *
lyric_runtime::VirtualTable::getCtor() const
{
    return &m_ctor;
}

const lyric_runtime::VirtualMember *
lyric_runtime::VirtualTable::getMember(const DataCell &descriptor) const
{
    for (auto *curr = this; curr != nullptr; curr = curr->m_parent) {
        if (curr->m_members.contains(descriptor)) {
            const auto &member = curr->m_members.at(descriptor);
            return &member;     // return pointer to the virtual member entry
        }
    }
    return nullptr;
}

const lyric_runtime::VirtualMethod *
lyric_runtime::VirtualTable::getMethod(const DataCell &descriptor) const
{
    for (auto *curr = this; curr != nullptr; curr = curr->m_parent) {
        if (curr->m_methods.contains(descriptor)) {
            const auto &method = curr->m_methods.at(descriptor);
            return &method;     // return pointer to the virtual method entry
        }
    }
    return nullptr;
}

const lyric_runtime::VirtualMethod *
lyric_runtime::VirtualTable::getExtension(const DataCell &conceptDescriptor, const DataCell &callDescriptor) const
{
    if (!m_impls.contains(conceptDescriptor))
        return nullptr;

    const auto &impl = m_impls.at(conceptDescriptor);
    return impl.getMethod(callDescriptor);
}