#ifndef LYRIC_ASSEMBLER_CLASS_SYMBOL_H
#define LYRIC_ASSEMBLER_CLASS_SYMBOL_H

#include <absl/container/flat_hash_map.h>

#include <lyric_importer/class_import.h>

#include "abstract_member_reifier.h"
#include "abstract_symbol.h"
#include "assembly_state.h"
#include "base_symbol.h"
#include "call_invoker.h"
#include "ctor_invoker.h"
#include "method_invoker.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct ClassSymbolPriv {
        lyric_object::AccessType access = lyric_object::AccessType::Invalid;
        lyric_object::DeriveType derive = lyric_object::DeriveType::Invalid;
        bool isAbstract = false;
        TypeHandle *classType = nullptr;
        TemplateHandle *classTemplate = nullptr;
        ClassSymbol *superClass = nullptr;
        tu_uint32 allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
        absl::flat_hash_map<std::string, DataReference> members;
        absl::flat_hash_set<std::string> initializedMembers;
        absl::flat_hash_map<std::string, BoundMethod> methods;
        absl::flat_hash_map<lyric_common::SymbolUrl, ImplHandle *> impls;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        std::unique_ptr<BlockHandle> classBlock;
    };

    class ClassSymbol : public BaseSymbol<ClassAddress,ClassSymbolPriv> {
    public:
        ClassSymbol(
            const lyric_common::SymbolUrl &classUrl,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive,
            bool isAbstract,
            ClassAddress address,
            TypeHandle *classType,
            ClassSymbol *superClass,
            BlockHandle *parentBlock,
            AssemblyState *state);

        ClassSymbol(
            const lyric_common::SymbolUrl &classUrl,
            lyric_object::AccessType access,
            lyric_object::DeriveType derive,
            bool isAbstract,
            ClassAddress address,
            TypeHandle *classType,
            TemplateHandle *classTemplate,
            ClassSymbol *superClass,
            BlockHandle *parentBlock,
            AssemblyState *state);

        ClassSymbol(
            const lyric_common::SymbolUrl &classUrl,
            lyric_importer::ClassImport *classImport,
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

        ClassSymbol *superClass() const;
        TypeHandle *classType() const;
        TemplateHandle *classTemplate() const;
        BlockHandle *classBlock() const;

        /*
         * class member management
         */
        bool hasMember(const std::string &name) const;
        Option<DataReference> getMember(const std::string &name) const;
        absl::flat_hash_map<std::string, DataReference>::const_iterator membersBegin() const;
        absl::flat_hash_map<std::string, DataReference>::const_iterator membersEnd() const;
        tu_uint32 numMembers() const;

        tempo_utils::Result<DataReference> declareMember(
            const std::string &name,
            const lyric_parser::Assignable &memberSpec,
            bool isVariable,
            lyric_object::AccessType access,
            const lyric_common::SymbolUrl &init = {});

        tempo_utils::Result<DataReference> resolveMember(
            const std::string &name,
            AbstractMemberReifier &reifier,
            const lyric_common::TypeDef &receiverType,
            bool thisReceiver = false) const;

        bool isMemberInitialized(const std::string &name) const;
        tempo_utils::Status setMemberInitialized(const std::string &name);
        bool isCompletelyInitialized() const;

        /*
         * class constructor management
         */
        lyric_common::SymbolUrl getCtor() const;
        tu_uint32 getAllocatorTrap() const;
        tempo_utils::Result<lyric_common::SymbolUrl> declareCtor(
            const std::vector<ParameterSpec> &parameterSpec,
            const Option<ParameterSpec> &restSpec,
            const std::vector<ParameterSpec> &ctxSpec,
            lyric_object::AccessType access,
            tu_uint32 allocatorTrap = lyric_object::INVALID_ADDRESS_U32);
        tempo_utils::Result<CtorInvoker> resolveCtor();

        /*
         * class method management
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
            bool thisReceiver = false) const;

        /*
         * class impl management
         */
        bool hasImpl(const lyric_common::SymbolUrl &implUrl) const;
        bool hasImpl(const lyric_common::TypeDef &implType) const;
        ImplHandle *getImpl(const lyric_common::SymbolUrl &implUrl) const;
        ImplHandle *getImpl(const lyric_common::TypeDef &implType) const;
        absl::flat_hash_map<lyric_common::SymbolUrl, ImplHandle *>::const_iterator implsBegin() const;
        absl::flat_hash_map<lyric_common::SymbolUrl, ImplHandle *>::const_iterator implsEnd() const;
        tu_uint32 numImpls() const;

        tempo_utils::Result<lyric_common::TypeDef> declareImpl(const lyric_parser::Assignable &implSpec);

        /*
         * subtype tracking for sealed class
         */
        bool hasSealedType(const lyric_common::TypeDef &sealedType) const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesBegin() const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesEnd() const;
        tempo_utils::Status putSealedType(const lyric_common::TypeDef &sealedType);

    private:
        lyric_common::SymbolUrl m_classUrl;
        lyric_importer::ClassImport *m_classImport = nullptr;
        AssemblyState *m_state;

        ClassSymbolPriv *load() override;
    };

    static inline const ClassSymbol *cast_symbol_to_class(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::CLASS);
        return static_cast<const ClassSymbol *>(sym);   // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    static inline ClassSymbol *cast_symbol_to_class(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::CLASS);
        return static_cast<ClassSymbol *>(sym);         // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_CLASS_SYMBOL_H
