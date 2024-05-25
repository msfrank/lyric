
#include <lyric_runtime/internal/assembly_reader.h>
#include <lyric_runtime/internal/get_class_virtual_table.h>
#include <lyric_runtime/internal/get_concept_table.h>
#include <lyric_runtime/internal/get_enum_virtual_table.h>
#include <lyric_runtime/internal/get_existential_table.h>
#include <lyric_runtime/internal/get_instance_virtual_table.h>
#include <lyric_runtime/internal/get_struct_virtual_table.h>
#include <lyric_runtime/internal/load_utils.h>
#include <lyric_runtime/internal/resolve_link.h>
#include <lyric_runtime/segment_manager.h>

lyric_runtime::SegmentManager::SegmentManager(std::shared_ptr<AbstractLoader> loader)
{
    m_data.loader = std::move(loader);
    TU_ASSERT (m_data.loader != nullptr);
}

lyric_runtime::SegmentManager::~SegmentManager()
{
    for (const auto &ventry : m_data.vtablecache) {
        delete ventry.second;
    }
    m_data.vtablecache.clear();

    for (auto *segment : m_data.segments) {
        delete segment;
    }
    m_data.segments.clear();
    m_data.segmentcache.clear();
}

lyric_runtime::BytecodeSegment *
lyric_runtime::SegmentManager::getSegment(tu_uint32 segmentIndex)
{
    if (segmentIndex < m_data.segments.size())
        return m_data.segments[segmentIndex];
    return nullptr;
}

lyric_runtime::BytecodeSegment *
lyric_runtime::SegmentManager::getSegment(const lyric_common::AssemblyLocation &location)
{
    auto entry = m_data.segmentcache.find(location);
    if (entry != m_data.segmentcache.cend())
        return getSegment(entry->second);
    return nullptr;
}

lyric_runtime::BytecodeSegment *
lyric_runtime::SegmentManager::getOrLoadSegment(const lyric_common::AssemblyLocation &location)
{
    return internal::get_or_load_segment(location, &m_data);
}

const lyric_runtime::LinkEntry *
lyric_runtime::SegmentManager::resolveLink(
    const BytecodeSegment *sp,
    tu_uint32 index,
    tempo_utils::Status &status)
{
    return internal::resolve_link(sp, index, &m_data, status);
}

lyric_runtime::DataCell
lyric_runtime::SegmentManager::resolveDescriptor(
    const BytecodeSegment *sp,
    lyric_object::LinkageSection section,
    tu_uint32 address,
    tempo_utils::Status &status)
{
    return internal::resolve_descriptor(sp, section, address, &m_data, status);
}

lyric_runtime::LiteralCell
lyric_runtime::SegmentManager::resolveLiteral(
    const BytecodeSegment *sp,
    tu_uint32 address,
    tempo_utils::Status &status)
{
    return internal::resolve_literal(sp, address, &m_data, status);
}

const lyric_runtime::VirtualTable *
lyric_runtime::SegmentManager::resolveClassVirtualTable(
    const DataCell &descriptor,
    tempo_utils::Status &status)
{
    return internal::get_class_virtual_table(descriptor, &m_data, status);
}

const lyric_runtime::ConceptTable *
lyric_runtime::SegmentManager::resolveConceptTable(
    const DataCell &descriptor,
    tempo_utils::Status &status)
{
    return internal::get_concept_table(descriptor, &m_data, status);
}

const lyric_runtime::VirtualTable *
lyric_runtime::SegmentManager::resolveEnumVirtualTable(
    const DataCell &descriptor,
    tempo_utils::Status &status)
{
    return internal::get_enum_virtual_table(descriptor, &m_data, status);
}

const lyric_runtime::ExistentialTable *
lyric_runtime::SegmentManager::resolveExistentialTable(
    const DataCell &descriptor,
    tempo_utils::Status &status)
{
    return internal::get_existential_table(descriptor, &m_data, status);
}

const lyric_runtime::VirtualTable *
lyric_runtime::SegmentManager::resolveInstanceVirtualTable(
    const DataCell &descriptor,
    tempo_utils::Status &status)
{
    return internal::get_instance_virtual_table(descriptor, &m_data, status);
}

const lyric_runtime::VirtualTable *
lyric_runtime::SegmentManager::resolveStructVirtualTable(
    const DataCell &descriptor,
    tempo_utils::Status &status)
{
    return internal::get_struct_virtual_table(descriptor, &m_data, status);
}

tempo_utils::Status
lyric_runtime::SegmentManager::pushLiteralOntoStack(
    const BytecodeSegment *sp,
    tu_uint32 address,
    StackfulCoroutine *currentCoro)
{
    return internal::push_literal_onto_stack(sp, address, currentCoro, &m_data);
}

tempo_utils::Status
lyric_runtime::SegmentManager::pushDescriptorOntoStack(
    const BytecodeSegment *sp,
    tu_uint8 section,
    tu_uint32 address,
    StackfulCoroutine *currentCoro)
{
    return internal::push_descriptor_onto_stack(sp, section, address, currentCoro, &m_data);
}

tempo_utils::Status
lyric_runtime::SegmentManager::pushSymbolDescriptorOntoStack(
    const lyric_common::SymbolUrl &symbolUrl,
    StackfulCoroutine *currentCoro)
{
    return internal::push_symbol_descriptor_onto_stack(symbolUrl, currentCoro, &m_data);
}

lyric_runtime::DataCell
lyric_runtime::SegmentManager::loadStatic(
    tu_uint32 address,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    auto *sp = currentCoro->peekSP();

    if (lyric_object::IS_NEAR(address))
        return sp->getStatic(address);

    const auto *linkage = resolveLink(sp, lyric_object::GET_LINK_OFFSET(address), status);
    if (linkage == nullptr)
        return {};
    if (linkage->linkage != lyric_object::LinkageSection::Static) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid static linkage");
        return {};
    }
    auto *segment = getSegment(linkage->assembly);
    return segment->getStatic(linkage->value);
}

bool
lyric_runtime::SegmentManager::storeStatic(
    tu_uint32 address,
    const DataCell &value,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    auto *sp = currentCoro->peekSP();

    if (lyric_object::IS_NEAR(address))
        return sp->setStatic(address, value);

    const auto *linkage = resolveLink(sp, lyric_object::GET_LINK_OFFSET(address), status);
    if (linkage == nullptr)
        return false;                   // failed to resolve dynamic link
    if (linkage->linkage != lyric_object::LinkageSection::Static) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid static linkage");
        return false;
    }
    auto *segment = getSegment(linkage->assembly);
    return segment->setStatic(linkage->value, value);
}

lyric_runtime::DataCell
lyric_runtime::SegmentManager::loadInstance(
    tu_uint32 address,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    auto *sp = currentCoro->peekSP();

    if (lyric_object::IS_NEAR(address))
        return sp->getInstance(address);

    const auto *linkage = resolveLink(sp, lyric_object::GET_LINK_OFFSET(address), status);
    if (linkage == nullptr)
        return {};               // failed to resolve dynamic link
    if (linkage->linkage != lyric_object::LinkageSection::Instance) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid instance linkage");
        return {};
    }
    auto *segment = getSegment(linkage->assembly);
    return segment->getInstance(linkage->value);
}

bool
lyric_runtime::SegmentManager::storeInstance(
    tu_uint32 address,
    const DataCell &value,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    auto *sp = currentCoro->peekSP();

    if (lyric_object::IS_NEAR(address))
        return sp->setInstance(address, value);

    const auto *linkage = resolveLink(sp, lyric_object::GET_LINK_OFFSET(address), status);
    if (linkage == nullptr)
        return false;                   // failed to resolve dynamic link
    if (linkage->linkage != lyric_object::LinkageSection::Instance) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid instance linkage");
        return false;
    }
    auto *segment = getSegment(linkage->assembly);
    return segment->setInstance(linkage->value, value);
}

lyric_runtime::DataCell
lyric_runtime::SegmentManager::loadEnum(
    tu_uint32 address,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    auto *sp = currentCoro->peekSP();

    if (lyric_object::IS_NEAR(address))
        return sp->getEnum(address);

    const auto *linkage = resolveLink(sp, lyric_object::GET_LINK_OFFSET(address), status);
    if (linkage == nullptr)
        return {};               // failed to resolve dynamic link
    if (linkage->linkage != lyric_object::LinkageSection::Enum) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid enum linkage");
        return {};
    }
    auto *segment = getSegment(linkage->assembly);
    return segment->getEnum(linkage->value);
}

bool
lyric_runtime::SegmentManager::storeEnum(
    tu_uint32 address,
    const DataCell &value,
    StackfulCoroutine *currentCoro,
    tempo_utils::Status &status)
{
    auto *sp = currentCoro->peekSP();

    if (lyric_object::IS_NEAR(address))
        return sp->setEnum(address, value);

    const auto *linkage = resolveLink(sp, lyric_object::GET_LINK_OFFSET(address), status);
    if (linkage == nullptr)
        return false;                   // failed to resolve dynamic link
    if (linkage->linkage != lyric_object::LinkageSection::Enum) {
        status = InterpreterStatus::forCondition(
            InterpreterCondition::kRuntimeInvariant, "invalid enum linkage");
        return false;
    }
    auto *segment = getSegment(linkage->assembly);
    return segment->setEnum(linkage->value, value);
}
