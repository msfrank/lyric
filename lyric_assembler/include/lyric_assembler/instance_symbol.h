#ifndef LYRIC_ASSEMBLER_INSTANCE_SYMBOL_H
#define LYRIC_ASSEMBLER_INSTANCE_SYMBOL_H

#include <absl/container/flat_hash_map.h>

#include "abstract_member_reifier.h"
#include "abstract_symbol.h"
#include "assembly_state.h"
#include "base_symbol.h"
#include "call_invoker.h"
#include "ctor_invoker.h"
#include "method_invoker.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct InstanceSymbolPriv {
        lyric_object::AccessType access;
        lyric_object::DeriveType derive;
        bool isAbstract;
        TypeHandle *instanceType;
        InstanceSymbol *superInstance;
        tu_uint32 allocatorTrap;
        absl::flat_hash_map<std::string, SymbolBinding> members;
        absl::flat_hash_set<std::string> initializedMembers;
        absl::flat_hash_map<std::string, BoundMethod> methods;
        absl::flat_hash_map<lyric_common::TypeDef, ImplHandle *> impls;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        std::unique_ptr<BlockHandle> instanceBlock;
    };

    class InstanceSymbol : public BaseSymbol<InstanceAddress,InstanceSymbolPriv> {

    public:
        InstanceSymbol(
            const lyric_common::SymbolUrl &instanceUrl,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive,
            bool isAbstract,
            InstanceAddress address,
            TypeHandle *instanceType,
            InstanceSymbol *superInstance,
            BlockHandle *parentBlock,
            AssemblyState *state);
        InstanceSymbol(
            const lyric_common::SymbolUrl &instanceUrl,
            lyric_importer::InstanceImport *instanceImport,
            AssemblyState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getAssignableType() const override;
        TypeSignature getTypeSignature() const override;
        void touch() override;

        lyric_object::AccessType getAccessType() const;
        lyric_object::DeriveType getDeriveType() const;
        bool isAbstract() const;

        TypeHandle *instanceType() const;
        InstanceSymbol *superInstance() const;
        BlockHandle *instanceBlock() const;

        /*
         * instance member management
         */
        bool hasMember(const std::string &name) const;
        Option<SymbolBinding> getMember(const std::string &name) const;
        absl::flat_hash_map<std::string, SymbolBinding>::const_iterator membersBegin() const;
        absl::flat_hash_map<std::string, SymbolBinding>::const_iterator membersEnd() const;
        tu_uint32 numMembers() const;

        tempo_utils::Result<SymbolBinding> declareMember(
            const std::string &name,
            const lyric_parser::Assignable &memberSpec,
            bool isVariable,
            lyric_object::AccessType access,
            const lyric_common::SymbolUrl &init = {});

        tempo_utils::Result<SymbolBinding> resolveMember(
            const std::string &name,
            AbstractMemberReifier &reifier,
            const lyric_common::TypeDef &receiverType,
            bool isReceiver = false) const;

        bool isMemberInitialized(const std::string &name) const;
        tempo_utils::Status setMemberInitialized(const std::string &name);
        bool isCompletelyInitialized() const;

        /*
         * instance constructor management
         */
        lyric_common::SymbolUrl getCtor() const;
        tu_uint32 getAllocatorTrap() const;
        tempo_utils::Result<lyric_common::SymbolUrl> declareCtor(
            lyric_object::AccessType access,
            tu_uint32 allocatorTrap = lyric_runtime::INVALID_ADDRESS_U32);
        tempo_utils::Result<CtorInvoker> resolveCtor();

        /*
         * instance method management
         */
        bool hasMethod(const std::string &name) const;
        Option<BoundMethod> getMethod(const std::string &name) const;
        absl::flat_hash_map<std::string, BoundMethod>::const_iterator methodsBegin() const;
        absl::flat_hash_map<std::string, BoundMethod>::const_iterator methodsEnd() const;
        tu_uint32 numMethods() const;

        tempo_utils::Result<lyric_common::SymbolUrl> declareMethod(
            const std::string &name,
            const std::vector<ParameterSpec> &parameterSpec,
            const Option<ParameterSpec> &restSpec,
            const std::vector<ParameterSpec> &ctxSpec,
            const lyric_parser::Assignable &returnSpec,
            lyric_object::AccessType access);

        tempo_utils::Result<MethodInvoker> resolveMethod(
            const std::string &name,
            const lyric_common::TypeDef &receiverType,
            bool isReceiver = false) const;

        /*
         * instance impl management
         */
        bool hasImpl(const lyric_common::TypeDef &implType) const;
        ImplHandle *getImpl(const lyric_common::TypeDef &implType) const;
        absl::flat_hash_map<lyric_common::TypeDef, ImplHandle *>::const_iterator implsBegin() const;
        absl::flat_hash_map<lyric_common::TypeDef, ImplHandle *>::const_iterator implsEnd() const;
        tu_uint32 numImpls() const;

        tempo_utils::Result<lyric_common::TypeDef> declareImpl(const lyric_parser::Assignable &implSpec);

//        tempo_utils::Result<lyric_common::SymbolUrl> declareExtension(
//            const lyric_parser::Assignable &implSpec,
//            const std::string &name,
//            const std::vector<ParameterSpec> &parameterSpec,
//            const Option<ParameterSpec> &restSpec,
//            const std::vector<ParameterSpec> &ctxSpec,
//            const lyric_parser::Assignable &returnSpec);

        /*
         * subtype tracking for sealed instance
         */
        bool hasSealedType(const lyric_common::TypeDef &sealedType) const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesBegin() const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesEnd() const;
        tempo_utils::Status putSealedType(const lyric_common::TypeDef &sealedType);

    private:
        lyric_common::SymbolUrl m_instanceUrl;
        lyric_importer::InstanceImport *m_instanceImport;
        AssemblyState *m_state;

        InstanceSymbolPriv *load() override;
    };

    static inline const InstanceSymbol *cast_symbol_to_instance(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::INSTANCE);
        return static_cast<const InstanceSymbol *>(sym);    // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline InstanceSymbol *cast_symbol_to_instance(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::INSTANCE);
        return static_cast<InstanceSymbol *>(sym);          // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_INSTANCE_SYMBOL_H
