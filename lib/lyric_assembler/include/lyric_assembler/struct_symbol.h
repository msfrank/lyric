#ifndef LYRIC_ASSEMBLER_STRUCT_SYMBOL_H
#define LYRIC_ASSEMBLER_STRUCT_SYMBOL_H

#include <lyric_importer/struct_import.h>

#include "abstract_member_reifier.h"
#include "abstract_symbol.h"
#include "object_state.h"
#include "base_symbol.h"
#include "callable_invoker.h"
#include "constructable_invoker.h"
#include "impl_handle.h"
#include "type_handle.h"

namespace lyric_assembler {

    struct StructSymbolPriv {
        bool isHidden = false;
        lyric_object::DeriveType derive = lyric_object::DeriveType::Invalid;
        bool isDeclOnly = false;
        TypeHandle *structType = nullptr;
        StructSymbol *superStruct = nullptr;
        std::string allocatorTrap;
        absl::flat_hash_map<std::string, DataReference> members;
        absl::flat_hash_set<std::string> initializedMembers;
        absl::flat_hash_map<std::string, BoundMethod> methods;
        absl::flat_hash_map<lyric_common::SymbolUrl, ImplHandle *> impls;
        absl::flat_hash_set<lyric_common::TypeDef> sealedTypes;
        std::unique_ptr<BlockHandle> structBlock;
    };

    class StructSymbol : public BaseSymbol<StructSymbolPriv> {

    public:
        StructSymbol(
            const lyric_common::SymbolUrl &structUrl,
            bool isHidden,
            lyric_object::DeriveType derive,
            TypeHandle *structType,
            StructSymbol *superStruct,
            bool isDeclOnly,
            BlockHandle *parentBlock,
            ObjectState *state);
        StructSymbol(
            const lyric_common::SymbolUrl &structUrl,
            lyric_importer::StructImport *structImport,
            bool isCopied,
            ObjectState *state);

        lyric_object::LinkageSection getLinkage() const override;

        SymbolType getSymbolType() const override;
        lyric_common::SymbolUrl getSymbolUrl() const override;
        lyric_common::TypeDef getTypeDef() const override;

        bool isHidden() const;
        lyric_object::DeriveType getDeriveType() const;
        bool isDeclOnly() const;

        TypeHandle *structType() const;
        StructSymbol *superStruct() const;
        BlockHandle *structBlock() const;

        /*
         * struct member management
         */
        bool hasMember(const std::string &name) const;
        Option<DataReference> getMember(const std::string &name) const;
        absl::flat_hash_map<std::string, DataReference>::const_iterator membersBegin() const;
        absl::flat_hash_map<std::string, DataReference>::const_iterator membersEnd() const;
        tu_uint32 numMembers() const;

        tempo_utils::Result<FieldSymbol *> declareMember(
            const std::string &name,
            const lyric_common::TypeDef &memberType,
            bool isHidden);

        tempo_utils::Result<DataReference> resolveMember(
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
        std::string getAllocatorTrap() const;
        tempo_utils::Result<CallSymbol *> declareCtor(
            bool isHidden,
            std::string allocatorTrap = {});
        tempo_utils::Status prepareCtor(ConstructableInvoker &invoker);

        /*
         * struct method management
         */
        bool hasMethod(const std::string &name) const;
        Option<BoundMethod> getMethod(const std::string &name) const;
        absl::flat_hash_map<std::string, BoundMethod>::const_iterator methodsBegin() const;
        absl::flat_hash_map<std::string, BoundMethod>::const_iterator methodsEnd() const;
        tu_uint32 numMethods() const;

        tempo_utils::Result<CallSymbol *> declareMethod(
            const std::string &name,
            bool isHidden,
            bool isFinal = false);
        tempo_utils::Status prepareMethod(
            const std::string &name,
            const lyric_common::TypeDef &receiverType,
            CallableInvoker &invoker,
            bool isReceiver = false) const;

        /*
         * struct impl management
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
         * subtype tracking for sealed struct
         */
        bool hasSealedType(const lyric_common::TypeDef &sealedType) const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesBegin() const;
        absl::flat_hash_set<lyric_common::TypeDef>::const_iterator sealedTypesEnd() const;
        tempo_utils::Status putSealedType(const lyric_common::TypeDef &sealedType);

    private:
        lyric_common::SymbolUrl m_structUrl;
        lyric_importer::StructImport *m_structImport = nullptr;
        ObjectState *m_state;

        StructSymbolPriv *load() override;
    };

    inline const StructSymbol *cast_symbol_to_struct(const AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::STRUCT);
        return static_cast<const StructSymbol *>(sym);      // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }

    inline StructSymbol *cast_symbol_to_struct(AbstractSymbol *sym) {
        TU_ASSERT (sym->getSymbolType() == SymbolType::STRUCT);
        return static_cast<StructSymbol *>(sym);            // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    }
}

#endif // LYRIC_ASSEMBLER_STRUCT_SYMBOL_H
