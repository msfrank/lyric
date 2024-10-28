#ifndef LYRIC_ASSEMBLER_OBJECT_ROOT_H
#define LYRIC_ASSEMBLER_OBJECT_ROOT_H

#include "object_state.h"

namespace lyric_assembler {

    class BlockHandle;

    class ObjectRoot {
    public:
        explicit ObjectRoot(ObjectState *state);

        tempo_utils::Status initialize(std::shared_ptr<lyric_importer::ModuleImport> preludeImport);

        BlockHandle *rootBlock();
        NamespaceSymbol *globalNamespace();
        CallSymbol *entryCall();

//        tempo_utils::Result<DataReference> declareStatic(
//            const std::string &name,
//            lyric_object::AccessType access,
//            const lyric_common::TypeDef &assignableType,
//            bool isVariable,
//            bool declOnly = false);
//
//        tempo_utils::Result<CallSymbol *> declareFunction(
//            const std::string &name,
//            lyric_object::AccessType access,
//            const std::vector<lyric_object::TemplateParameter> &templateParameters,
//            bool declOnly = false);
//
//        tempo_utils::Result<ClassSymbol *> declareClass(
//            const std::string &name,
//            ClassSymbol *superClass,
//            lyric_object::AccessType access,
//            const std::vector<lyric_object::TemplateParameter> &templateParameters,
//            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
//            bool isAbstract = false,
//            bool declOnly = false);
//
//        tempo_utils::Result<ConceptSymbol *> declareConcept(
//            const std::string &name,
//            ConceptSymbol *superConcept,
//            lyric_object::AccessType access,
//            const std::vector<lyric_object::TemplateParameter> &templateParameters,
//            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
//            bool declOnly = false);
//
//        tempo_utils::Result<EnumSymbol *> declareEnum(
//            const std::string &name,
//            EnumSymbol *superEnum,
//            lyric_object::AccessType access,
//            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
//            bool isAbstract = false,
//            bool declOnly = false);
//
//        tempo_utils::Result<InstanceSymbol *> declareInstance(
//            const std::string &name,
//            InstanceSymbol *superInstance,
//            lyric_object::AccessType access,
//            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
//            bool isAbstract = false,
//            bool declOnly = false);
//
//        tempo_utils::Result<StructSymbol *> declareStruct(
//            const std::string &name,
//            StructSymbol *superStruct,
//            lyric_object::AccessType access,
//            lyric_object::DeriveType derive = lyric_object::DeriveType::Any,
//            bool isAbstract = false,
//            bool declOnly = false);
//
//        tempo_utils::Status useSymbol(
//            const lyric_common::SymbolUrl &symbolUrl,
//            const absl::flat_hash_set<lyric_common::TypeDef> &implTypes = {});

    private:
        ObjectState *m_state;
        std::unique_ptr<BlockHandle> m_preludeBlock;
        std::unique_ptr<BlockHandle> m_rootBlock;
        NamespaceSymbol *m_globalNamespace;
        CallSymbol *m_entryCall;
    };
}

#endif // LYRIC_ASSEMBLER_OBJECT_ROOT_H
