
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/code_builder.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/enum_import.h>

lyric_assembler::EnumSymbol::EnumSymbol(
    const lyric_common::SymbolUrl &enumUrl,
    lyric_object::AccessType access,
    lyric_object::DeriveType derive,
    bool isAbstract,
    EnumAddress address,
    TypeHandle *enumType,
    EnumSymbol *superEnum,
    BlockHandle *parentBlock,
    AssemblyState *state)
    : BaseSymbol(address, new EnumSymbolPriv()),
      m_enumUrl(enumUrl),
      m_state(state)
{
    TU_ASSERT (m_enumUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->access = access;
    priv->derive = derive;
    priv->isAbstract = isAbstract;
    priv->enumType = enumType;
    priv->superEnum = superEnum;
    priv->allocatorTrap = lyric_object::INVALID_ADDRESS_U32;
    priv->enumBlock = std::make_unique<BlockHandle>(enumUrl, parentBlock, false);

    TU_ASSERT (priv->enumType != nullptr);
    TU_ASSERT (priv->superEnum != nullptr);
}

lyric_assembler::EnumSymbol::EnumSymbol(
    const lyric_common::SymbolUrl &enumUrl,
    lyric_importer::EnumImport *enumImport,
    AssemblyState *state)
    : m_enumUrl(enumUrl),
      m_enumImport(enumImport),
      m_state(state)
{
    TU_ASSERT (m_enumUrl.isValid());
    TU_ASSERT (m_enumImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::EnumSymbolPriv *
lyric_assembler::EnumSymbol::load()
{
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<EnumSymbolPriv>();
    priv->access = lyric_object::AccessType::Public;
    priv->derive = m_enumImport->getDerive();
    priv->isAbstract = m_enumImport->isAbstract();

    auto *enumType = m_enumImport->getEnumType();
    TU_ASSIGN_OR_RAISE (priv->enumType, typeCache->importType(enumType));

    auto superEnumUrl = m_enumImport->getSuperEnum();
    if (superEnumUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superEnum, importCache->importEnum(superEnumUrl));
    }

    for (auto iterator = m_enumImport->membersBegin(); iterator != m_enumImport->membersEnd(); iterator++) {
        FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RAISE (fieldSymbol, importCache->importField(iterator->second));

        DataReference memberRef;
        memberRef.symbolUrl = iterator->second;
        memberRef.typeDef = fieldSymbol->getAssignableType();
        memberRef.referenceType = fieldSymbol->isVariable()? ReferenceType::Variable : ReferenceType::Value;
        priv->members[iterator->first] = memberRef;
    }

    for (auto iterator = m_enumImport->methodsBegin(); iterator != m_enumImport->methodsEnd(); iterator++) {
        CallSymbol *callSymbol;
        TU_ASSIGN_OR_RAISE (callSymbol, importCache->importCall(iterator->second));

        BoundMethod methodBinding;
        methodBinding.methodCall = iterator->second;
        methodBinding.access = callSymbol->getAccessType();
        methodBinding.final = false;    // FIXME: this should come from the call symbol
        priv->methods[iterator->first] = methodBinding;
    }

    auto *implCache = m_state->implCache();
    for (auto iterator = m_enumImport->implsBegin(); iterator != m_enumImport->implsEnd(); iterator++) {
        ImplHandle *implHandle;
        TU_ASSIGN_OR_RAISE (implHandle, implCache->importImpl(iterator->second));
        auto implUrl = iterator->first.getConcreteUrl();
        priv->impls[implUrl] = implHandle;
    }

    for (auto iterator = m_enumImport->sealedTypesBegin(); iterator != m_enumImport->sealedTypesEnd(); iterator++) {
        priv->sealedTypes.insert(*iterator);
    }

    priv->allocatorTrap = m_enumImport->getAllocator();

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::EnumSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Enum;
}

lyric_assembler::SymbolType
lyric_assembler::EnumSymbol::getSymbolType() const
{
    return SymbolType::ENUM;
}

lyric_common::SymbolUrl
lyric_assembler::EnumSymbol::getSymbolUrl() const
{
    return m_enumUrl;
}

lyric_common::TypeDef
lyric_assembler::EnumSymbol::getAssignableType() const
{
    auto *priv = getPriv();
    return priv->enumType->getTypeDef();
}

lyric_assembler::TypeSignature
lyric_assembler::EnumSymbol::getTypeSignature() const
{
    auto *priv = getPriv();
    return priv->enumType->getTypeSignature();
}

void
lyric_assembler::EnumSymbol::touch()
{
    if (getAddress().isValid())
        return;
    m_state->touchEnum(this);
}

lyric_object::AccessType
lyric_assembler::EnumSymbol::getAccessType() const
{
    auto *priv = getPriv();
    return priv->access;
}

lyric_object::DeriveType
lyric_assembler::EnumSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
}

bool
lyric_assembler::EnumSymbol::isAbstract() const
{
    auto *priv = getPriv();
    return priv->isAbstract;
}

lyric_assembler::TypeHandle *
lyric_assembler::EnumSymbol::enumType() const
{
    auto *priv = getPriv();
    return priv->enumType;
}

lyric_assembler::EnumSymbol *
lyric_assembler::EnumSymbol::superEnum() const
{
    auto *priv = getPriv();
    return priv->superEnum;
}

lyric_assembler::BlockHandle *
lyric_assembler::EnumSymbol::enumBlock() const
{
    auto *priv = getPriv();
    return priv->enumBlock.get();
}

bool
lyric_assembler::EnumSymbol::hasMember(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->members.contains(name);
}

Option<lyric_assembler::DataReference>
lyric_assembler::EnumSymbol::getMember(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->members.contains(name))
        return Option<DataReference>(priv->members.at(name));
    return {};
}

absl::flat_hash_map<std::string,lyric_assembler::DataReference>::const_iterator
lyric_assembler::EnumSymbol::membersBegin() const
{
    auto *priv = getPriv();
    return priv->members.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::DataReference>::const_iterator
lyric_assembler::EnumSymbol::membersEnd() const
{
    auto *priv = getPriv();
    return priv->members.cend();
}

tu_uint32
lyric_assembler::EnumSymbol::numMembers() const
{
    auto *priv = getPriv();
    return priv->members.size();
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::EnumSymbol::declareMember(
    const std::string &name,
    const lyric_parser::Assignable &memberSpec,
    bool isVariable,
    lyric_object::AccessType access,
    const lyric_common::SymbolUrl &init)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare member on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();

    if (priv->members.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "member {} already defined for enum {}", name, m_enumUrl.toString());

    auto resolveMemberTypeResult = priv->enumBlock->resolveAssignable(memberSpec);
    if (resolveMemberTypeResult.isStatus())
        return resolveMemberTypeResult.getStatus();
    auto memberType = resolveMemberTypeResult.getResult();
    if (!m_state->typeCache()->hasType(memberType))
        m_state->throwAssemblerInvariant("missing type {}", memberType.toString());
    auto *fieldType = m_state->typeCache()->getType(memberType);

    auto memberPath = m_enumUrl.getSymbolPath().getPath();
    memberPath.push_back(name);
    auto memberUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(memberPath));
    auto fieldIndex = m_state->numFields();
    auto address = FieldAddress::near(fieldIndex);

    // construct the field symbol
    FieldSymbol *fieldSymbol;
    if (init.isValid()) {
        fieldSymbol = new FieldSymbol(memberUrl, access, isVariable, init, address, fieldType, m_state);
    } else {
        fieldSymbol = new FieldSymbol(memberUrl, access, isVariable, address, fieldType, m_state);
    }

    auto status = m_state->appendField(fieldSymbol);
    if (status.notOk()) {
        delete fieldSymbol;
        return status;
    }

    m_state->typeCache()->touchType(memberType);

    DataReference ref;
    ref.symbolUrl = memberUrl;
    ref.typeDef = memberType;
    ref.referenceType = isVariable? ReferenceType::Variable : ReferenceType::Value;
    priv->members[name] = ref;

    return ref;
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::EnumSymbol::resolveMember(
    const std::string &name,
    AbstractMemberReifier &reifier,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    if (!priv->members.contains(name)) {
        if (priv->superEnum == nullptr)
            return m_state->logAndContinue(AssemblerCondition::kMissingMember,
                tempo_tracing::LogSeverity::kError,
                "missing member {}", name);
        return priv->superEnum->resolveMember(name, reifier, receiverType, thisReceiver);
    }
    const auto &member = priv->members.at(name);
    auto *sym = m_state->symbolCache()->getSymbol(member.symbolUrl);
    if (sym->getSymbolType() != SymbolType::FIELD)
        m_state->throwAssemblerInvariant("invalid field symbol {}", member.symbolUrl.toString());
    auto *fieldSymbol = cast_symbol_to_field(sym);
    auto access = fieldSymbol->getAccessType();

    bool thisSymbol = receiverType.getConcreteUrl() == m_enumUrl;

    if (thisReceiver) {
        if (access == lyric_object::AccessType::Private && !thisSymbol)
            return m_state->logAndContinue(AssemblerCondition::kInvalidAccess,
                tempo_tracing::LogSeverity::kError,
                "access to private member {} is not allowed", name);
    } else {
        if (access != lyric_object::AccessType::Public)
            return m_state->logAndContinue(AssemblerCondition::kInvalidAccess,
                tempo_tracing::LogSeverity::kError,
                "access to protected member {} is not allowed", name);
    }

    return reifier.reifyMember(name, fieldSymbol);
}

bool
lyric_assembler::EnumSymbol::isMemberInitialized(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->initializedMembers.contains(name);
}

tempo_utils::Status
lyric_assembler::EnumSymbol::setMemberInitialized(const std::string &name)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't set member initialized on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();
    if (isMemberInitialized(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidBinding,
            "member {} is already initialized", name);
    priv->initializedMembers.insert(name);
    return AssemblerStatus::ok();
}

bool
lyric_assembler::EnumSymbol::isCompletelyInitialized() const
{
    auto *priv = getPriv();
    for (const auto &member : priv->members) {
        if (!priv->initializedMembers.contains(member.first))
            return false;
    }
    return true;
}

lyric_common::SymbolUrl
lyric_assembler::EnumSymbol::getCtor() const
{
    auto location = m_enumUrl.getAssemblyLocation();
    auto path = m_enumUrl.getSymbolPath();
    return lyric_common::SymbolUrl(location, lyric_common::SymbolPath(path.getPath(), "$ctor"));
}

tu_uint32
lyric_assembler::EnumSymbol::getAllocatorTrap() const
{
    auto *priv = getPriv();
    return priv->allocatorTrap;
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::EnumSymbol::declareCtor(
    const std::vector<lyric_assembler::ParameterSpec> &parameterSpec,
    lyric_object::AccessType access,
    tu_uint32 allocatorTrap)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare ctor on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();

    auto path = m_enumUrl.getSymbolPath().getPath();
    path.push_back("$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(path));

    if (m_state->symbolCache()->hasSymbol(ctorUrl))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "ctor already defined for enum {}", m_enumUrl.toString());

    //
    auto returnType = getAssignableType();
    m_state->typeCache()->touchType(returnType);

    auto fundamentalEnum = m_state->fundamentalCache()->getFundamentalUrl(FundamentalSymbol::Enum);
    if (!m_state->symbolCache()->hasSymbol(fundamentalEnum))
        m_state->throwAssemblerInvariant("missing fundamental symbol Enum");
    m_state->symbolCache()->touchSymbol(fundamentalEnum);

//    auto deriveTypeResult = m_state->declareParameterizedType(fundamentalEnum, {returnType});
//    if (deriveTypeResult.isStatus())
//        return tempo_utils::Result<lyric_common::SymbolUrl>(deriveTypeResult.getStatus());
//    auto ctorClassType = deriveTypeResult.getResult();
//    m_state->touchType(ctorClassType);

    std::vector<lyric_object::Parameter> parameters;
    absl::flat_hash_set<std::string> names;
    absl::flat_hash_set<std::string> labels;

    AbstractResolver *resolver = priv->enumBlock.get();

    for (const auto &p : parameterSpec) {
        auto resolveParamTypeResult = resolver->resolveAssignable(p.type);
        if (resolveParamTypeResult.isStatus())
            return resolveParamTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.name = p.name;
        param.label = !p.label.empty()? p.label : p.name;
        param.placement = !p.label.empty()? lyric_object::PlacementType::Named : lyric_object::PlacementType::List;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE? true : false;
        param.typeDef = resolveParamTypeResult.getResult();

        if (!p.init.isEmpty()) {
            if (param.placement != lyric_object::PlacementType::Named) {
                return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid initializer for positional parameter {}; only named parameters can be default-initialized",
                    p.name);
            } else {
                param.placement = lyric_object::PlacementType::Opt;
            }
        }

        if (names.contains(p.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for ctor on enum {}",
                p.name, m_enumUrl.toString());
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for ctor on enum {}",
                p.label, m_enumUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    auto callIndex = m_state->numCalls();
    auto address = CallAddress::near(callIndex);

    // construct call symbol
    auto *ctorSymbol = new CallSymbol(ctorUrl, parameters, {}, returnType, m_enumUrl, access,
        address, lyric_object::CallMode::Constructor, priv->enumType, priv->enumBlock.get(), m_state);

    auto status = m_state->appendCall(ctorSymbol);
    if (status.notOk()) {
        delete ctorSymbol;
        return status;
    }

    // add bound method
    BoundMethod method;
    method.methodCall = ctorUrl;
    method.access = access;
    method.final = false;
    priv->methods["$ctor"] = method;

    // set allocator trap
    priv->allocatorTrap = allocatorTrap;

    return ctorUrl;
}

tempo_utils::Result<lyric_assembler::CtorInvoker>
lyric_assembler::EnumSymbol::resolveCtor()
{
    lyric_common::SymbolPath ctorPath = lyric_common::SymbolPath(m_enumUrl.getSymbolPath().getPath(), "$ctor");
    auto ctorUrl = lyric_common::SymbolUrl(m_enumUrl.getAssemblyLocation(), ctorPath);

    if (!m_state->symbolCache()->hasSymbol(ctorUrl))
        return m_state->logAndContinue(AssemblerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing ctor for enum {}", m_enumUrl.toString());
    auto *ctorSym = m_state->symbolCache()->getSymbol(ctorUrl);
    if (ctorSym->getSymbolType() != SymbolType::CALL)
        m_state->throwAssemblerInvariant("missing call symbol {}", ctorUrl.toString());
    if (ctorSym->getSymbolType() != SymbolType::CALL)
        m_state->throwAssemblerInvariant("invalid call symbol {}", ctorUrl.toString());
    auto *call = cast_symbol_to_call(ctorSym);

    return CtorInvoker(call, this);
}

bool
lyric_assembler::EnumSymbol::hasMethod(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

Option<lyric_assembler::BoundMethod>
lyric_assembler::EnumSymbol::getMethod(const std::string &name) const
{
    auto *priv = getPriv();
    if (priv->methods.contains(name))
        return Option<BoundMethod>(priv->methods.at(name));
    return {};
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
lyric_assembler::EnumSymbol::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::BoundMethod>::const_iterator
lyric_assembler::EnumSymbol::methodsEnd() const
{
    auto *priv = getPriv();
    return priv->methods.cend();
}

tu_uint32
lyric_assembler::EnumSymbol::numMethods() const
{
    auto *priv = getPriv();
    return priv->methods.size();
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_assembler::EnumSymbol::declareMethod(
    const std::string &name,
    const std::vector<lyric_assembler::ParameterSpec> &parameterSpec,
    const Option<lyric_assembler::ParameterSpec> &restSpec,
    const std::vector<lyric_assembler::ParameterSpec> &ctxSpec,
    const lyric_parser::Assignable &returnSpec,
    lyric_object::AccessType access)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare method on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();

    if (priv->methods.contains(name))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "method {} already defined for enum {}", name, m_enumUrl.toString());

    std::vector<lyric_object::Parameter> parameters;
    Option<lyric_object::Parameter> rest;
    absl::flat_hash_set<std::string> names;
    absl::flat_hash_set<std::string> labels;

    for (const auto &p : parameterSpec) {
        auto resolveParamTypeResult = priv->enumBlock->resolveAssignable(p.type);
        if (resolveParamTypeResult.isStatus())
            return resolveParamTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.name = p.name;
        param.label = !p.label.empty()? p.label : p.name;
        param.placement = !p.label.empty()? lyric_object::PlacementType::Named : lyric_object::PlacementType::List;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE? true : false;
        param.typeDef = resolveParamTypeResult.getResult();

        if (!p.init.isEmpty()) {
            if (param.placement != lyric_object::PlacementType::Named) {
                return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
                    tempo_tracing::LogSeverity::kError,
                    "invalid initializer for positional parameter {}; only named parameters can be default-initialized",
                    p.name);
            } else {
                param.placement = lyric_object::PlacementType::Opt;
            }
        }

        if (names.contains(p.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for method {} on enum {}",
                p.name, name, m_enumUrl.toString());
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for method {} on enum {}",
                p.label, name, m_enumUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    for (const auto &p : ctxSpec) {
        auto resolveParamTypeResult = priv->enumBlock->resolveAssignable(p.type);
        if (resolveParamTypeResult.isStatus())
            return resolveParamTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.placement = lyric_object::PlacementType::Ctx;
        param.isVariable = false;
        param.typeDef = resolveParamTypeResult.getResult();

        // if ctx parameter name is not specified, then generate a unique name
        param.name = p.name.empty()? absl::StrCat("$ctx", parameters.size()) : p.name;
        param.label = param.name;

        if (names.contains(param.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for method {} on enum {}",
                p.name, name, m_enumUrl.toString());
        names.insert(param.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for method {} on enum {}",
                p.label, name, m_enumUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        parameters.push_back(param);
    }

    if (!restSpec.isEmpty()) {
        const auto &p = restSpec.getValue();
        auto resolveRestTypeResult = priv->enumBlock->resolveAssignable(p.type);
        if (resolveRestTypeResult.isStatus())
            return resolveRestTypeResult.getStatus();

        lyric_object::Parameter param;
        param.index = parameters.size();
        param.name = p.name;
        param.label = param.name;
        param.placement = lyric_object::PlacementType::Rest;
        param.isVariable = p.binding == lyric_parser::BindingType::VARIABLE? true : false;
        param.typeDef = resolveRestTypeResult.getResult();

        if (names.contains(p.name))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "parameter {} already defined for method {} on enum {}",
                p.name, name, m_enumUrl.toString());
        names.insert(p.name);

        if (labels.contains(param.label))
            return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
                tempo_tracing::LogSeverity::kError,
                "label {} already defined for method {} on enum {}",
                p.label, name, m_enumUrl.toString());
        labels.insert(param.label);

        m_state->typeCache()->touchType(param.typeDef);
        rest = Option<lyric_object::Parameter>(param);
    }

    auto resolveReturnTypeResult = priv->enumBlock->resolveAssignable(returnSpec);
    if (resolveReturnTypeResult.isStatus())
        return resolveReturnTypeResult.getStatus();
    auto returnType = resolveReturnTypeResult.getResult();
    m_state->typeCache()->touchType(returnType);

    // build reference path to function
    auto methodPath = m_enumUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));
    auto callIndex = m_state->numCalls();
    auto address = CallAddress::near(callIndex);

    // construct call symbol
    auto *callSymbol = new CallSymbol(methodUrl, parameters, rest, returnType, m_enumUrl,
        access, address, lyric_object::CallMode::Normal, priv->enumType, priv->enumBlock.get(), m_state);

    auto status = m_state->appendCall(callSymbol);
    if (status.notOk()) {
        delete callSymbol;
        return status;
    }

    // add bound method
    priv->methods[name] = { methodUrl, access, true /* final */ };

    return methodUrl;
}

tempo_utils::Result<lyric_assembler::MethodInvoker>
lyric_assembler::EnumSymbol::resolveMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    if (!priv->methods.contains(name)) {
        if (priv->superEnum == nullptr)
            return m_state->logAndContinue(AssemblerCondition::kMissingMethod,
                tempo_tracing::LogSeverity::kError,
                "missing method {}", name);
        return priv->superEnum->resolveMethod(name, receiverType);
    }

    const auto &method = priv->methods.at(name);
    auto *methodSym = m_state->symbolCache()->getSymbol(method.methodCall);
    if (methodSym == nullptr)
        m_state->throwAssemblerInvariant("missing call symbol {}", method.methodCall.toString());
    if (methodSym->getSymbolType() != SymbolType::CALL)
        m_state->throwAssemblerInvariant("invalid call symbol {}", method.methodCall.toString());
    auto *callSymbol = cast_symbol_to_call(methodSym);
    auto access = callSymbol->getAccessType();

    bool thisSymbol = receiverType.getConcreteUrl() == m_enumUrl;

    if (thisReceiver) {
        if (access == lyric_object::AccessType::Private && !thisSymbol)
            return m_state->logAndContinue(AssemblerCondition::kInvalidAccess,
                tempo_tracing::LogSeverity::kError,
                "cannot access private method {} on {}", name, m_enumUrl.toString());
    } else {
        if (access != lyric_object::AccessType::Public)
            return m_state->logAndContinue(AssemblerCondition::kInvalidAccess,
                tempo_tracing::LogSeverity::kError,
                "cannot access protected method {} on {}", name, m_enumUrl.toString());
    }

    if (callSymbol->isInline())
        return MethodInvoker(callSymbol, callSymbol->callProc());
    if (!callSymbol->isBound())
        m_state->throwAssemblerInvariant("invalid call symbol {}", callSymbol->getSymbolUrl().toString());

    return MethodInvoker(callSymbol);
}

bool
lyric_assembler::EnumSymbol::hasImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    return priv->impls.contains(implUrl);
}

bool
lyric_assembler::EnumSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    return hasImpl(implType.getConcreteUrl());
}

lyric_assembler::ImplHandle *
lyric_assembler::EnumSymbol::getImpl(const lyric_common::SymbolUrl &implUrl) const
{
    auto *priv = getPriv();
    auto iterator = priv->impls.find(implUrl);
    if (iterator != priv->impls.cend())
        return iterator->second;
    return nullptr;
}

lyric_assembler::ImplHandle *
lyric_assembler::EnumSymbol::getImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return nullptr;
    return getImpl(implType.getConcreteUrl());
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::EnumSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::SymbolUrl,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::EnumSymbol::implsEnd() const
{
    auto *priv = getPriv();
    return priv->impls.cend();
}

tu_uint32
lyric_assembler::EnumSymbol::numImpls() const
{
    auto *priv = getPriv();
    return priv->impls.size();
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_assembler::EnumSymbol::declareImpl(const lyric_parser::Assignable &implSpec)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't declare impl on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();

    auto resolveImplTypeResult = priv->enumBlock->resolveAssignable(implSpec);
    if (resolveImplTypeResult.isStatus())
        return resolveImplTypeResult.getStatus();
    auto implType = resolveImplTypeResult.getResult();

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        m_state->throwAssemblerInvariant("invalid impl type {}", implType.toString());
    auto implUrl = implType.getConcreteUrl();

    if (priv->impls.contains(implUrl))
        return m_state->logAndContinue(AssemblerCondition::kSymbolAlreadyDefined,
            tempo_tracing::LogSeverity::kError,
            "impl {} already defined for enum {}", implType.toString(), m_enumUrl.toString());

    // touch the impl type
    auto *implTypeHandle = m_state->typeCache()->getType(implType);
    if (implTypeHandle == nullptr)
        m_state->throwAssemblerInvariant("missing type {}", implType.toString());
    TU_RETURN_IF_NOT_OK (m_state->typeCache()->touchType(implType));

    // confirm that the impl concept exists
    auto implConcept = implType.getConcreteUrl();
    if (!m_state->symbolCache()->hasSymbol(implConcept))
        m_state->throwAssemblerInvariant("missing concept symbol {}", implConcept.toString());

    // resolve the concept symbol
    auto *conceptSym = m_state->symbolCache()->getSymbol(implConcept);
    if (conceptSym->getSymbolType() != SymbolType::CONCEPT)
        m_state->throwAssemblerInvariant("invalid concept symbol {}", implConcept.toString());
    auto *conceptSymbol = cast_symbol_to_concept(conceptSym);

    conceptSymbol->touch();

    auto *implCache = m_state->implCache();

    auto name = absl::StrCat("$impl", priv->impls.size());

    ImplHandle *implHandle;
    TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(
        name, implTypeHandle, conceptSymbol, m_enumUrl, priv->enumBlock.get()));

    priv->impls[implUrl] = implHandle;

    return implType;
}

bool
lyric_assembler::EnumSymbol::hasSealedType(const lyric_common::TypeDef &sealedType) const
{
    auto *priv = getPriv();
    return priv->sealedTypes.contains(sealedType);
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::EnumSymbol::sealedTypesBegin() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::EnumSymbol::sealedTypesEnd() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cend();
}

tempo_utils::Status
lyric_assembler::EnumSymbol::putSealedType(const lyric_common::TypeDef &sealedType)
{
    if (isImported())
        m_state->throwAssemblerInvariant(
            "can't put sealed type on imported enum {}", m_enumUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "enum {} is not sealed", m_enumUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "invalid derived type {} for sealed enum {}", sealedType.toString(), m_enumUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    if (!m_state->symbolCache()->hasSymbol(sealedUrl))
        m_state->throwAssemblerInvariant("missing symbol {}", sealedUrl.toString());
    auto *sym = m_state->symbolCache()->getSymbol(sealedType.getConcreteUrl());
    TU_ASSERT (sym != nullptr);

    if (sym->getSymbolType() != SymbolType::ENUM || cast_symbol_to_enum(sym)->superEnum() != this)
        return m_state->logAndContinue(AssemblerCondition::kSyntaxError,
            tempo_tracing::LogSeverity::kError,
            "{} does not derive from sealed enum {}", sealedType.toString(), m_enumUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return AssemblerStatus::ok();
}