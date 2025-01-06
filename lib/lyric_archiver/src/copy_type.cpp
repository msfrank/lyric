
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/archive_utils.h>
#include <lyric_archiver/copy_type.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/type_cache.h>

struct CopyTypeData {
    lyric_common::ModuleLocation location;
    lyric_assembler::NamespaceSymbol *targetNamespace;
    std::string importHash;
    lyric_archiver::SymbolReferenceSet *symbolReferenceSet;

};

static tempo_utils::Result<lyric_common::TypeDef>
update_type(const lyric_common::TypeDef &typeDef, CopyTypeData &data);

static tempo_utils::Result<lyric_common::TypeDef>
update_concrete_type(const lyric_common::TypeDef &concreteType, CopyTypeData &data)
{
    auto concreteUrl = concreteType.getConcreteUrl();
    if (concreteUrl.getModuleLocation() == data.location) {
        data.symbolReferenceSet->insertReference(concreteUrl);
        concreteUrl = lyric_archiver::build_relative_url(data.importHash, concreteUrl);
    }
    std::vector<lyric_common::TypeDef> concreteArguments;
    for (const auto &argType : concreteType.getConcreteArguments()) {
        lyric_common::TypeDef copiedType;
        TU_ASSIGN_OR_RETURN (copiedType, update_type(argType, data));
        concreteArguments.push_back(std::move(copiedType));
    }
    return lyric_common::TypeDef::forConcrete(concreteUrl, concreteArguments);
}

static tempo_utils::Result<lyric_common::TypeDef>
update_placeholder_type(const lyric_common::TypeDef &placeholderType, CopyTypeData &data)
{
    auto placeholderIndex = placeholderType.getPlaceholderIndex();
    auto templateUrl = placeholderType.getPlaceholderTemplateUrl();
    if (templateUrl.getModuleLocation() == data.location) {
        data.symbolReferenceSet->insertReference(templateUrl);
        templateUrl = lyric_archiver::build_relative_url(data.importHash, templateUrl);
    }
    std::vector<lyric_common::TypeDef> placeholderArguments;
    for (const auto &argType : placeholderType.getPlaceholderArguments()) {
        lyric_common::TypeDef copiedType;
        TU_ASSIGN_OR_RETURN (copiedType, update_type(argType, data));
        placeholderArguments.push_back(std::move(copiedType));
    }
    return lyric_common::TypeDef::forPlaceholder(placeholderIndex, templateUrl, placeholderArguments);
}

static tempo_utils::Result<lyric_common::TypeDef>
update_intersection_type(const lyric_common::TypeDef &intersectionType, CopyTypeData &data)
{
    std::vector<lyric_common::TypeDef> intersectionMembers;
    for (const auto &memberType : intersectionType.getIntersectionMembers()) {
        lyric_common::TypeDef copiedType;
        TU_ASSIGN_OR_RETURN (copiedType, update_type(memberType, data));
        intersectionMembers.push_back(std::move(copiedType));
    }
    return lyric_common::TypeDef::forIntersection(intersectionMembers);
}

static tempo_utils::Result<lyric_common::TypeDef>
update_union_type(const lyric_common::TypeDef &unionType, CopyTypeData &data)
{
    std::vector<lyric_common::TypeDef> unionMembers;
    for (const auto &memberType : unionType.getIntersectionMembers()) {
        lyric_common::TypeDef copiedType;
        TU_ASSIGN_OR_RETURN (copiedType, update_type(memberType, data));
        unionMembers.push_back(std::move(copiedType));
    }
    return lyric_common::TypeDef::forUnion(unionMembers);
}

static tempo_utils::Result<lyric_common::TypeDef>
update_type(const lyric_common::TypeDef &typeDef, CopyTypeData &data)
{
    switch (typeDef.getType()) {
        case lyric_common::TypeDefType::Concrete:
            return update_concrete_type(typeDef, data);
        case lyric_common::TypeDefType::Placeholder:
            return update_placeholder_type(typeDef, data);
        case lyric_common::TypeDefType::Intersection:
            return update_intersection_type(typeDef, data);
        case lyric_common::TypeDefType::Union:
            return update_union_type(typeDef, data);

        case lyric_common::TypeDefType::NoReturn:
        case lyric_common::TypeDefType::Invalid:
            return typeDef;
    }
}

tempo_utils::Result<lyric_assembler::TypeHandle *>
lyric_archiver::copy_type(
    lyric_importer::TypeImport *typeImport,
    const std::string &importHash,
    lyric_assembler::NamespaceSymbol *targetNamespace,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    auto moduleImport = typeImport->getModuleImport();

    CopyTypeData data;
    data.location = moduleImport->getLocation();
    data.targetNamespace = targetNamespace;
    data.importHash = importHash;
    data.symbolReferenceSet = &symbolReferenceSet;

    lyric_common::TypeDef copiedType;
    TU_ASSIGN_OR_RETURN (copiedType, update_type(typeImport->getTypeDef(), data));

    auto *objectState = archiverState.objectState();
    auto *typeCache = objectState->typeCache();

    return typeCache->getOrMakeType(copiedType);
}