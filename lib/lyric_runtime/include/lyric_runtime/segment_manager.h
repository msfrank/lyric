#ifndef LYRIC_RUNTIME_SEGMENT_MANAGER_H
#define LYRIC_RUNTIME_SEGMENT_MANAGER_H

#include <lyric_runtime/abstract_loader.h>
#include <lyric_runtime/bytecode_segment.h>
#include <lyric_runtime/call_cell.h>
#include <lyric_runtime/interpreter_result.h>
#include <lyric_runtime/port_multiplexer.h>
#include <lyric_runtime/ref_handle.h>

namespace lyric_runtime {

    struct SegmentManagerData {
        lyric_common::ModuleLocation origin;
        std::shared_ptr<AbstractLoader> systemLoader;
        std::shared_ptr<AbstractLoader> applicationLoader;
        std::vector<BytecodeSegment *> segments;
        absl::flat_hash_map<lyric_common::ModuleLocation,tu_uint32> segmentcache;
        absl::flat_hash_map<OperandIdentity,const VirtualTable *> vtablecache;
        absl::flat_hash_map<OperandIdentity,const ExistentialTable *> etablecache;
        absl::flat_hash_map<OperandIdentity,const ConceptTable *> ctablecache;
    };

    class SegmentManager {
    public:
        SegmentManager(
            std::shared_ptr<AbstractLoader> systemLoader,
            std::shared_ptr<AbstractLoader> applicationLoader);
        virtual ~SegmentManager();

        virtual lyric_common::ModuleLocation getOrigin() const;
        virtual tempo_utils::Status setOrigin(const lyric_common::ModuleLocation &origin);

        virtual BytecodeSegment *getSegment(tu_uint32 segmentIndex);
        virtual BytecodeSegment *getSegment(const lyric_common::ModuleLocation &location);

        virtual BytecodeSegment *getOrLoadSegment(const lyric_common::ModuleLocation &location, bool useSystemLoader);

        virtual bool hasResource(
            const lyric_common::ModuleLocation &location,
            bool useSystemLoader,
            tempo_utils::Status *statusptr);
        virtual std::shared_ptr<const tempo_utils::ImmutableBytes> loadResource(
            const lyric_common::ModuleLocation &location,
            bool useSystemLoader,
            tempo_utils::Status *statusptr);

        virtual const LinkEntry *resolveLink(
            const BytecodeSegment *sp,
            tu_uint32 index,
            tempo_utils::Status &status);

        virtual const std::string_view resolveString(
            const BytecodeSegment *sp,
            tu_uint32 index,
            tempo_utils::Status &status);

        virtual Operand resolveDescriptor(
            const BytecodeSegment *sp,
            lyric_object::LinkageSection section,
            tu_uint32 address,
            tempo_utils::Status &status);

        virtual Operand resolveSymbol(
            const BytecodeSegment *sp,
            tu_uint32 address,
            tempo_utils::Status &status);

        virtual Operand resolveReceiver(
            const BytecodeSegment *sp,
            tu_uint32 address,
            tempo_utils::Status &status);

        virtual const ExistentialTable *resolveExistentialTable(
            const Operand &descriptor,
            tempo_utils::Status &status);

        virtual const ConceptTable *resolveConceptTable(
            const Operand &descriptor,
            tempo_utils::Status &status);

        virtual const VirtualTable *resolveClassVirtualTable(
            const Operand &descriptor,
            tempo_utils::Status &status);
        virtual const VirtualTable *resolveEnumVirtualTable(
            const Operand &descriptor,
            tempo_utils::Status &status);
        virtual const VirtualTable *resolveInstanceVirtualTable(
            const Operand &descriptor,
            tempo_utils::Status &status);
        virtual const VirtualTable *resolveStructVirtualTable(
            const Operand &descriptor,
            tempo_utils::Status &status);

        virtual tempo_utils::Status pushDescriptorOntoStack(
            const BytecodeSegment *sp,
            tu_uint8 section,
            tu_uint32 address,
            StackfulCoroutine *currentCoro);
        virtual tempo_utils::Status pushSymbolDescriptorOntoStack(
            const lyric_common::SymbolUrl &symbolUrl,
            const BytecodeSegment *sp,
            StackfulCoroutine *currentCoro);

        virtual Operand loadStatic(
            tu_uint32 address,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);
        virtual bool storeStatic(
            tu_uint32 address,
            const Operand &value,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        virtual Operand loadInstance(
            tu_uint32 address,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);
        virtual bool storeInstance(
            tu_uint32 address,
            const Operand &value,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        virtual Operand loadEnum(
            tu_uint32 address,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);
        virtual bool storeEnum(
            tu_uint32 address,
            const Operand &value,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        virtual Operand loadProtocol(
            tu_uint32 address,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);
        virtual bool storeProtocol(
            tu_uint32 address,
            const Operand &value,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

        virtual Operand loadNamespace(
            tu_uint32 address,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);
        virtual bool storeNamespace(
            tu_uint32 address,
            const Operand &value,
            StackfulCoroutine *currentCoro,
            tempo_utils::Status &status);

    private:
        SegmentManagerData m_data;
    };
}

#endif // LYRIC_RUNTIME_SEGMENT_MANAGER_H
