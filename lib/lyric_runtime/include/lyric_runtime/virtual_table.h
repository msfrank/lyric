#ifndef LYRIC_RUNTIME_VIRTUAL_TABLE_H
#define LYRIC_RUNTIME_VIRTUAL_TABLE_H

#include <absl/container/flat_hash_map.h>

#include "native_interface.h"
#include "operand.h"
#include "runtime_types.h"

namespace lyric_runtime {

    // forward declarations
    class BytecodeSegment;

    class VirtualMember {
    public:
        VirtualMember();
        VirtualMember(BytecodeSegment *segment, tu_uint32 memberIndex, tu_uint32 layoutOffset);
        VirtualMember(const VirtualMember &other);

        bool isValid() const;
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
        VirtualMethod(const VirtualMethod &other);

        bool isValid() const;
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

    class AbstractMemberResolver {
    public:
        virtual ~AbstractMemberResolver() = default;
        virtual const VirtualMember *getMember(const Operand &descriptor) const = 0;
    };

    class AbstractMethodResolver {
    public:
        virtual ~AbstractMethodResolver() = default;
        virtual const VirtualMethod *getMethod(const Operand &descriptor) const = 0;
    };

    class AbstractExtensionResolver {
    public:
        virtual ~AbstractExtensionResolver() = default;
        virtual const VirtualMethod *getExtension(
            const Operand &conceptDescriptor,
            const Operand &callDescriptor) const = 0;
    };

    class ImplTable : public AbstractMethodResolver {

    public:
        ImplTable(
            BytecodeSegment *segment,
            DescriptorEntry *descriptorEntry,
            TypeEntry *typeEntry,
            absl::flat_hash_map<OperandIdentity,VirtualMethod> &methods);

        BytecodeSegment *getSegment() const;
        DescriptorEntry *getDescriptorEntry() const;
        TypeEntry *getTypeEntry() const;

        Operand getDescriptor() const;
        Operand getType() const;

        lyric_common::SymbolUrl getSymbolUrl() const;
        lyric_object::LinkageSection getLinkageSection() const;
        tu_uint32 getDescriptorIndex() const;

        const VirtualMethod *getMethod(const Operand &descriptor) const override;

    private:
        BytecodeSegment *m_segment;
        DescriptorEntry *m_descriptor;
        TypeEntry *m_type;
        const absl::flat_hash_map<OperandIdentity,VirtualMethod> m_methods;
    };

    class ExistentialTable : public AbstractMethodResolver, public AbstractExtensionResolver{

    public:
        ExistentialTable(
            BytecodeSegment *segment,
            DescriptorEntry *descriptorEntry,
            TypeEntry *typeEntry,
            const ExistentialTable *parentTable,
            absl::flat_hash_map<OperandIdentity,VirtualMethod> &methods,
            absl::flat_hash_map<OperandIdentity,ImplTable> &impls);

        BytecodeSegment *getSegment() const;
        DescriptorEntry *getDescriptorEntry() const;
        TypeEntry *getTypeEntry() const;
        const ExistentialTable *getParent() const;

        Operand getDescriptor() const;
        Operand getType() const;

        lyric_common::SymbolUrl getSymbolUrl() const;
        lyric_object::LinkageSection getLinkageSection() const;
        tu_uint32 getDescriptorIndex() const;

        const VirtualMethod *getMethod(const Operand &descriptor) const override;
        const VirtualMethod *getExtension(
            const Operand &conceptDescriptor,
            const Operand &callDescriptor) const override;

    private:
        BytecodeSegment *m_segment;
        DescriptorEntry *m_descriptor;
        TypeEntry *m_type;
        const ExistentialTable *m_parent;
        const absl::flat_hash_map<OperandIdentity,VirtualMethod> m_methods;
        const absl::flat_hash_map<OperandIdentity,ImplTable> m_impls;
    };

    class ConceptTable : public AbstractExtensionResolver {

    public:
        ConceptTable(
            BytecodeSegment *segment,
            DescriptorEntry *descriptorEntry,
            TypeEntry *typeEntry,
            const ConceptTable *parentTable,
            absl::flat_hash_map<OperandIdentity,ImplTable> &impls);

        BytecodeSegment *getSegment() const;
        DescriptorEntry *getDescriptorEntry() const;
        TypeEntry *getTypeEntry() const;
        const ConceptTable *getParent() const;

        Operand getDescriptor() const;
        Operand getType() const;

        lyric_common::SymbolUrl getSymbolUrl() const;
        lyric_object::LinkageSection getLinkageSection() const;
        tu_uint32 getDescriptorIndex() const;

        const VirtualMethod *getExtension(
            const Operand &conceptDescriptor,
            const Operand &callDescriptor) const override;

    private:
        BytecodeSegment *m_segment;
        DescriptorEntry *m_descriptor;
        TypeEntry *m_type;
        const ConceptTable *m_parent;
        const absl::flat_hash_map<OperandIdentity,ImplTable> m_impls;
    };

    class VirtualTable : public AbstractMemberResolver, public AbstractMethodResolver, public AbstractExtensionResolver {

    public:
        VirtualTable(
            BytecodeSegment *segment,
            DescriptorEntry *descriptorEntry,
            TypeEntry *typeEntry,
            const VirtualTable *parentTable,
            NativeFunc allocator,
            absl::flat_hash_map<OperandIdentity,VirtualMember> &members,
            absl::flat_hash_map<OperandIdentity,VirtualMethod> &methods,
            absl::flat_hash_map<OperandIdentity,ImplTable> &impls);
        VirtualTable(
            BytecodeSegment *segment,
            DescriptorEntry *descriptorEntry,
            TypeEntry *typeEntry,
            const VirtualTable *parentTable,
            NativeFunc allocator,
            const VirtualMethod &initializer,
            absl::flat_hash_map<OperandIdentity,VirtualMember> &members,
            absl::flat_hash_map<OperandIdentity,VirtualMethod> &methods,
            absl::flat_hash_map<OperandIdentity,ImplTable> &impls);

        BytecodeSegment *getSegment() const;
        DescriptorEntry *getDescriptorEntry() const;
        TypeEntry *getTypeEntry() const;
        const VirtualTable *getParent() const;

        Operand getDescriptor() const;
        Operand getType() const;

        lyric_common::SymbolUrl getSymbolUrl() const;
        lyric_object::LinkageSection getLinkageSection() const;
        tu_uint32 getDescriptorIndex() const;
        tu_uint32 getLayoutStart() const;
        tu_uint32 getLayoutTotal() const;

        NativeFunc getAllocator() const;

        bool hasInitializer() const;
        const VirtualMethod *getInitializer() const;

        const VirtualMember *getMember(const Operand &descriptor) const override;
        const VirtualMethod *getMethod(const Operand &descriptor) const override;
        const VirtualMethod *getExtension(
            const Operand &conceptDescriptor,
            const Operand &callDescriptor) const override;

    private:
        BytecodeSegment *m_segment;
        DescriptorEntry *m_descriptor;
        TypeEntry *m_type;
        const VirtualTable *m_parent;
        const NativeFunc m_allocator;
        VirtualMethod m_initializer;
        const absl::flat_hash_map<OperandIdentity,VirtualMember> m_members;
        const absl::flat_hash_map<OperandIdentity,VirtualMethod> m_methods;
        const absl::flat_hash_map<OperandIdentity,ImplTable> m_impls;
    };
}

#endif // LYRIC_RUNTIME_VIRTUAL_TABLE_H
