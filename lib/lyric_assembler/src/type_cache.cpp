
#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/lexical_variable.h>
#include <lyric_assembler/local_variable.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/object_state.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/typename_symbol.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/type_handle.h>
#include <lyric_assembler/type_set.h>

lyric_assembler::TypeCache::TypeCache(ObjectState *objectState)
    : m_objectState(objectState)
{
    TU_ASSERT (m_objectState != nullptr);
}

lyric_assembler::TypeCache::~TypeCache()
{
    for (auto &pair : m_typecache) {
        delete pair.second;
    }
    for (auto &ptr : m_templatecache) {
        delete ptr.second;
    }
}

bool
lyric_assembler::TypeCache::hasType(const lyric_common::TypeDef &typeDef) const
{
    return m_typecache.contains(typeDef);
}

lyric_assembler::TypeHandle *
lyric_assembler::TypeCache::getType(const lyric_common::TypeDef &typeDef) const
{
    auto entry = m_typecache.find(typeDef);
    if (entry != m_typecache.cend())
        return entry->second;
    return nullptr;
}

int
lyric_assembler::TypeCache::numTypes() const
{
    return m_typecache.size();
}

/**
 * Creates a type handle for a definition. The type definition is concrete and the symbol url identifies
 * the symbol for the definition in the appropriate descriptor section. If the definition is generic then
 * the type definition contains the type arguments which are the template parameters to the definition.
 *
 * @param typeDef
 * @param concreteTypeHandle
 * @param typecache
 * @param state
 * @return
 */
static tempo_utils::Result<lyric_assembler::TypeHandle *>
make_definition_type(
    const lyric_common::TypeDef &typeDef,
    lyric_assembler::TypeHandle *concreteTypeHandle,
    absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::TypeHandle *> &typecache,
    lyric_assembler::ObjectState *state)
{
    // if concrete type is not parameterized then we are done, return the handle
    auto it = typeDef.concreteArgumentsBegin();
    if (it == typeDef.concreteArgumentsEnd())
        return concreteTypeHandle;

    auto *typeCache = state->typeCache();

    // otherwise ensure all type arguments exist in the cache
    for (; it != typeDef.concreteArgumentsEnd(); it++) {
        TU_RETURN_IF_STATUS(typeCache->getOrMakeType(*it));
    }

    auto superTypeHandle = concreteTypeHandle->getSuperType();

    auto *typeHandle = new lyric_assembler::TypeHandle(typeDef, superTypeHandle);
    typecache[typeDef] = typeHandle;
    return typeHandle;
}

/**
 * Creates a type handle for a specific function. The type definition is concrete and the symbol url
 * identifies the call symbol entry for the function. If the function is generic then the type definition
 * contains the type arguments which are the template parameters to the function. A function type does
 * not have a supertype.
 *
 * @param typeDef
 * @param typecache
 * @param state
 * @return
 */
static tempo_utils::Result<lyric_assembler::TypeHandle *>
make_function_type(
    const lyric_common::TypeDef &typeDef,
    lyric_assembler::CallSymbol *callSymbol,
    absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::TypeHandle *> &typecache,
    lyric_assembler::ObjectState *state)
{
    auto *typeCache = state->typeCache();

    auto functionUrl = typeDef.getConcreteUrl();
    std::vector functionArguments(typeDef.concreteArgumentsBegin(), typeDef.concreteArgumentsEnd());
    for (auto &argType: functionArguments) {
        TU_RETURN_IF_STATUS(typeCache->getOrMakeType(argType));
    }

    auto *typeHandle = new lyric_assembler::TypeHandle(functionUrl, functionArguments, callSymbol);
    typecache[typeDef] = typeHandle;
    return typeHandle;
}

/**
 * Creates a type handle for a specific global variable. The type definition is concrete and the symbol
 * url identifies the static symbol entry for the global variable. A global type does not have a supertype.
 *
 * @param typeDef
 * @param typecache
 * @param state
 * @return
 */
static tempo_utils::Result<lyric_assembler::TypeHandle *>
make_global_type(
    const lyric_common::TypeDef &typeDef,
    lyric_assembler::StaticSymbol *staticSymbol,
    absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::TypeHandle *> &typecache)
{
    auto globalUrl = typeDef.getConcreteUrl();
    auto *typeHandle = new lyric_assembler::TypeHandle(globalUrl, {}, staticSymbol);
    typecache[typeDef] = typeHandle;
    return typeHandle;
}

/**
 *
 * @param typeDef
 * @param linkageSymbol
 * @param typecache
 * @return
 */
static tempo_utils::Result<lyric_assembler::TypeHandle *>
make_typename_type(
    const lyric_common::TypeDef &typeDef,
    lyric_assembler::TypenameSymbol *typenameSymbol,
    absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::TypeHandle *> &typecache)
{
    auto typenameUrl = typeDef.getConcreteUrl();
    auto *typeHandle = new lyric_assembler::TypeHandle(typenameUrl, {}, typenameSymbol);
    typecache[typeDef] = typeHandle;
    return typeHandle;
}


/**
 * Creates a type handle for the specified type if it does not exist in the type cache. The resulting
 * type handle is not touched.
 *
 * @param assignableType The type to insert into the typecache.
 * @returns A `tempo_utils::Result` containing the type handle for the type.
 */
tempo_utils::Result<lyric_assembler::TypeHandle *>
lyric_assembler::TypeCache::getOrMakeType(const lyric_common::TypeDef &assignableType)
{
    if (!assignableType.isValid())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "failed to make type {}; invalid type", assignableType.toString());

    // if type exists in the cache already then return it
    auto entry = m_typecache.find(assignableType);
    if (entry != m_typecache.cend())
        return entry->second;

    // if type is not in the cache then make the type handle for the type
    switch (assignableType.getType()) {

        case lyric_common::TypeDefType::Concrete: {
            auto *symbolCache = m_objectState->symbolCache();

            // get or import the type handle for the concrete url (which inserts the handle into the cache)
            AbstractSymbol *sym;
            TU_ASSIGN_OR_RETURN (sym, symbolCache->getOrImportSymbol(assignableType.getConcreteUrl()));

            switch (sym->getSymbolType()) {
                case SymbolType::CLASS:
                    return make_definition_type(assignableType,
                        cast_symbol_to_class(sym)->classType(), m_typecache, m_objectState);
                case SymbolType::CONCEPT:
                    return make_definition_type(assignableType,
                        cast_symbol_to_concept(sym)->conceptType(), m_typecache, m_objectState);
                case SymbolType::ENUM:
                    return make_definition_type(assignableType,
                        cast_symbol_to_enum(sym)->enumType(), m_typecache, m_objectState);
                case SymbolType::EXISTENTIAL:
                    return make_definition_type(assignableType,
                        cast_symbol_to_existential(sym)->existentialType(), m_typecache, m_objectState);
                case SymbolType::INSTANCE:
                    return make_definition_type(assignableType,
                        cast_symbol_to_instance(sym)->instanceType(), m_typecache, m_objectState);
                case SymbolType::STRUCT:
                    return make_definition_type(assignableType,
                        cast_symbol_to_struct(sym)->structType(), m_typecache, m_objectState);

                case SymbolType::CALL:
                    return make_function_type(assignableType, cast_symbol_to_call(sym), m_typecache, m_objectState);
                case SymbolType::STATIC:
                    return make_global_type(assignableType, cast_symbol_to_static(sym), m_typecache);
                case SymbolType::TYPENAME:
                    return make_typename_type(assignableType, cast_symbol_to_typename(sym), m_typecache);

                default:
                    return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                        "failed to make type {}; invalid type", assignableType.toString());
            }
        }

        case lyric_common::TypeDefType::Placeholder: {
            for (auto it = assignableType.placeholderArgumentsBegin(); it != assignableType.placeholderArgumentsEnd(); it++) {
                TU_RETURN_IF_STATUS(getOrMakeType(*it));
            }
            auto templateUrl = assignableType.getPlaceholderTemplateUrl();
            TU_RETURN_IF_STATUS (getOrImportTemplate(templateUrl));
            auto *typeHandle = new TypeHandle(assignableType);
            m_typecache[assignableType] = typeHandle;
            return typeHandle;
        }

        case lyric_common::TypeDefType::Union: {
            for (auto it = assignableType.unionMembersBegin(); it != assignableType.unionMembersEnd(); it++) {
                TU_RETURN_IF_STATUS(getOrMakeType(*it));
            }
            auto *typeHandle = new TypeHandle(assignableType);
            m_typecache[assignableType] = typeHandle;
            return typeHandle;
        }

        case lyric_common::TypeDefType::Intersection: {
            for (auto it = assignableType.intersectionMembersBegin(); it != assignableType.intersectionMembersEnd(); it++) {
                TU_RETURN_IF_STATUS(getOrMakeType(*it));
            }
            auto *typeHandle = new TypeHandle(assignableType);
            m_typecache[assignableType] = typeHandle;
            return typeHandle;
        }

        case lyric_common::TypeDefType::NoReturn: {
            auto *typeHandle = new TypeHandle(assignableType);
            m_typecache[assignableType] = typeHandle;
            return typeHandle;
        }

        default:
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "failed to make type {}; invalid type", assignableType.toString());
    }
}

tempo_utils::Status
lyric_assembler::TypeCache::appendType(TypeHandle *typeHandle)
{
    TU_ASSERT (typeHandle != nullptr);
    auto typeDef = typeHandle->getTypeDef();

    if (m_typecache.contains(typeDef))
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "type {} is already defined", typeDef.toString());

    m_typecache[typeDef] = typeHandle;
    return {};
}

tempo_utils::Result<lyric_assembler::TypeHandle *>
lyric_assembler::TypeCache::importType(lyric_importer::TypeImport *typeImport)
{
    TU_ASSERT (typeImport != nullptr);

    auto assignableType = typeImport->getTypeDef();
    auto entry = m_typecache.find(assignableType);
    if (entry != m_typecache.cend())
        return entry->second;

    for (auto iterator = typeImport->argumentsBegin(); iterator != typeImport->argumentsEnd(); iterator++) {
        TU_RETURN_IF_STATUS (importType(*iterator));
    }

    TypeHandle *typeHandle;
    if (typeImport->getSuperType() != nullptr) {
        auto assignableSuperType = typeImport->getSuperType()->getTypeDef();
        TypeHandle *superTypeHandle;
        TU_ASSIGN_OR_RETURN(superTypeHandle, importType(typeImport->getSuperType()));
        typeHandle = new TypeHandle(assignableType, superTypeHandle);
    } else {
        typeHandle = new TypeHandle(assignableType);
    }

    m_typecache[assignableType] = typeHandle;
    return typeHandle;
}

/**
 * Create a type handle for the incompletely defined symbol specified by `subTypeUrl`. If `subTypePlaceholders`
 * is specified then each element must be a placeholder type. The resulting type handle is touched.
 *
 */
tempo_utils::Result<lyric_assembler::TypeHandle *>
lyric_assembler::TypeCache::declareSubType(
    const lyric_common::SymbolUrl &subTypeUrl,
    const std::vector<lyric_common::TypeDef> &subTypePlaceholders,
    const lyric_common::TypeDef &superType)
{
    TU_ASSERT (subTypeUrl.isValid());
    TU_ASSERT (superType.isValid());

    auto symbolCache = m_objectState->symbolCache();

    // verify the subtype symbol either does not exist, or exists as a typename
    auto *sym = symbolCache->getSymbolOrNull(subTypeUrl);
    if (sym != nullptr && sym->getSymbolType() != SymbolType::TYPENAME)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "symbol {} is already defined", subTypeUrl.toString());

    // ensure supertype handle exists
    auto superEntry = m_typecache.find(superType);
    if (superEntry == m_typecache.cend())
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingType,
            "missing super type {}", superType.toString());
    auto *superTypeHandle = superEntry->second;

    // ensure all subtype placeholders are valid
    for (const auto &placeholderType : subTypePlaceholders) {
        if (placeholderType.getType() != lyric_common::TypeDefType::Placeholder)
            return AssemblerStatus::forCondition(AssemblerCondition::kTypeError,
                "invalid type placeholder {}", placeholderType.toString());
        TU_RETURN_IF_STATUS (getOrMakeType(placeholderType));
    }

    auto subTypeDef = lyric_common::TypeDef::forConcrete(subTypeUrl, subTypePlaceholders);
    auto subEntry = m_typecache.find(subTypeDef);

    TypeHandle *typeHandle;
    if (subEntry == m_typecache.cend()) {
        typeHandle = new TypeHandle(subTypeUrl, subTypePlaceholders, superTypeHandle);
        m_typecache[typeHandle->getTypeDef()] = typeHandle;
    } else {
        typeHandle = subEntry->second;
        TU_RETURN_IF_NOT_OK (typeHandle->defineType(subTypePlaceholders, superTypeHandle));
    }

    return typeHandle;
}

/**
 * Create a type handle for the symbol specified by `baseUrl` with the specified `typeArguments` substituted
 * for the symbol type parameters. The resulting type handle is touched.
 */
tempo_utils::Result<lyric_assembler::TypeHandle *>
lyric_assembler::TypeCache::declareParameterizedType(
    const lyric_common::SymbolUrl &baseUrl,
    const std::vector<lyric_common::TypeDef> &typeArguments)
{
    TU_ASSERT (baseUrl.isValid());
    TU_ASSERT (!typeArguments.empty());

    // if type already exists in cache then return it
    auto parameterizedType = lyric_common::TypeDef::forConcrete(baseUrl, typeArguments);
    auto piterator = m_typecache.find(parameterizedType);
    if (piterator != m_typecache.cend()) {
        return piterator->second;
    }

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_objectState->symbolCache()->getOrImportSymbol(baseUrl));
    auto superType = symbol->getTypeDef();

    auto titerator = m_templatecache.find(baseUrl);
    if (titerator == m_templatecache.cend())
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingTemplate,
            "missing template {}", baseUrl.toString());
    auto *superTemplate = titerator->second;
    superTemplate->touch();

    if (std::cmp_less(typeArguments.size(), superTemplate->numPlaceholders())) {
        const auto firstMissing = superTemplate->getPlaceholder(typeArguments.size());
        return AssemblerStatus::forCondition(AssemblerCondition::kIncompatibleType,
            "missing type parameter {}", firstMissing.toString());
    }
    if (std::cmp_greater(typeArguments.size(), superTemplate->numPlaceholders()))
        return AssemblerStatus::forCondition(AssemblerCondition::kIncompatibleType,
            "wrong number of type parameters for {}; expected {} but found {}",
            baseUrl.toString(), superTemplate->numPlaceholders(), typeArguments.size());

    auto siterator = m_typecache.find(superType);
    if (siterator == m_typecache.cend())
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingType,
            "missing type {}", superType.toString());
    auto *superTypeHandle = siterator->second;

    for (const auto &argType : typeArguments) {
        TU_RETURN_IF_STATUS (getOrMakeType(argType));
    }

    auto *typeHandle = new TypeHandle(baseUrl, typeArguments, superTypeHandle);
    m_typecache[parameterizedType] = typeHandle;

    return typeHandle;
}

tempo_utils::Result<lyric_assembler::TypeHandle *>
lyric_assembler::TypeCache::declareFunctionType(
    const lyric_common::TypeDef &functionReturn,
    const std::vector<lyric_common::TypeDef> &functionParameters,
    const Option<lyric_common::TypeDef> &functionRest)
{
    TU_RETURN_IF_STATUS(getOrMakeType(functionReturn));

    // FIXME: if rest param exists then resolve RestFunction instead
    if (!functionRest.isEmpty())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "variadic function not supported");

    auto arity = functionParameters.size();
    auto functionClassUrl = m_objectState->fundamentalCache()->getFunctionUrl(arity);

    // build the type parameters from the function return value and parameters
    std::vector<lyric_common::TypeDef> typeParameters;
    typeParameters.reserve(1 + functionParameters.size() + (functionRest.isEmpty()? 0 : 1));
    typeParameters.push_back(functionReturn);
    typeParameters.insert(typeParameters.end(), functionParameters.cbegin(), functionParameters.cend());
    if (!functionRest.isEmpty()) {
        typeParameters.push_back(functionRest.getValue());
    }

    // create the type if it doesn't already exist
    return declareParameterizedType(functionClassUrl, typeParameters);
}

bool
lyric_assembler::TypeCache::hasTemplate(const lyric_common::SymbolUrl &templateUrl) const
{
    return m_templatecache.contains(templateUrl);
}

lyric_assembler::TemplateHandle *
lyric_assembler::TypeCache::getTemplate(const lyric_common::SymbolUrl &templateUrl) const
{
    auto entry = m_templatecache.find(templateUrl);
    if (entry != m_templatecache.cend())
        return entry->second;
    return nullptr;
}

int
lyric_assembler::TypeCache::numTemplates() const
{
    return m_templatecache.size();
}

tempo_utils::Result<lyric_assembler::TemplateHandle *>
lyric_assembler::TypeCache::getOrImportTemplate(const lyric_common::SymbolUrl &templateUrl)
{
    auto entry = m_templatecache.find(templateUrl);
    if (entry != m_templatecache.cend())
        return entry->second;

    auto *importCache = m_objectState->importCache();
    AbstractSymbol *templateSymbol;
    TU_ASSIGN_OR_RETURN (templateSymbol, importCache->importSymbol(templateUrl));

    TemplateHandle *templateHandle = nullptr;
    switch (templateSymbol->getSymbolType()) {
        case SymbolType::CLASS:
            templateHandle = cast_symbol_to_class(templateSymbol)->classTemplate();
            break;
        case SymbolType::CONCEPT:
            templateHandle = cast_symbol_to_concept(templateSymbol)->conceptTemplate();
            break;
        case SymbolType::EXISTENTIAL:
            templateHandle = cast_symbol_to_existential(templateSymbol)->existentialTemplate();
            break;
        default:
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid template {}", templateUrl.toString());
    }

    m_templatecache[templateUrl] = templateHandle;
    return templateHandle;
}

tempo_utils::Result<lyric_assembler::TemplateHandle *>
lyric_assembler::TypeCache::makeTemplate(
    const lyric_common::SymbolUrl &templateUrl,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    BlockHandle *parentBlock)
{
    if (!templateUrl.isValid())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid template {}", templateUrl.toString());
    if (templateParameters.empty())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "cannot make template {}; missing template parameters", templateUrl.toString());
    if (m_templatecache.contains(templateUrl))
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "template {} is already defined", templateUrl.toString());

    // create the template handle
    auto *templateHandle = new TemplateHandle(templateUrl, templateParameters, parentBlock, m_objectState);
    m_templatecache[templateUrl] = templateHandle;

    // create placeholder type for each template parameter
    for (const auto &tp : templateParameters) {
        auto *typeHandle = new TypeHandle(tp.index, templateUrl, {});
        m_typecache[typeHandle->getTypeDef()] = typeHandle;
    }

    TU_RETURN_IF_NOT_OK (touchTemplateParameters(templateParameters));

    return templateHandle;
}

tempo_utils::Result<lyric_assembler::TemplateHandle *>
lyric_assembler::TypeCache::makeTemplate(
    const lyric_common::SymbolUrl &templateUrl,
    TemplateHandle *superTemplate,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    BlockHandle *parentBlock)
{
    TU_ASSERT (superTemplate != nullptr);

    if (!templateUrl.isValid())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid template {}", templateUrl.toString());
    if (templateParameters.empty())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "cannot make template {}; missing template parameters", templateUrl.toString());
    if (m_templatecache.contains(templateUrl))
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "template {} is already defined", templateUrl.toString());

    // create the template handle
    auto *templateHandle = new TemplateHandle(templateUrl, templateParameters, superTemplate, parentBlock, m_objectState);
    m_templatecache[templateUrl] = templateHandle;

    // create placeholder type for each template parameter
    for (const auto &tp : templateParameters) {
        auto *typeHandle = new TypeHandle(tp.index, templateUrl, {});
        m_typecache[typeHandle->getTypeDef()] = typeHandle;
    }

    TU_RETURN_IF_NOT_OK (touchTemplateParameters(templateParameters));

    return templateHandle;
}

tempo_utils::Status
lyric_assembler::TypeCache::appendTemplate(lyric_assembler::TemplateHandle *templateHandle)
{
    TU_ASSERT (templateHandle != nullptr);
    auto templateUrl = templateHandle->getTemplateUrl();

    if (m_templatecache.contains(templateUrl))
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "template {} is already defined", templateUrl.toString());

    m_templatecache[templateUrl] = templateHandle;
    return {};
}

tempo_utils::Result<lyric_assembler::TemplateHandle *>
lyric_assembler::TypeCache::importTemplate(lyric_importer::TemplateImport *templateImport)
{
    TU_ASSERT (templateImport != nullptr);

    auto templateUrl = templateImport->getTemplateUrl();
    TU_ASSERT (templateUrl.isValid());
    if (m_templatecache.contains(templateUrl))
        return m_templatecache.at(templateUrl);

    auto *superTemplateImport = templateImport->getSuperTemplate();
    TemplateHandle *superTemplate = nullptr;
    if (superTemplateImport != nullptr) {
        TU_ASSIGN_OR_RETURN (superTemplate, importTemplate(superTemplateImport));
    }

    std::vector<lyric_object::TemplateParameter> templateParameters;
    for (auto it = templateImport->templateParametersBegin(); it != templateImport->templateParametersEnd(); it++) {
        lyric_object::TemplateParameter tp;
        tp.index = it->index;
        tp.name = it->name;
        tp.variance = it->variance;
        tp.bound = it->bound;

        if (it->type != nullptr) {
            TypeHandle *paramType;
            TU_ASSIGN_OR_RETURN (paramType, importType(it->type));
            tp.typeDef = paramType->getTypeDef();
            if (!tp.typeDef.isValid())
                return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                    "cannot import template {}; invalid constraint type for template parameter {}",
                    templateUrl.toString(), it->index);
        } else if (tp.bound == lyric_object::BoundType::None) {
            auto *fundamentalCache = m_objectState->fundamentalCache();
            tp.typeDef = fundamentalCache->getFundamentalType(FundamentalSymbol::Any);
        } else {
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "cannot import template {}; missing constraint type for template parameter {}",
                templateUrl.toString(), it->index);
        }

        templateParameters.push_back(tp);
    }

    auto *templateHandle = new TemplateHandle(templateUrl, superTemplate, templateParameters, m_objectState);

    m_templatecache[templateUrl] = templateHandle;
    return templateHandle;
}

tempo_utils::Status
lyric_assembler::TypeCache::touchTemplateParameters(const std::vector<lyric_object::TemplateParameter> &parameters)
{
    for (const auto &tp : parameters) {
        if (tp.typeDef.isValid()) {    // tp.typeDef is not valid if there is no constraint specified
            TU_RETURN_IF_STATUS (getOrMakeType(tp.typeDef));
        }
    }
    return {};
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::TypeCache::resolveConcrete(
    const lyric_common::SymbolUrl &concreteUrl,
    const std::vector<lyric_common::TypeDef> &typeArguments)
{
    auto assignable = lyric_common::TypeDef::forConcrete(concreteUrl, typeArguments);
    // if the concrete type does not exist in the typecache, then create it
    TU_RETURN_IF_STATUS (getOrMakeType(assignable));
    return assignable;
}

inline tempo_utils::Result<lyric_common::SymbolUrl>
get_placeholder_bound(
    const lyric_common::TypeDef &placeholderType,
    lyric_assembler::ObjectState *objectState)
{
    TU_ASSERT (placeholderType.getType() == lyric_common::TypeDefType::Placeholder);
    auto *typeCache = objectState->typeCache();

    auto templateUrl = placeholderType.getPlaceholderTemplateUrl();
    //auto *templateHandle = typeCache->getTemplate(templateUrl);
    lyric_assembler::TemplateHandle *templateHandle;
    TU_ASSIGN_OR_RETURN (templateHandle, typeCache->getOrImportTemplate(templateUrl));

    auto tp = templateHandle->getTemplateParameter(placeholderType.getPlaceholderIndex());
    if (tp.bound != lyric_object::BoundType::None && tp.bound != lyric_object::BoundType::Extends)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kIncompatibleType,
            "incompatible type bound for placeholder {}", placeholderType.toString());

    auto boundType = tp.typeDef;
    if (boundType.getType() != lyric_common::TypeDefType::Concrete)
        return lyric_assembler::AssemblerStatus::forCondition(
            lyric_assembler::AssemblerCondition::kAssemblerInvariant,
            "invalid constraint for placeholder {}", placeholderType.toString());

    return boundType.getConcreteUrl();
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::TypeCache::resolveUnion(const std::vector<lyric_common::TypeDef> &unionMembers)
{
    if (unionMembers.empty())
        return AssemblerStatus::forCondition(AssemblerCondition::kTypeError,
            "invalid union set; set must be non-empty");

    DisjointTypeSet disjointTypeSet(m_objectState);

    // validate union member invariants
    for (const auto &member : unionMembers) {
        lyric_common::SymbolUrl memberBase;

        // we only allow a concrete or placeholder type as a union member
        switch (member.getType()) {
            case lyric_common::TypeDefType::Concrete:
                memberBase = member.getConcreteUrl();
                break;
            case lyric_common::TypeDefType::Placeholder:
                TU_ASSIGN_OR_RETURN (memberBase, get_placeholder_bound(member, m_objectState));
                break;
            default:
                return AssemblerStatus::forCondition(AssemblerCondition::kTypeError,
                    "invalid union member {}; type must be concrete or placeholder", member.toString());
        }

        // all union members must be disjoint
        TU_RETURN_IF_NOT_OK (disjointTypeSet.putType(lyric_common::TypeDef::forConcrete(memberBase)));

        // member symbol must exist in the cache
        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, m_objectState->symbolCache()->getOrImportSymbol(memberBase));

        // member symbol is restricted to certain symbol types
        switch (symbol->getSymbolType()) {
            case SymbolType::CLASS:
            case SymbolType::ENUM:
            case SymbolType::EXISTENTIAL:
            case SymbolType::INSTANCE:
            case SymbolType::STRUCT:
            case SymbolType::TYPENAME:
                break;
            default:
                return AssemblerStatus::forCondition(AssemblerCondition::kTypeError,
                    "invalid symbol {} for union member", memberBase.toString());
        }
    }

    auto assignable = lyric_common::TypeDef::forUnion(unionMembers);
    // if the union type does not exist in the typecache, then create it
    TU_RETURN_IF_STATUS (getOrMakeType(assignable));
    return assignable;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::TypeCache::resolveIntersection(const std::vector<lyric_common::TypeDef> &intersectionMembers)
{
    if (intersectionMembers.empty())
        return AssemblerStatus::forCondition(AssemblerCondition::kTypeError,
            "invalid intersection set; set must be non-empty");

    absl::flat_hash_set<lyric_common::SymbolUrl> memberSet;

    // validate intersection member invariants
    for (const auto &member : intersectionMembers) {

        // we don't allow placeholder, union, or intersection type as a union member
        if (member.getType() != lyric_common::TypeDefType::Concrete)
            return AssemblerStatus::forCondition(AssemblerCondition::kTypeError,
                "invalid intersection member {}; type must be concrete", member.toString());
        auto memberBase = member.getConcreteUrl();

        // concrete symbol cannot appear more than once (for example parameterized over different types)
        if (memberSet.contains(memberBase))
            return AssemblerStatus::forCondition(AssemblerCondition::kTypeError,
                "duplicate intersection member {}", member.toString());
        memberSet.insert(memberBase);

        // concrete symbol must exist in the cache
        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, m_objectState->symbolCache()->getOrImportSymbol(memberBase));

        // intersection member symbol must be a concept which is either sealed or final
        DeriveType derive = DeriveType::Any;
        switch (symbol->getSymbolType()) {
            case SymbolType::CONCEPT: {
                //auto *conceptSymbol = cast_symbol_to_concept(baseSym);
                //FIXME: derive = conceptSymbol->getDeriveType();
                derive = DeriveType::Final;
                break;
            }
            default:
                return AssemblerStatus::forCondition(AssemblerCondition::kTypeError,
                    "invalid symbol {} for intersection member", memberBase.toString());
        }

        switch (derive) {
            case DeriveType::Final:
            case DeriveType::Sealed:
                break;
            default:
                return AssemblerStatus::forCondition(AssemblerCondition::kTypeError,
                    "invalid intersection member {}; must be sealed or final", member.toString());
        }
    }

    auto assignable = lyric_common::TypeDef::forIntersection(intersectionMembers);
    // if the intersection type does not exist in the typecache, then create it
    TU_RETURN_IF_STATUS (getOrMakeType(assignable));
    return assignable;
}

tempo_utils::Result<lyric_assembler::TypeSignature>
lyric_assembler::TypeCache::resolveSignature(const lyric_common::TypeDef &typeDef)
{
    auto siterator = m_signaturecache.find(typeDef);
    if (siterator != m_signaturecache.cend())
        return siterator->second;
    auto hiterator = m_typecache.find(typeDef);
    if (hiterator == m_typecache.cend())
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant, "missing type {}", typeDef.toString());
    const TypeHandle *typeHandle = hiterator->second;

    std::vector<const TypeHandle *> typepath;
    for (; typeHandle != nullptr; typeHandle = typeHandle->getSuperType()) {
        typepath.insert(typepath.cbegin(), typeHandle);
    }

    TypeSignature signature(typepath);
    m_signaturecache[typeDef] = signature;
    return signature;
}

tempo_utils::Result<lyric_assembler::TypeSignature>
lyric_assembler::TypeCache::resolveSignature(const lyric_common::SymbolUrl &symbolUrl)
{
    TU_ASSERT (symbolUrl.isValid());

    auto typeDef = lyric_common::TypeDef::forConcrete(symbolUrl);
    auto iterator = m_signaturecache.find(typeDef);
    if (iterator != m_signaturecache.cend())
        return iterator->second;

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_objectState->symbolCache()->getOrImportSymbol(symbolUrl));

    TypeHandle *typeHandle;
    switch (symbol->getSymbolType()) {
        case SymbolType::ARGUMENT:
            return resolveSignature(cast_symbol_to_argument(symbol)->getTypeDef());
        case SymbolType::LEXICAL:
            return resolveSignature(cast_symbol_to_lexical(symbol)->getTypeDef());
        case SymbolType::LOCAL:
            return resolveSignature(cast_symbol_to_local(symbol)->getTypeDef());
        case SymbolType::CLASS:
            typeHandle = cast_symbol_to_class(symbol)->classType();
            break;
        case SymbolType::CONCEPT:
            typeHandle = cast_symbol_to_concept(symbol)->conceptType();
            break;
        case SymbolType::ENUM:
            typeHandle = cast_symbol_to_enum(symbol)->enumType();
            break;
        case SymbolType::EXISTENTIAL:
            typeHandle = cast_symbol_to_existential(symbol)->existentialType();
            break;
        case SymbolType::INSTANCE:
            typeHandle = cast_symbol_to_instance(symbol)->instanceType();
            break;
        case SymbolType::STRUCT:
            typeHandle = cast_symbol_to_struct(symbol)->structType();
            break;
        default:
            return AssemblerStatus::forCondition(AssemblerCondition::kTypeError,
                "cannot resolve type signature for symbol {}", symbolUrl.toString());
    }

    std::vector<const TypeHandle *> typepath;
    for (; typeHandle != nullptr; typeHandle = typeHandle->getSuperType()) {
        typepath.insert(typepath.cbegin(), typeHandle);
    }

    TypeSignature signature(typepath);
    m_signaturecache[typeDef] = signature;
    return signature;
}

tempo_utils::Result<lyric_assembler::TypeSignature>
lyric_assembler::TypeCache::resolveSignature(const TypeHandle *typeHandle)
{
    TU_ASSERT (typeHandle != nullptr);

    auto typeDef = typeHandle->getTypeDef();
    auto iterator = m_signaturecache.find(typeDef);
    if (iterator != m_signaturecache.cend())
        return iterator->second;

    std::vector<const TypeHandle *> typepath;
    for (; typeHandle != nullptr; typeHandle = typeHandle->getSuperType()) {
        typepath.insert(typepath.cbegin(), typeHandle);
    }

    TypeSignature signature(typepath);
    m_signaturecache[typeDef] = signature;
    return signature;
}
