#ifndef LYRIC_ASSEMBLER_TYPE_CACHE_H
#define LYRIC_ASSEMBLER_TYPE_CACHE_H

#include <lyric_importer/template_import.h>
#include <lyric_importer/type_import.h>

#include "assembler_types.h"
#include "template_handle.h"
#include "type_handle.h"

namespace lyric_assembler {

    class TypeCache {
    public:
        explicit TypeCache(ObjectState *objectState);
        ~TypeCache();

        bool hasType(const lyric_common::TypeDef &typeDef) const;
        TypeHandle *getType(const lyric_common::TypeDef &typeDef) const;
        int numTypes() const;

        tempo_utils::Result<TypeHandle *> getOrMakeType(const lyric_common::TypeDef &assignableType);
        tempo_utils::Status appendType(TypeHandle *typeHandle);
        tempo_utils::Result<TypeHandle *> importType(lyric_importer::TypeImport *typeImport);

        tempo_utils::Result<TypeHandle *> declareSubType(
            const lyric_common::SymbolUrl &subTypeUrl,
            const std::vector<lyric_common::TypeDef> &subTypePlaceholders,
            const lyric_common::TypeDef &superType);

        tempo_utils::Result<TypeHandle *> declareParameterizedType(
            const lyric_common::SymbolUrl &baseUrl,
            const std::vector<lyric_common::TypeDef> &typeArguments);

        tempo_utils::Result<TypeHandle *> declareFunctionType(
            const lyric_common::TypeDef &functionReturn,
            const std::vector<lyric_common::TypeDef> &functionParameters,
            const Option<lyric_common::TypeDef> &functionRest);

        bool hasTemplate(const lyric_common::SymbolUrl &templateUrl) const;
        TemplateHandle *getTemplate(const lyric_common::SymbolUrl &templateUrl) const;
        int numTemplates() const;

        tempo_utils::Result<TemplateHandle *> getOrImportTemplate(const lyric_common::SymbolUrl &templateUrl);

        tempo_utils::Result<TemplateHandle *> makeTemplate(
            const lyric_common::SymbolUrl &templateUrl,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            BlockHandle *parentBlock);
        tempo_utils::Result<TemplateHandle *> makeTemplate(
            const lyric_common::SymbolUrl &templateUrl,
            TemplateHandle *superTemplate,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            BlockHandle *parentBlock);
        tempo_utils::Result<TemplateHandle *> importTemplate(lyric_importer::TemplateImport *templateImport);
        tempo_utils::Status appendTemplate(TemplateHandle *templateHandle);
        tempo_utils::Status touchTemplateParameters(const std::vector<lyric_object::TemplateParameter> &parameters);

        tempo_utils::Result<lyric_common::TypeDef> resolveConcrete(
            const lyric_common::SymbolUrl &concreteUrl, const std::vector<lyric_common::TypeDef> &typeArguments = {});

        tempo_utils::Result<lyric_common::TypeDef> resolveUnion(
            const std::vector<lyric_common::TypeDef> &unionMembers);

        tempo_utils::Result<lyric_common::TypeDef> resolveIntersection(
            const std::vector<lyric_common::TypeDef> &intersectionMembers);

        tempo_utils::Result<TypeSignature> resolveSignature(const lyric_common::TypeDef &typeDef);
        tempo_utils::Result<TypeSignature> resolveSignature(const lyric_common::SymbolUrl &symbolUrl);
        tempo_utils::Result<TypeSignature> resolveSignature(const TypeHandle *typeHandle);

    private:
        ObjectState *m_objectState;
        absl::flat_hash_map<lyric_common::TypeDef, TypeHandle *> m_typecache;
        absl::flat_hash_map<lyric_common::SymbolUrl, TemplateHandle *> m_templatecache;
        absl::flat_hash_map<lyric_common::TypeDef, TypeSignature> m_signaturecache;
    };
}

#endif // LYRIC_ASSEMBLER_TYPE_CACHE_H
