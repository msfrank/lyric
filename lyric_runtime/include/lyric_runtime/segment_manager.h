#ifndef LYRIC_RUNTIME_SEGMENT_MANAGER_H
#define LYRIC_RUNTIME_SEGMENT_MANAGER_H

#include <lyric_runtime/abstract_loader.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/call_cell.h>
#include <lyric_runtime/data_cell.h>
#include <lyric_runtime/interpreter_result.h>
#include <lyric_runtime/port_multiplexer.h>
#include <lyric_runtime/ref_handle.h>

namespace lyric_runtime {

    struct SegmentManagerData {
        std::shared_ptr<AbstractLoader> loader;
        std::vector<BytecodeSegment *> segments;
        absl::flat_hash_map<lyric_common::AssemblyLocation,tu_uint32> segmentcache;
        absl::flat_hash_map<DataCell,const VirtualTable *> vtablecache;
        absl::flat_hash_map<DataCell,const ExistentialTable *> etablecache;
        absl::flat_hash_map<DataCell,const ConceptTable *> ctablecache;
    };

    class SegmentManager {
    public:
        explicit SegmentManager(std::shared_ptr<AbstractLoader> loader);
        virtual ~SegmentManager();

        virtual BytecodeSegment *getSegment(tu_uint32 segmentIndex);

        virtual BytecodeSegment *loadAssembly(const lyric_common::AssemblyLocation &location);

        virtual const LinkEntry *resolveLink(
            const BytecodeSegment *sp,
            tu_uint32 address,
            tempo_utils::Status &status);
        virtual DataCell resolveDescriptor(
            const BytecodeSegment *sp,
            lyric_object::LinkageSection section,
            tu_uint32 address,
            tempo_utils::Status &status);

        virtual const ExistentialTable *resolveExistentialTable(
            const DataCell &descriptor,
            tempo_utils::Status &status);

        virtual const ConceptTable *resolveConceptTable(
            const DataCell &descriptor,
            tempo_utils::Status &status);

        virtual const VirtualTable *resolveClassVirtualTable(
            const DataCell &descriptor,
            tempo_utils::Status &status);
        virtual const VirtualTable *resolveEnumVirtualTable(
            const DataCell &descriptor,
            tempo_utils::Status &status);
        virtual const VirtualTable *resolveInstanceVirtualTable(
            const DataCell &descriptor,
            tempo_utils::Status &status);
        virtual const VirtualTable *resolveStructVirtualTable(
            const DataCell &descriptor,
            tempo_utils::Status &status);

        virtual tempo_utils::Status pushLiteralOntoStack(
            tu_uint32 address,
            const BytecodeSegment **ptr,
            StackfulCoroutine *currentCoro);
        virtual tempo_utils::Status pushDescriptorOntoStack(
            const BytecodeSegment *sp,
            tu_uint8 section,
            tu_uint32 address,
            StackfulCoroutine *currentCoro);
        virtual tempo_utils::Status pushSymbolDescriptorOntoStack(
            const lyric_common::SymbolUrl &symbolUrl,
            StackfulCoroutine *currentCoro);

        virtual DataCell loadStatic(
            tu_uint32 address,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);
        virtual bool storeStatic(
            tu_uint32 address,
            const DataCell &value,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        virtual DataCell loadInstance(
            tu_uint32 address,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);
        virtual bool storeInstance(
            tu_uint32 address,
            const DataCell &value,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        virtual DataCell loadEnum(
            tu_uint32 address,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);
        virtual bool storeEnum(
            tu_uint32 address,
            const DataCell &value,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

    private:
        SegmentManagerData m_data;
    };
}

#endif // LYRIC_RUNTIME_SEGMENT_MANAGER_H
