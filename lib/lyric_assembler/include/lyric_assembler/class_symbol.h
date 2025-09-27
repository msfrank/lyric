#ifndef LYRIC_ASSEMBLER_CLASS_SYMBOL_H
#define LYRIC_ASSEMBLER_CLASS_SYMBOL_H

#include <absl/container/flat_hash_map.h>

#include <lyric_importer/class_import.h>

#include "abstract_member_reifier.h"
#include "abstract_symbol.h"
#include "object_state.h"
#include "base_symbol.h"
#include "callable_invoker.h"
#include "constructable_invoker.h"
#include "impl_handle.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct ClassSymbolPriv {
        bool isHidden = false;
        lyric_object::DeriveType derive = lyric_object::DeriveType::Invalid;
        bool isDeclOnly = false;
        TypeHandle *classType = nullptr;
        TemplateHandle *classTemplate = nullptr;
        ClassSymbol *superClass = nullptr;
        std::string allocatorTrap;
        absl::flat_hash_map<std::string, DataReference> members;
        absl::flat_hash_set<std::string> initializedMembers;
        absl::flat_hash_map<std::string, BoundMethod> methods;
        absl::flat_hash_map<lyric_common::SymbolUrl, ImplHandle *> impls;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        std::unique_ptr<BlockHandle> classBlock;
    };

    class ClassSymbol : public BaseSymbol<ClassSymbolPriv> {
    public:
        ClassSymbol(
            const lyric_common::SymbolUrl &classUrl,
            bool isHidden,
            lyric_object::DeriveType derive,
            TypeHandle *classType,
            ClassSymbol *superClass,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);

        ClassSymbol(
            const lyric_common::SymbolUrl &classUrl,
            bool isHidden,
            lyric_object::DeriveType derive,
            TypeHandle *classType,
            TemplateHandle *classTemplate,
            ClassSymbol *superClass,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);

        ClassSymbol(
            const lyric_common::SymbolUrl &classUrl,
            lyric_importer::ClassImport *classImport,
            bool isCopied,
            ObjectState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        bool isHidden() const;
        lyric_object::DeriveType getDeriveType() const;
        bool isDeclOnly() const;

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

        tempo_utils::Result<FieldSymbol *> declareMember(
            const std::string &name,
            const lyric_common::TypeDef &memberType,
            bool isVariable,
            bool isHidden);

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
        std::string getAllocatorTrap() const;
        bool hasCtor(const std::string &name) const;
        lyric_common::SymbolUrl getCtor(const std::string &name) const;
        tempo_utils::Result<CallSymbol *> declareCtor(
            const std::string &name,
            bool isHidden,
            std::string allocatorTrap = {});
        tempo_utils::Status prepareCtor(const std::string &name, ConstructableInvoker &invoker);

        /*
         * class method management
         */
        bool hasMethod(const std::string &name) const;
        Option<BoundMethod> getMethod(const std::string &name) const;
        absl::flat_hash_map<std::string, BoundMethod>::const_iterator methodsBegin() const;
        absl::flat_hash_map<std::string, BoundMethod>::const_iterator methodsEnd() const;
        tu_uint32 numMethods() const;

        tempo_utils::Result<CallSymbol *> declareMethod(
            const std::string &name,
            bool isHidden,
            DispatchType dispatch = DispatchType::Virtual,
            const std::vector<lyric_object::TemplateParameter> &templateParameters = {});

        tempo_utils::Status prepareMethod(
            const std::string &name,
            const lyric_common::TypeDef &receiverType,
            CallableInvoker &invoker,
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

        tempo_utils::Result<ImplHandle *> declareImpl(const lyric_common::TypeDef &implType);

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
        ObjectState *m_state;

        ClassSymbolPriv *load() override;
    };

    inline const ClassSymbol *cast_symbol_to_class(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::CLASS);
        return static_cast<const ClassSymbol *>(sym);   // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    inline ClassSymbol *cast_symbol_to_class(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::CLASS);
        return static_cast<ClassSymbol *>(sym);         // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_CLASS_SYMBOL_H
