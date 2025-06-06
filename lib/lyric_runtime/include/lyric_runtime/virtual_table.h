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
        VirtualMember(BytecodeSegment *segment, tu_uint32 memberIndex, tu_uint32 layoutOffset);

        BytecodeSegment *getSegment() const;
        tu_uint32 getMemberIndex() const;
        tu_uint32 getLayoutOffset() const;

    private:
        BytecodeSegment *m_segment;
        const tu_uint32 m_memberIndex;
        const tu_uint32 m_layoutOffset;
    };

    class VirtualMethod {
    public:
        VirtualMethod();
        VirtualMethod(BytecodeSegment *segment, tu_uint32 callIndex, tu_uint32 procOffset, bool returnsValue);

        BytecodeSegment *getSegment() const;
        tu_uint32 getCallIndex() const;
        tu_uint32 getProcOffset() const;
        bool returnsValue() const;

    private:
        BytecodeSegment *m_segment;
        const tu_uint32 m_callIndex;
        const tu_uint32 m_procOffset;
        bool m_returnsValue;
    };

    class ImplTable {

    public:
        ImplTable(
            BytecodeSegment *segment,
            const DataCell &descriptor,
            const DataCell &type,
            absl::flat_hash_map<DataCell,VirtualMethod> &methods);

        BytecodeSegment *getSegment() const;
        DataCell getDescriptor() const;
        DataCell getType() const;

        lyric_common::SymbolUrl getSymbolUrl() const;
        lyric_object::LinkageSection getLinkageSection() const;
        tu_uint32 getDescriptorIndex() const;

        const VirtualMethod *getMethod(const DataCell &descriptor) const;

    private:
        BytecodeSegment *m_segment;
        const DataCell m_descriptor;
        const DataCell m_type;
        const absl::flat_hash_map<DataCell,VirtualMethod> m_methods;
    };

    class ExistentialTable {

    public:
        ExistentialTable(
            BytecodeSegment *segment,
            const DataCell &descriptor,
            const DataCell &type,
            const ExistentialTable *parentTable,
            absl::flat_hash_map<DataCell,VirtualMethod> &methods,
            absl::flat_hash_map<DataCell,ImplTable> &impls);

        BytecodeSegment *getSegment() const;
        DataCell getDescriptor() const;
        DataCell getType() const;
        const ExistentialTable *getParent() const;

        lyric_common::SymbolUrl getSymbolUrl() const;
        lyric_object::LinkageSection getLinkageSection() const;
        tu_uint32 getDescriptorIndex() const;

        const VirtualMethod *getMethod(const DataCell &descriptor) const;
        const VirtualMethod *getExtension(const DataCell &conceptDescriptor, const DataCell &callDescriptor) const;

    private:
        BytecodeSegment *m_segment;
        const DataCell m_descriptor;
        const DataCell m_type;
        const ExistentialTable *m_parent;
        const absl::flat_hash_map<DataCell,VirtualMethod> m_methods;
        const absl::flat_hash_map<DataCell,ImplTable> m_impls;
    };

    class ConceptTable {

    public:
        ConceptTable(
            BytecodeSegment *segment,
            const DataCell &descriptor,
            const DataCell &type,
            const ConceptTable *parentTable,
            absl::flat_hash_map<DataCell,ImplTable> &impls);

        BytecodeSegment *getSegment() const;
        DataCell getDescriptor() const;
        DataCell getType() const;
        const ConceptTable *getParent() const;

        lyric_common::SymbolUrl getSymbolUrl() const;
        lyric_object::LinkageSection getLinkageSection() const;
        tu_uint32 getDescriptorIndex() const;

        const VirtualMethod *getExtension(const DataCell &conceptDescriptor, const DataCell &callDescriptor) const;

    private:
        BytecodeSegment *m_segment;
        const DataCell m_descriptor;
        const DataCell m_type;
        const ConceptTable *m_parent;
        const absl::flat_hash_map<DataCell,ImplTable> m_impls;
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
            absl::flat_hash_map<DataCell,VirtualMethod> &methods,
            absl::flat_hash_map<DataCell,ImplTable> &impls);

        BytecodeSegment *getSegment() const;
        DataCell getDescriptor() const;
        DataCell getType() const;
        const VirtualTable *getParent() const;

        lyric_common::SymbolUrl getSymbolUrl() const;
        lyric_object::LinkageSection getLinkageSection() const;
        tu_uint32 getDescriptorIndex() const;
        tu_uint32 getLayoutStart() const;
        tu_uint32 getLayoutTotal() const;

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
        const absl::flat_hash_map<DataCell,ImplTable> m_impls;
    };
}

#endif // LYRIC_RUNTIME_VIRTUAL_TABLE_H
