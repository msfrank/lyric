#ifndef LYRIC_ASSEMBLER_STRUCT_SYMBOL_H
#define LYRIC_ASSEMBLER_STRUCT_SYMBOL_H

#include "abstract_member_reifier.h"
#include "abstract_symbol.h"
#include "assembly_state.h"
#include "base_symbol.h"
#include "call_invoker.h"
#include "ctor_invoker.h"
#include "method_invoker.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct StructSymbolPriv {
        lyric_object::AccessType access;
        lyric_object::DeriveType derive;
        bool isAbstract;
        TypeHandle *structType;
        StructSymbol *superStruct;
        tu_uint32 allocatorTrap;
        absl::flat_hash_map<std::string, SymbolBinding> members;
        absl::flat_hash_set<std::string> initializedMembers;
        absl::flat_hash_map<std::string, BoundMethod> methods;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        std::unique_ptr<BlockHandle> structBlock;
    };

    class StructSymbol : public BaseSymbol<StructAddress,StructSymbolPriv> {

    public:
        StructSymbol(
            const lyric_common::SymbolUrl &structUrl,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive,
            bool isAbstract,
            StructAddress address,
            TypeHandle *structType,
            StructSymbol *superStruct,
            BlockHandle *parentBlock,
            AssemblyState *state);
        StructSymbol(
            const lyric_common::SymbolUrl &structUrl,
            lyric_importer::StructImport *structImport,
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

        TypeHandle *structType() const;
        StructSymbol *superStruct() const;
        BlockHandle *structBlock() const;

        /*
         * struct member management
         */
        bool hasMember(const std::string &name) const;
        Option<SymbolBinding> getMember(const std::string &name) const;
        absl::flat_hash_map<std::string, SymbolBinding>::const_iterator membersBegin() const;
        absl::flat_hash_map<std::string, SymbolBinding>::const_iterator membersEnd() const;
        tu_uint32 numMembers() const;

        tempo_utils::Result<SymbolBinding> declareMember(
            const std::string &name,
            const lyric_parser::Assignable &memberSpec,
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
         * struct constructor management
         */
        lyric_common::SymbolUrl getCtor() const;
        tu_uint32 getAllocatorTrap() const;
        tempo_utils::Result<lyric_common::SymbolUrl> declareCtor(
            const std::vector<ParameterSpec> &parameterSpec,
            const Option<ParameterSpec> &restSpec,
            lyric_object::AccessType access,
            tu_uint32 allocatorTrap = lyric_runtime::INVALID_ADDRESS_U32);
        tempo_utils::Result<CtorInvoker> resolveCtor();

        /*
         * struct method management
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
            const lyric_parser::Assignable &returnSpec);
        tempo_utils::Result<MethodInvoker> resolveMethod(
            const std::string &name,
            const lyric_common::TypeDef &receiverType,
            bool isReceiver = false) const;

        /*
         * subtype tracking for sealed struct
         */
        bool hasSealedType(const lyric_common::TypeDef &sealedType) const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesBegin() const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesEnd() const;
        tempo_utils::Status putSealedType(const lyric_common::TypeDef &sealedType);

    private:
        lyric_common::SymbolUrl m_structUrl;
        lyric_importer::StructImport *m_structImport = nullptr;
        AssemblyState *m_state;

        StructSymbolPriv *load() override;
    };

    static inline const StructSymbol *cast_symbol_to_struct(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::STRUCT);
        return static_cast<const StructSymbol *>(sym);      // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline StructSymbol *cast_symbol_to_struct(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::STRUCT);
        return static_cast<StructSymbol *>(sym);            // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_STRUCT_SYMBOL_H
