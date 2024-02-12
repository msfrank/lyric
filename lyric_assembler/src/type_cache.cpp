
#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/disjoint_type_set.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_assembler/type_handle.h>

lyric_assembler::TypeCache::TypeCache(AssemblyState *assemblyState, lyric_assembler::AssemblerTracer *tracer)
    : m_assemblyState(assemblyState),
      m_tracer(tracer)
{
    TU_ASSERT (m_assemblyState != nullptr);
    TU_ASSERT (m_tracer != nullptr);
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
lyric_assembler::TypeCache::hasType(const lyric_common::TypeDef &assignableType) const
{
    return m_typecache.contains(assignableType);
}

lyric_assembler::TypeHandle *
lyric_assembler::TypeCache::getType(const lyric_common::TypeDef &assignableType) const
{
    if (m_typecache.contains(assignableType))
        return m_typecache.at(assignableType);
    return nullptr;
}

lyric_assembler::TypeHandle *
lyric_assembler::TypeCache::getType(TypeAddress typeAddress) const
{
    uint32_t address = typeAddress.getAddress();

    if (lyric_object::IS_NEAR(address)) {
        if (m_types.size() <= address)
            return nullptr;
        return m_types.at(address);
    }
    return nullptr;
}

std::vector<lyric_assembler::TypeHandle *>::const_iterator
lyric_assembler::TypeCache::typesBegin() const
{
    return m_types.cbegin();
}

std::vector<lyric_assembler::TypeHandle *>::const_iterator
lyric_assembler::TypeCache::typesEnd() const
{
    return m_types.cend();
}

int
lyric_assembler::TypeCache::numTypes() const
{
    return m_types.size();
}

tempo_utils::Status
lyric_assembler::TypeCache::makeType(const lyric_common::TypeDef &assignableType)
{
    if (!assignableType.isValid())
        m_tracer->throwAssemblerInvariant("failed to make type {}; invalid type", assignableType.toString());
    if (m_typecache.contains(assignableType))
        return AssemblerStatus::ok();

    switch (assignableType.getType()) {

        case lyric_common::TypeDefType::Concrete: {
            for (auto it = assignableType.concreteArgumentsBegin(); it != assignableType.concreteArgumentsEnd(); it++) {
                if (!m_typecache.contains(*it))
                    return m_tracer->logAndContinue(AssemblerCondition::kMissingType,
                        tempo_tracing::LogSeverity::kError, "missing type {}", it->toString());
            }
            auto concreteUrl = assignableType.getConcreteUrl();
            if (!m_assemblyState->symbolCache()->hasSymbol(concreteUrl))
                return m_tracer->logAndContinue(AssemblerCondition::kMissingSymbol,
                    tempo_tracing::LogSeverity::kError, "missing symbol {}", concreteUrl.toString());
            auto superAssignable = m_assemblyState->symbolCache()->getSymbol(concreteUrl)->getAssignableType();
            if (!m_typecache.contains(superAssignable))
                return m_tracer->logAndContinue(AssemblerCondition::kMissingType,
                    tempo_tracing::LogSeverity::kError, "missing type {}", superAssignable.toString());
            auto *superType = m_typecache[superAssignable];
            auto *typeHandle = new TypeHandle(assignableType, superType, m_assemblyState);
            m_typecache[assignableType] = typeHandle;
            return AssemblerStatus::ok();
        }

        case lyric_common::TypeDefType::Placeholder: {
            for (auto it = assignableType.placeholderArgumentsBegin(); it != assignableType.placeholderArgumentsEnd(); it++) {
                if (!m_typecache.contains(*it))
                    return m_tracer->logAndContinue(AssemblerCondition::kMissingType,
                        tempo_tracing::LogSeverity::kError, "missing type {}", it->toString());
            }
            auto templateUrl = assignableType.getPlaceholderTemplateUrl();
            if (!m_templatecache.contains(templateUrl))
                return m_tracer->logAndContinue(AssemblerCondition::kMissingTemplate,
                    tempo_tracing::LogSeverity::kError, "missing template {}", templateUrl.toString());
            auto *typeHandle = new TypeHandle(assignableType, nullptr, m_assemblyState);
            m_typecache[assignableType] = typeHandle;
            return AssemblerStatus::ok();
        }

        case lyric_common::TypeDefType::Union: {
            for (auto it = assignableType.unionMembersBegin(); it != assignableType.unionMembersEnd(); it++) {
                if (!m_typecache.contains(*it))
                    return m_tracer->logAndContinue(AssemblerCondition::kMissingType,
                        tempo_tracing::LogSeverity::kError, "missing type {}", it->toString());
            }
            auto *typeHandle = new TypeHandle(assignableType, nullptr, m_assemblyState);
            m_typecache[assignableType] = typeHandle;
            return AssemblerStatus::ok();
        }

        case lyric_common::TypeDefType::Intersection: {
            for (auto it = assignableType.intersectionMembersBegin(); it != assignableType.intersectionMembersEnd(); it++) {
                if (!m_typecache.contains(*it))
                    return m_tracer->logAndContinue(AssemblerCondition::kMissingType,
                        tempo_tracing::LogSeverity::kError, "missing type {}", it->toString());
            }
            auto *typeHandle = new TypeHandle(assignableType, nullptr, m_assemblyState);
            m_typecache[assignableType] = typeHandle;
            return AssemblerStatus::ok();
        }

        case lyric_common::TypeDefType::NoReturn: {
            auto *typeHandle = new TypeHandle(assignableType, nullptr, m_assemblyState);
            m_typecache[assignableType] = typeHandle;
            return AssemblerStatus::ok();
        }

        default:
            m_tracer->throwAssemblerInvariant("failed to make type {}; invalid type", assignableType.toString());
    }
}

tempo_utils::Result<lyric_assembler::TypeHandle *>
lyric_assembler::TypeCache::importType(lyric_importer::TypeImport *typeImport)
{
    TU_ASSERT (typeImport != nullptr);

    auto assignableType = typeImport->getTypeDef();
    if (m_typecache.contains(assignableType))
        return m_typecache.at(assignableType);

    for (auto iterator = typeImport->argumentsBegin(); iterator != typeImport->argumentsEnd(); iterator++) {
        TU_RETURN_IF_STATUS (importType(*iterator));
    }

    TypeHandle *superTypeHandle = nullptr;
    if (typeImport->getSuperType() != nullptr) {
        auto assignableSuperType = typeImport->getSuperType()->getTypeDef();
        TU_ASSIGN_OR_RETURN(superTypeHandle, importType(typeImport->getSuperType()));
    }

    auto *typeHandle = new TypeHandle(assignableType, superTypeHandle, m_assemblyState);
    m_typecache[assignableType] = typeHandle;
    return typeHandle;
}

tempo_utils::Status
lyric_assembler::TypeCache::touchType(TypeHandle *typeHandle)
{
    TU_ASSERT (typeHandle != nullptr);

    // type has been touched already, nothing to do
    if (typeHandle->getAddress().isValid())
        return AssemblerStatus::ok();

    // verify that the type handle is in the type cache
    auto type = typeHandle->getTypeDef();
    if (!m_typecache.contains(type))
        m_tracer->throwAssemblerInvariant("handle for type {} exists but is not in cache", type.toString());

    // touch all super types first
    for (auto *superType = typeHandle->getSuperType(); superType != nullptr; superType = superType->getSuperType()) {
        touchType(superType);
    }

    // next touch type arguments, if there are any
    switch (type.getType()) {
        case lyric_common::TypeDefType::Concrete:
        case lyric_common::TypeDefType::Placeholder:
        case lyric_common::TypeDefType::Union:
        case lyric_common::TypeDefType::Intersection:
            for (auto it = typeHandle->typeArgumentsBegin(); it != typeHandle->typeArgumentsEnd(); it++) {
                auto status = touchType(*it);
                if (!status.isOk())
                    return status;
            }
            break;
        // the below types never have type arguments
        case lyric_common::TypeDefType::NoReturn:
            break;
        default:
            m_tracer->throwAssemblerInvariant("invalid type {}", type.toString());
    }

    // check again if type has been touched already via a placeholder type parameter
    if (typeHandle->getAddress().isValid())
        return AssemblerStatus::ok();

    // add type handle to types array and update the type handle address
    auto address = TypeAddress(m_types.size());
    auto status = typeHandle->updateAddress(address);
    if (status.notOk())
        return status;
    m_types.push_back(typeHandle);

    // if the type has an associated type symbol, then touch the type symbol as well
    auto typeSymbol = typeHandle->getTypeSymbol();
    if (typeSymbol.isValid()) {
        status = m_assemblyState->symbolCache()->touchSymbol(typeSymbol);
        if (status.notOk())
            return status;
    }

    return AssemblerStatus::ok();
}

tempo_utils::Status
lyric_assembler::TypeCache::touchType(const lyric_common::TypeDef &typeDef)
{
    TU_ASSERT (typeDef.isValid());
    return touchType(m_typecache[typeDef]);
}

tempo_utils::Result<lyric_assembler::TypeHandle *>
lyric_assembler::TypeCache::declareSubType(
    const lyric_common::SymbolUrl &subUrl,
    const std::vector<lyric_common::TypeDef> &subTypeArguments,
    const lyric_common::TypeDef &superType)
{
    TU_ASSERT (subUrl.isValid());
    TU_ASSERT (superType.isValid());

    if (m_assemblyState->symbolCache()->hasSymbol(subUrl))
        return m_tracer->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError, "symbol {} is already defined", subUrl.toString());

    if (!m_typecache.contains(superType))
        return m_tracer->logAndContinue(AssemblerCondition::kMissingType,
            tempo_tracing::LogSeverity::kError, "missing type {}", superType.toString());
    auto *superTypeHandle = m_typecache[superType];

    auto type_index = m_types.size();
    auto typeAddress = TypeAddress::near(type_index);
    auto *typeHandle = new TypeHandle(subUrl, subTypeArguments, typeAddress, superTypeHandle, m_assemblyState);
    auto subType = typeHandle->getTypeDef();
    if (m_typecache.contains(subType)) {
        delete typeHandle;
        touchType(subType);
    } else {
        m_types.push_back(typeHandle);
        m_typecache[typeHandle->getTypeDef()] = typeHandle;
    }

    return typeHandle;
}

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
        TU_RETURN_IF_NOT_OK (touchType(parameterizedType));
        return piterator->second;
    }

    auto *sym = m_assemblyState->symbolCache()->getSymbol(baseUrl);
    if (sym == nullptr)
        return m_tracer->logAndContinue(AssemblerCondition::kMissingType,
            tempo_tracing::LogSeverity::kError, "missing type {}",
            lyric_common::TypeDef::forConcrete(baseUrl).toString());
    auto superType = sym->getAssignableType();

    auto titerator = m_templatecache.find(baseUrl);
    if (titerator == m_templatecache.cend())
        return m_tracer->logAndContinue(AssemblerCondition::kMissingTemplate,
            tempo_tracing::LogSeverity::kError, "missing template {}", baseUrl.toString());
    auto *superTemplate = titerator->second;
    superTemplate->touch();

    if (std::cmp_less(typeArguments.size(), superTemplate->numPlaceholders())) {
        const auto firstMissing = superTemplate->getPlaceholder(typeArguments.size());
        return m_tracer->logAndContinue(AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError, "missing type parameter {}", firstMissing.toString());
    }
    if (std::cmp_greater(typeArguments.size(), superTemplate->numPlaceholders()))
        return m_tracer->logAndContinue(AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "wrong number of type parameters for {}; expected {} but found {}",
            baseUrl.toString(), superTemplate->numPlaceholders(), typeArguments.size());

    auto siterator = m_typecache.find(superType);
    if (siterator == m_typecache.cend())
        return m_tracer->logAndContinue(AssemblerCondition::kMissingType,
            tempo_tracing::LogSeverity::kError, "missing type {}", superType.toString());
    auto *superTypeHandle = siterator->second;
    superTypeHandle->touch();

    auto type_index = m_types.size();
    auto typeAddress = TypeAddress::near(type_index);
    auto *typeHandle = new TypeHandle(baseUrl, typeArguments, typeAddress, superTypeHandle, m_assemblyState);
    m_types.push_back(typeHandle);
    m_typecache[parameterizedType] = typeHandle;

    return typeHandle;
}

tempo_utils::Result<lyric_assembler::TypeHandle *>
lyric_assembler::TypeCache::declareFunctionType(
    const lyric_common::TypeDef &functionReturn,
    const std::vector<lyric_object::Parameter> &functionParameters,
    const Option<lyric_object::Parameter> &functionRest)
{
    TU_RETURN_IF_NOT_OK (touchType(functionReturn));

    // FIXME: if rest param exists then resolve RestFunction instead
    if (!functionRest.isEmpty())
        m_assemblyState->throwAssemblerInvariant("variadic function not supported");

    auto arity = functionParameters.size();
    auto functionClassUrl = m_assemblyState->fundamentalCache()->getFunctionUrl(arity);

    // build the type parameters from the function return value and parameters
    std::vector<lyric_common::TypeDef> typeParameters;
    typeParameters.reserve(1 + functionParameters.size() + (functionRest.isEmpty()? 0 : 1));
    typeParameters.push_back(functionReturn);
    for (const auto &p : functionParameters) {
        typeParameters.push_back(p.typeDef);
    }
    if (!functionRest.isEmpty()) {
        typeParameters.push_back(functionRest.getValue().typeDef);
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
    if (m_templatecache.contains(templateUrl))
        return m_templatecache.at(templateUrl);
    return nullptr;
}

std::vector<lyric_assembler::TemplateHandle *>::const_iterator
lyric_assembler::TypeCache::templatesBegin() const
{
    return m_templates.cbegin();
}

std::vector<lyric_assembler::TemplateHandle *>::const_iterator
lyric_assembler::TypeCache::templatesEnd() const
{
    return m_templates.cend();
}

int
lyric_assembler::TypeCache::numTemplates() const
{
    return m_templates.size();
}

tempo_utils::Status
lyric_assembler::TypeCache::makeTemplate(
    const lyric_common::SymbolUrl &templateUrl,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    BlockHandle *parentBlock)
{
    if (!templateUrl.isValid())
        m_tracer->throwAssemblerInvariant("invalid template {}", templateUrl.toString());
    if (templateParameters.empty())
        m_tracer->throwAssemblerInvariant("cannot make template {}; missing template parameters", templateUrl.toString());
    if (m_templatecache.contains(templateUrl))
        m_tracer->throwAssemblerInvariant("template {} is already defined", templateUrl.toString());

    // create the template handle
    auto template_index = m_templates.size();
    auto templateAddress = TemplateAddress::near(template_index);
    auto *templateHandle = new TemplateHandle(templateUrl, templateParameters, templateAddress,
        parentBlock, m_assemblyState);
    m_templates.push_back(templateHandle);
    m_templatecache[templateUrl] = templateHandle;

    // create placeholder type for each template parameter
    for (const auto &tp : templateParameters) {
        auto type_index = m_types.size();
        auto typeAddress = TypeAddress::near(type_index);
        auto *typeHandle = new TypeHandle(tp.index, templateUrl, {}, typeAddress, m_assemblyState);
        m_types.push_back(typeHandle);
        m_typecache[typeHandle->getTypeDef()] = typeHandle;
    }

    auto status = touchTemplateParameters(templateParameters);
    if (!status.isOk())
        return status;

    return AssemblerStatus::ok();
}

tempo_utils::Result<lyric_assembler::TemplateHandle *>
lyric_assembler::TypeCache::importTemplate(lyric_importer::TemplateImport *templateImport)
{
    TU_ASSERT (templateImport != nullptr);

    auto templateUrl = templateImport->getTemplateUrl();
    TU_ASSERT (templateUrl.isValid());
    if (m_templatecache.contains(templateUrl))
        return m_templatecache.at(templateUrl);

    std::vector<lyric_object::TemplateParameter> templateParameters;
    for (auto it = templateImport->templateParametersBegin(); it != templateImport->templateParametersEnd(); it++) {
        lyric_object::TemplateParameter tp;
        tp.index = it->index;
        tp.name = it->name;
        tp.variance = it->variance;
        tp.bound = it->bound;

        if (it->type != nullptr) {
            TypeHandle *paramType = nullptr;
            TU_ASSIGN_OR_RAISE (paramType, importType(it->type));
            tp.typeDef = paramType->getTypeDef();
        } else {
            tp.typeDef = {};
        }

        templateParameters.push_back(tp);
    }

    TU_ASSERT (!templateParameters.empty());

    auto *templateHandle = new TemplateHandle(templateUrl, templateParameters, m_assemblyState);
    m_templatecache[templateUrl] = templateHandle;
    return templateHandle;
}

tempo_utils::Status
lyric_assembler::TypeCache::touchTemplateParameters(const std::vector<lyric_object::TemplateParameter> &parameters)
{
    for (const auto &tp : parameters) {
        if (tp.typeDef.isValid()) {    // tp.typeDef is not valid if there is no constraint specified
            auto status = touchType(tp.typeDef);
            if (!status.isOk())
                return status;
        }
    }
    return AssemblerStatus::ok();
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::TypeCache::resolveConcrete(
    const lyric_common::SymbolUrl &concreteUrl,
    const std::vector<lyric_common::TypeDef> typeArguments)
{
    auto assignable = lyric_common::TypeDef::forConcrete(concreteUrl, typeArguments);

    // if the concrete type does not exist in the typecache, then create it
    if (!m_typecache.contains(assignable)) {
        auto status = makeType(assignable);
        if (status.notOk())
            return status;
    }

    return assignable;
}

inline tempo_utils::Result<lyric_common::SymbolUrl>
get_placeholder_bound(
    const lyric_common::TypeDef &placeholderType,
    lyric_assembler::AssemblyState *assemblyState,
    lyric_assembler::AssemblerTracer *tracer)
{
    TU_ASSERT (placeholderType.getType() == lyric_common::TypeDefType::Placeholder);
    auto *typeCache = assemblyState->typeCache();

    auto templateUrl = placeholderType.getPlaceholderTemplateUrl();
    auto *templateHandle = typeCache->getTemplate(templateUrl);
    if (templateHandle == nullptr)
        tracer->throwAssemblerInvariant("missing template for placeholder {}", placeholderType.toString());

    auto tp = templateHandle->getTemplateParameter(placeholderType.getPlaceholderIndex());
    if (tp.bound != lyric_object::BoundType::Extends)
        return tracer->logAndContinue(
            lyric_assembler::AssemblerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "incompatible type bound for placeholder {}", placeholderType.toString());

    auto boundType = tp.typeDef;
    if (boundType.getType() != lyric_common::TypeDefType::Concrete)
        tracer->throwAssemblerInvariant("invalid constraint for placeholder {}", placeholderType.toString());

    return boundType.getConcreteUrl();
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::TypeCache::resolveUnion(const std::vector<lyric_common::TypeDef> &unionMembers)
{
    DisjointTypeSet disjointTypeSet(m_assemblyState);

    // validate union member invariants
    for (const auto &member : unionMembers) {
        lyric_common::SymbolUrl memberBase;

        // we only allow a concrete or placeholder type as a union member
        switch (member.getType()) {
            case lyric_common::TypeDefType::Concrete:
                memberBase = member.getConcreteUrl();
                break;
            case lyric_common::TypeDefType::Placeholder:
                TU_ASSIGN_OR_RETURN (memberBase, get_placeholder_bound(member, m_assemblyState, m_tracer));
                break;
            default:
                return m_tracer->logAndContinue(AssemblerCondition::kTypeError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid union member {}; type must be concrete or placeholder", member.toString());
        }

        // all union members must be disjoint
        TU_RETURN_IF_NOT_OK (disjointTypeSet.putType(lyric_common::TypeDef::forConcrete(memberBase)));

        // member symbol must exist in the cache
        auto *baseSym = m_assemblyState->symbolCache()->getSymbol(memberBase);
        if (baseSym == nullptr)
            return m_tracer->logAndContinue(AssemblerCondition::kMissingSymbol,
                tempo_tracing::LogSeverity::kError, "missing symbol {}", memberBase.toString());

        // member symbol is restricted to certain symbol types
        switch (baseSym->getSymbolType()) {
            case SymbolType::CLASS:
            case SymbolType::ENUM:
            case SymbolType::EXISTENTIAL:
            case SymbolType::INSTANCE:
            case SymbolType::STRUCT:
                break;
            default:
                return m_tracer->logAndContinue(AssemblerCondition::kTypeError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid symbol {} for union member", memberBase.toString());
        }
    }

    auto assignable = lyric_common::TypeDef::forUnion(unionMembers);

    // if the union type does not exist in the typecache, then create it
    if (!m_typecache.contains(assignable)) {
        auto status = makeType(assignable);
        if (status.notOk())
            return status;
    }

    return assignable;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::TypeCache::resolveIntersection(const std::vector<lyric_common::TypeDef> &intersectionMembers)
{
    absl::flat_hash_set<lyric_common::SymbolUrl> memberSet;

    // validate intersection member invariants
    for (const auto &member : intersectionMembers) {

        // we don't allow placeholder, union, or intersection type as a union member
        if (member.getType() != lyric_common::TypeDefType::Concrete)
            return m_tracer->logAndContinue(AssemblerCondition::kTypeError,
                tempo_tracing::LogSeverity::kError,
                "invalid intersection member {}; type must be concrete", member.toString());
        auto memberBase = member.getConcreteUrl();
        // concrete symbol cannot appear more than once (for example parameterized over different types)
        if (memberSet.contains(memberBase))
            return m_tracer->logAndContinue(AssemblerCondition::kTypeError,
                tempo_tracing::LogSeverity::kError,
                "duplicate intersection member {}", member.toString());
        memberSet.insert(memberBase);
        // concrete symbol must exist in the cache
        if (!m_assemblyState->symbolCache()->hasSymbol(memberBase))
            return m_tracer->logAndContinue(AssemblerCondition::kMissingSymbol,
                tempo_tracing::LogSeverity::kError, "missing symbol {}", memberBase.toString());
        auto *baseSym = m_assemblyState->symbolCache()->getSymbol(memberBase);

        // intersection member symbol must be a concept which is either sealed or final
        DeriveType derive = DeriveType::ANY;
        switch (baseSym->getSymbolType()) {
            case SymbolType::CONCEPT: {
                //auto *conceptSymbol = cast_symbol_to_concept(baseSym);
                //FIXME: derive = conceptSymbol->getDeriveType();
                derive = DeriveType::FINAL;
                break;
            }
            default:
                return m_tracer->logAndContinue(AssemblerCondition::kTypeError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid symbol {} for intersection member", memberBase.toString());
        }

        switch (derive) {
            case DeriveType::FINAL:
            case DeriveType::SEALED:
                break;
            default:
                return m_tracer->logAndContinue(AssemblerCondition::kTypeError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid intersection member {}; must be sealed or final", member.toString());
        }
    }

    auto assignable = lyric_common::TypeDef::forIntersection(intersectionMembers);

    // if the intersection type does not exist in the typecache, then create it
    if (!m_typecache.contains(assignable)) {
        auto status = makeType(assignable);
        if (status.notOk())
            return status;
    }

    return assignable;
}

tempo_utils::Result<lyric_assembler::TypeSignature>
lyric_assembler::TypeCache::resolveSignature(const lyric_common::SymbolUrl &symbolUrl) const
{
    TU_ASSERT (symbolUrl.isValid());

    if (!m_assemblyState->symbolCache()->hasSymbol(symbolUrl))
        return m_tracer->logAndContinue(AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError, "missing symbol {}", symbolUrl.toString());
    auto *sym = m_assemblyState->symbolCache()->getSymbol(symbolUrl);
    sym->touch();
    switch (sym->getSymbolType()) {
        case SymbolType::EXISTENTIAL:
        case SymbolType::CLASS:
        case SymbolType::INSTANCE:
        case SymbolType::ENUM:
        case SymbolType::CONCEPT:
        case SymbolType::STRUCT:
            return sym->getTypeSignature();
        default:
            return m_tracer->logAndContinue(AssemblerCondition::kTypeError,
                tempo_tracing::LogSeverity::kError,
                "cannot resolve type signature for symbol {}", symbolUrl.toString());
    }
}
