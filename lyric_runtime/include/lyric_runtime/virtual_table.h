#ifndef LYRIC_RUNTIME_VIRTUAL_TABLE_H
#define LYRIC_RUNTIME_VIRTUAL_TABLE_H

#include <absl/container/flat_hash_map.h>

#include "data_cell.h"
#include "hash_data_cell.h"
#include "native_interface.h"
#include "runtime_types.h"

namespace lyric_runtime {

    // forward declarations
    class BytecodeSegment;

    class VirtualMember {
    public:
        VirtualMember();
        VirtualMember(BytecodeSegment *segment, uint32_t memberIndex, uint32_t layoutOffset);

        BytecodeSegment *getSegment() const;
        uint32_t getMemberIndex() const;
        uint32_t getLayoutOffset() const;

    private:
        BytecodeSegment *m_segment;
        const uint32_t m_memberIndex;
        const uint32_t m_layoutOffset;
    };

    class VirtualMethod {
    public:
        VirtualMethod();
        VirtualMethod(BytecodeSegment *segment, uint32_t callIndex, uint32_t procOffset);

        BytecodeSegment *getSegment() const;
        uint32_t getCallIndex() const;
        uint32_t getProcOffset() const;

    private:
        BytecodeSegment *m_segment;
        const uint32_t m_callIndex;
        const uint32_t m_procOffset;
    };

    class VirtualTable {

    public:
        VirtualTable(
            BytecodeSegment *segment,
            const DataCell &descriptor,
            const DataCell &type,
            const VirtualTable *parentTable,
            NativeFunc allocator,
            const VirtualMethod &ctor,
            absl::flat_hash_map<DataCell,VirtualMember> &members,
            absl::flat_hash_map<DataCell,VirtualMethod> &methods);
        VirtualTable(
            BytecodeSegment *segment,
            const DataCell &descriptor,
            const DataCell &type,
            const VirtualTable *parentTable,
            NativeFunc allocator,
            const VirtualMethod &ctor,
            absl::flat_hash_map<DataCell,VirtualMember> &members,
            absl::flat_hash_map<DataCell,VirtualMethod> &methods,
            absl::flat_hash_map<DataCell,VirtualTable> &impls);
        VirtualTable(
            BytecodeSegment *segment,
            const DataCell &descriptor,
            const DataCell &type,
            absl::flat_hash_map<DataCell,VirtualMethod> &methods);

        BytecodeSegment *getSegment() const;
        DataCell getDescriptor() const;
        DataCell getType() const;
        const VirtualTable *getParent() const;

        lyric_common::SymbolUrl getSymbolUrl() const;
        lyric_object::LinkageSection getLinkageSection() const;
        uint32_t getDescriptorIndex() const;
        uint32_t getLayoutStart() const;
        uint32_t getLayoutTotal() const;

        NativeFunc getAllocator() const;
        const VirtualMethod *getCtor() const;

        const VirtualMember *getMember(const DataCell &descriptor) const;
        const VirtualMethod *getMethod(const DataCell &descriptor) const;
        const VirtualMethod *getExtension(const DataCell &conceptDescriptor, const DataCell &callDescriptor) const;

    private:
        BytecodeSegment *m_segment;
        const DataCell m_descriptor;
        const DataCell m_type;
        const VirtualTable *m_parent;
        const NativeFunc m_allocator;
        const VirtualMethod m_ctor;
        const absl::flat_hash_map<DataCell,VirtualMember> m_members;
        const absl::flat_hash_map<DataCell,VirtualMethod> m_methods;
        const absl::flat_hash_map<DataCell,VirtualTable> m_impls;
    };
}

#endif // LYRIC_RUNTIME_VIRTUAL_TABLE_H
