#ifndef LYRIC_ASSEMBLER_TYPE_CACHE_H
#define LYRIC_ASSEMBLER_TYPE_CACHE_H

#include <lyric_importer/template_import.h>
#include <lyric_importer/type_import.h>

#include "assembler_tracer.h"
#include "assembler_types.h"
#include "template_handle.h"
#include "type_handle.h"

namespace lyric_assembler {

    class TypeCache {
    public:
        TypeCache(AssemblyState *assemblyState, AssemblerTracer *tracer);
        ~TypeCache();

        bool hasType(const lyric_common::TypeDef &assignableType) const;
        //TypeHandle *getType(const lyric_common::TypeDef &assignableType) const;
        TypeHandle *getType(TypeAddress typeAddress) const;
        std::vector<TypeHandle *>::const_iterator typesBegin() const;
        std::vector<TypeHandle *>::const_iterator typesEnd() const;
        int numTypes() const;

        tempo_utils::Result<TypeHandle *> getOrMakeType(const lyric_common::TypeDef &assignableType);
        tempo_utils::Result<TypeHandle *> importType(lyric_importer::TypeImport *typeImport);
        tempo_utils::Status touchType(TypeHandle *typeHandle);
        tempo_utils::Status touchType(const lyric_common::TypeDef &type);

        tempo_utils::Result<TypeHandle *> declareSubType(
            const lyric_common::SymbolUrl &subUrl,
            const std::vector<lyric_common::TypeDef> &subTypeArguments,
            const lyric_common::TypeDef &superType);

        tempo_utils::Result<TypeHandle *> declareParameterizedType(
            const lyric_common::SymbolUrl &baseUrl,
            const std::vector<lyric_common::TypeDef> &typeArguments);

        tempo_utils::Result<TypeHandle *> declareFunctionType(
            const lyric_common::TypeDef &functionReturn,
            const std::vector<lyric_object::Parameter> &functionParameters,
            const Option<lyric_object::Parameter> &functionRest);

        bool hasTemplate(const lyric_common::SymbolUrl &templateUrl) const;
        //TemplateHandle *getTemplate(const lyric_common::SymbolUrl &templateUrl) const;
        tempo_utils::Result<TemplateHandle *> getOrImportTemplate(const lyric_common::SymbolUrl &templateUrl);
        std::vector<TemplateHandle *>::const_iterator templatesBegin() const;
        std::vector<TemplateHandle *>::const_iterator templatesEnd() const;
        int numTemplates() const;

        tempo_utils::Status makeTemplate(
            const lyric_common::SymbolUrl &templateUrl,
            const std::vector<lyric_object::TemplateParameter> &templateParameters,
            BlockHandle *parentBlock);
        tempo_utils::Result<TemplateHandle *> importTemplate(lyric_importer::TemplateImport *templateImport);
        tempo_utils::Status touchTemplateParameters(const std::vector<lyric_object::TemplateParameter> &parameters);

        tempo_utils::Result<lyric_common::TypeDef> resolveConcrete(
            const lyric_common::SymbolUrl &concreteUrl, const std::vector<lyric_common::TypeDef> &typeArguments = {});

        tempo_utils::Result<lyric_common::TypeDef> resolveUnion(
            const std::vector<lyric_common::TypeDef> &unionMembers);

        tempo_utils::Result<lyric_common::TypeDef> resolveIntersection(
            const std::vector<lyric_common::TypeDef> &intersectionMembers);

        tempo_utils::Result<TypeSignature> resolveSignature(
            const lyric_common::SymbolUrl &symbolUrl) const;

    private:
        AssemblyState *m_assemblyState;
        AssemblerTracer *m_tracer;
        std::vector<TypeHandle *> m_types;
        absl::flat_hash_map<lyric_common::TypeDef, TypeHandle *> m_typecache;
        std::vector<TemplateHandle *> m_templates;
        absl::flat_hash_map<lyric_common::SymbolUrl, TemplateHandle *> m_templatecache;
    };
}

#endif // LYRIC_ASSEMBLER_TYPE_CACHE_H
