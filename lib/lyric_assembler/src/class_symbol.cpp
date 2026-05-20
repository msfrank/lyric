#include <absl/container/flat_hash_set.h>

#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/ctor_constructable.h>
#include <lyric_assembler/field_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/impl_cache.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/method_callable.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/stub_callable.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/type_import.h>

lyric_assembler::ClassSymbol::ClassSymbol(
    const lyric_common::SymbolUrl &classUrl,
    bool isHidden,
    bool isAbstract,
    lyric_object::DeriveType derive,
    TypeHandle *classType,
    ClassSymbol *superClass,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseSymbol(new ClassSymbolPriv()),
      m_classUrl(classUrl),
      m_state(state)
{
    TU_ASSERT (m_classUrl.isValid());
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->isHidden = isHidden;
    priv->isAbstract = isAbstract;
    priv->derive = derive;
    priv->isDeclOnly = isDeclOnly;
    priv->classType = classType;
    priv->classTemplate = nullptr;
    priv->superClass = superClass;
    priv->classBlock = std::make_unique<BlockHandle>(classUrl, parentBlock);

    TU_ASSERT (priv->classType != nullptr);
    TU_ASSERT (priv->superClass != nullptr);
}

lyric_assembler::ClassSymbol::ClassSymbol(
    const lyric_common::SymbolUrl &classUrl,
    bool isHidden,
    bool isAbstract,
    lyric_object::DeriveType derive,
    TypeHandle *classType,
    TemplateHandle *classTemplate,
    ClassSymbol *superClass,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : ClassSymbol(
        classUrl,
        isHidden,
        isAbstract,
        derive,
        classType,
        superClass,
        isDeclOnly,
        parentBlock,
        state)
{
    auto *priv = getPriv();
    priv->classTemplate = classTemplate;
    TU_ASSERT(priv->classTemplate != nullptr);
    for (auto it = classTemplate->templateParametersBegin(); it != classTemplate->templateParametersEnd(); it++) {
        const auto &tp = *it;
        TU_RAISE_IF_STATUS (priv->classBlock->declareAlias(tp.name, classTemplate->getTemplateUrl(), tp.index));
    }
}

lyric_assembler::ClassSymbol::ClassSymbol(
    const lyric_common::SymbolUrl &classUrl,
    std::shared_ptr<lyric_importer::ClassImport> classImport,
    bool isCopied,
    ObjectState *state)
    : BaseSymbol(isCopied),
      m_classUrl(classUrl),
      m_classImport(std::move(classImport)),
      m_state(state)
{
    TU_ASSERT (m_classUrl.isValid());
    TU_ASSERT (m_classImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

lyric_assembler::ClassSymbolPriv *
lyric_assembler::ClassSymbol::load()
{
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<ClassSymbolPriv>();

    priv->classBlock = std::make_unique<BlockHandle>(
        m_classUrl, absl::flat_hash_map<std::string, SymbolBinding>(), m_state);

    priv->isHidden = m_classImport->isHidden();
    priv->isAbstract = m_classImport->isAbstract();
    priv->derive = m_classImport->getDerive();
    priv->isDeclOnly = m_classImport->isDeclOnly();

    auto typeImport = m_classImport->getClassType().lock();
    if (typeImport == nullptr)
        throw tempo_utils::StatusException(
            AssemblerStatus::forCondition(AssemblerCondition::kImportError,
            "cannot import class {}; missing type",
            m_classUrl.toString()));
    TU_ASSIGN_OR_RAISE (priv->classType, typeCache->importType(typeImport));

    if (m_classImport->hasClassTemplate()) {
        auto templateImport = m_classImport->getClassTemplate().lock();
        if (templateImport == nullptr)
            throw tempo_utils::StatusException(
                AssemblerStatus::forCondition(AssemblerCondition::kImportError,
                "cannot import class {}; missing template",
                m_classUrl.toString()));
        TU_ASSIGN_OR_RAISE (priv->classTemplate, typeCache->importTemplate(templateImport));
    }

    auto superClassUrl = m_classImport->getSuperClass();
    if (superClassUrl.isValid()) {
        TU_ASSIGN_OR_RAISE (priv->superClass, importCache->importClass(superClassUrl));
    }

    for (auto it = m_classImport->membersBegin(); it != m_classImport->membersEnd(); it++) {
        FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RAISE (fieldSymbol, importCache->importField(it->second));
        TU_RAISE_IF_NOT_OK (priv->classBlock->putBinding(fieldSymbol));
        priv->members[it->first] = fieldSymbol;
    }

    for (auto it = m_classImport->methodsBegin(); it != m_classImport->methodsEnd(); it++) {
        CallSymbol *callSymbol;
        TU_ASSIGN_OR_RAISE (callSymbol, importCache->importCall(it->second));
        TU_RAISE_IF_NOT_OK (priv->classBlock->putBinding(callSymbol));
        priv->methods[it->first] = callSymbol;
    }

    for (auto it = m_classImport->stubsBegin(); it != m_classImport->stubsEnd(); it++) {
        ActionSymbol *actionSymbol;
        TU_ASSIGN_OR_RAISE (actionSymbol, importCache->importAction(it->second));
        TU_RAISE_IF_NOT_OK (priv->classBlock->putBinding(actionSymbol));
        priv->stubs[it->first] = actionSymbol;
    }

    auto *implCache = m_state->implCache();
    for (auto it = m_classImport->implsBegin(); it != m_classImport->implsEnd(); it++) {
        auto implImport = it->second.lock();
        if (implImport == nullptr)
            throw tempo_utils::StatusException(AssemblerStatus::forCondition(
                AssemblerCondition::kImportError, "invalid impl import"));
        ImplHandle *implHandle;
        TU_ASSIGN_OR_RAISE (implHandle, implCache->importImpl(implImport));
        auto contract = implHandle->getContract();

        auto consumerType = contract.getConsumerType();
        if (priv->impls.contains(consumerType))
            throw tempo_utils::StatusException(AssemblerStatus::forCondition(
                AssemblerCondition::kImportError, "impl {} is already imported for {}",
                    consumerType.toString(), m_classUrl.toString()));
        priv->impls[consumerType] = implHandle;

        auto implementationType = contract.getImplementationType();
        if (implementationType == consumerType)
            continue;
        if (priv->impls.contains(implementationType))
            throw tempo_utils::StatusException(AssemblerStatus::forCondition(
                AssemblerCondition::kImportError, "impl {} is already imported for {}",
                    implementationType.toString(), m_classUrl.toString()));
        priv->impls[implementationType] = implHandle;
    }

    for (auto iterator = m_classImport->sealedTypesBegin(); iterator != m_classImport->sealedTypesEnd(); iterator++) {
        priv->sealedTypes.insert(*iterator);
    }

    priv->allocatorTrap = m_classImport->getAllocator();

    return priv.release();
}

lyric_object::LinkageSection
lyric_assembler::ClassSymbol::getLinkage() const
{
    return lyric_object::LinkageSection::Class;
}

lyric_assembler::SymbolType
lyric_assembler::ClassSymbol::getSymbolType() const
{
    return SymbolType::CLASS;
}

lyric_common::SymbolUrl
lyric_assembler::ClassSymbol::getSymbolUrl() const
{
    return m_classUrl;
}

lyric_common::TypeDef
lyric_assembler::ClassSymbol::getTypeDef() const
{
    auto *priv = getPriv();
    return priv->classType->getTypeDef();
}

lyric_assembler::BlockHandle *
lyric_assembler::ClassSymbol::derefBlock()
{
    auto *priv = getPriv();
    return priv->classBlock.get();
}

bool
lyric_assembler::ClassSymbol::isHidden() const
{
    auto *priv = getPriv();
    return priv->isHidden;
}

bool
lyric_assembler::ClassSymbol::isAbstract() const
{
    auto *priv = getPriv();
    return priv->isAbstract;
}

lyric_object::DeriveType
lyric_assembler::ClassSymbol::getDeriveType() const
{
    auto *priv = getPriv();
    return priv->derive;
}

bool
lyric_assembler::ClassSymbol::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

lyric_assembler::ClassSymbol *
lyric_assembler::ClassSymbol::superClass() const
{
    auto *priv = getPriv();
    return priv->superClass;
}

lyric_assembler::TypeHandle *
lyric_assembler::ClassSymbol::classType() const
{
    auto *priv = getPriv();
    return priv->classType;
}

lyric_assembler::TemplateHandle *
lyric_assembler::ClassSymbol::classTemplate() const
{
    auto *priv = getPriv();
    return priv->classTemplate;
}

lyric_assembler::BlockHandle *
lyric_assembler::ClassSymbol::classBlock() const
{
    auto *priv = getPriv();
    return priv->classBlock.get();
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::ClassSymbol::resolveGlobalMember(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *symbolCache = m_state->symbolCache();
    auto *priv = getPriv();

    auto globalSymbolUrl = priv->classBlock->makeSymbolUrl(name);

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(globalSymbolUrl, /* allowMissing= */ true));
    if (symbol == nullptr) {
        if (priv->superClass == nullptr)
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
                "missing global member {}", name);
        return priv->superClass->resolveGlobalMember(name, receiverType, thisReceiver);
    }

    DataReference ref;
    bool isHidden;
    switch (symbol->getSymbolType()) {
        case SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(symbol);
            ref.referenceType = ReferenceType::Value;
            isHidden = enumSymbol->isHidden();
            break;
        }
        case SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(symbol);
            ref.referenceType = ReferenceType::Value;
            isHidden = instanceSymbol->isHidden();
            break;
        }
        case SymbolType::STATIC: {
            auto *staticSymbol = cast_symbol_to_static(symbol);
            ref.referenceType = staticSymbol->isVariable()? ReferenceType::Variable : ReferenceType::Value;
            isHidden = staticSymbol->isHidden();
            break;
        }
        default:
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
                "missing member {}", name);
    }

    bool thisSymbol = receiverType.getConcreteUrl() == m_classUrl;
    if (isHidden && !(thisReceiver && thisSymbol))
        return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
            "access to hidden member {} is not allowed", name);
    ref.symbolUrl = globalSymbolUrl;
    ref.typeDef = symbol->getTypeDef();
    return ref;
}

tempo_utils::Status
lyric_assembler::ClassSymbol::prepareGlobalMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    std::unique_ptr<AbstractCallable> &callable,
    bool thisReceiver) const
{
    auto *symbolCache = m_state->symbolCache();
    auto *priv = getPriv();

    auto globalSymbolUrl = priv->classBlock->makeSymbolUrl(name);

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(globalSymbolUrl, /* allowMissing= */ true));
    if (symbol == nullptr) {
        if (priv->superClass == nullptr)
            return AssemblerStatus::forCondition(AssemblerCondition::kMissingMethod,
                "missing global method {}", name);
        return priv->superClass->prepareGlobalMethod(name, receiverType, callable, thisReceiver);
    }

    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", symbol->getSymbolUrl().toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    if (callSymbol->isHidden()) {
        if (!thisReceiver)
            return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                "cannot access hidden method {} on {}", name, m_classUrl.toString());
    }

    if (callSymbol->isBound())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", callSymbol->getSymbolUrl().toString());

    callable = std::make_unique<FunctionCallable>(callSymbol, callSymbol->isInline());
    return {};
}

bool
lyric_assembler::ClassSymbol::hasMember(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->members.contains(name);
}

lyric_assembler::FieldSymbol *
lyric_assembler::ClassSymbol::getMember(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->members.find(name);
    if (entry != priv->members.cend())
        return entry->second;
    return nullptr;
}

absl::flat_hash_map<std::string,lyric_assembler::FieldSymbol *>::const_iterator
lyric_assembler::ClassSymbol::membersBegin() const
{
    auto *priv = getPriv();
    return priv->members.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::FieldSymbol *>::const_iterator
lyric_assembler::ClassSymbol::membersEnd() const
{
    auto *priv = getPriv();
    return priv->members.cend();
}

tu_uint32
lyric_assembler::ClassSymbol::numMembers() const
{
    auto *priv = getPriv();
    return static_cast<tu_uint32>(priv->members.size());
}

static lyric_assembler::AbstractSymbol *
find_existing_or_overridden_class_binding(
    lyric_assembler::ClassSymbol *classSymbol,
    const std::string &name)
{
    // if class contains the named binding then return the ClassSymbol pointer
    auto *block = classSymbol->classBlock();
    if (block->hasBinding(name))
        return classSymbol;

    // otherwise if a superclass contains the named binding then return pointer to the binding symbol
    for (auto *currClass = classSymbol->superClass(); currClass != nullptr; currClass = currClass->superClass()) {
        block = currClass->classBlock();
        if (!block->hasBinding(name))
            continue;

        auto binding = block->getBinding(name);
        if (binding.bindingType == lyric_assembler::BindingType::Descriptor) {
            auto *state = block->blockState();
            auto *symbolCache = state->symbolCache();
            return symbolCache->getSymbolOrNull(binding.symbolUrl);
        }
    }

    // otherwise no binding exists in any superclass
    return nullptr;
}

tempo_utils::Result<lyric_assembler::FieldSymbol *>
lyric_assembler::ClassSymbol::declareMember(
    const std::string &name,
    const lyric_common::TypeDef &memberType,
    bool isVariable,
    bool isHidden)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare member on imported class {}", m_classUrl.toString());

    auto *priv = getPriv();

    auto *existingOrOverridden = find_existing_or_overridden_class_binding(this, name);
    if (existingOrOverridden == this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "'{}' is already defined for class {}", name, m_classUrl.toString());
    if (existingOrOverridden != nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "{} cannot be overridden by class {}",
            existingOrOverridden->getSymbolUrl().toString(), m_classUrl.toString());

    TypeHandle *fieldType;
    TU_ASSIGN_OR_RETURN (fieldType, m_state->typeCache()->getOrMakeType(memberType));

    auto memberPath = m_classUrl.getSymbolPath().getPath();
    memberPath.push_back(name);
    auto memberUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(memberPath));

    // construct the field symbol
    auto fieldSymbol = std::make_unique<FieldSymbol>(memberUrl, isHidden, isVariable,
        fieldType, priv->isDeclOnly, priv->classBlock.get(), m_state);

    FieldSymbol *fieldPtr;
    TU_ASSIGN_OR_RETURN (fieldPtr, m_state->appendField(std::move(fieldSymbol)));
    TU_RETURN_IF_NOT_OK (priv->classBlock->putBinding(fieldPtr));

    priv->members[name] = fieldPtr;

    return fieldPtr;
}

tempo_utils::Result<lyric_assembler::DataReference>
lyric_assembler::ClassSymbol::resolveMember(
    const std::string &name,
    AbstractMemberReifier &reifier,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    auto entry = priv->members.find(name);
    if (entry != priv->members.cend()) {
        auto *fieldSymbol = entry->second;

        if (fieldSymbol->isHidden()) {
            bool thisSymbol = receiverType.getConcreteUrl() == m_classUrl;
            if (!(thisReceiver && thisSymbol))
                return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                    "access to hidden member {} is not allowed", name);
        }

        return reifier.reifyMember(name, fieldSymbol);
    }

    if (priv->superClass == nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingMember,
            "missing member {}", name);
    return priv->superClass->resolveMember(name, reifier, receiverType, thisReceiver);
}

bool
lyric_assembler::ClassSymbol::isMemberInitialized(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->initializedMembers.contains(name);
}

tempo_utils::Status
lyric_assembler::ClassSymbol::setMemberInitialized(const std::string &name)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't set member initialized on imported class {}", m_classUrl.toString());

    auto *priv = getPriv();
    if (isMemberInitialized(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "member {} was already initialized", name);
    priv->initializedMembers.insert(name);
    return {};
}

bool
lyric_assembler::ClassSymbol::isCompletelyInitialized() const
{
    auto *priv = getPriv();
    for (const auto &member : priv->members) {
        if (!priv->initializedMembers.contains(member.first))
            return false;
    }
    return true;
}

std::string
lyric_assembler::ClassSymbol::getAllocatorTrap() const
{
    auto *priv = getPriv();
    return priv->allocatorTrap;
}

bool
lyric_assembler::ClassSymbol::hasCtor(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->methods.find(name);
    if (entry == priv->methods.cend())
        return false;
    auto *callSymbol = entry->second;
    return callSymbol->isCtor();
}

lyric_assembler::CallSymbol *
lyric_assembler::ClassSymbol::getCtor(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->methods.find(name);
    if (entry == priv->methods.cend())
        return {};
    auto *callSymbol = entry->second;
    if (callSymbol->isCtor())
        return callSymbol;
    return nullptr;
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::ClassSymbol::declareCtor(
    const std::string &name,
    bool isHidden,
    std::string allocatorTrap)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare ctor on imported class {}", m_classUrl.toString());

    auto *priv = getPriv();

    auto path = m_classUrl.getSymbolPath().getPath();
    path.emplace_back(name);
    auto ctorUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(path));

    auto *existingSymbol = m_state->symbolCache()->getSymbolOrNull(ctorUrl);
    if (existingSymbol != nullptr) {
        if (existingSymbol->getSymbolType() == SymbolType::CALL) {
            auto existingCall = cast_symbol_to_call(existingSymbol);
            if (existingCall->isCtor()) {
                auto friendlyName = name == lyric_object::kCtorSpecialSymbol?
                    "default" : absl::StrCat("'", name, "'");
                return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
                    "{} constructor already defined for class {}",
                    friendlyName, m_classUrl.toString());
            }
        }
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "'{}' already defined for class {}", name, m_classUrl.toString());
    }

    // construct call symbol
    std::unique_ptr<CallSymbol> ctorSymbol;
    if (priv->classTemplate != nullptr) {
        ctorSymbol = std::make_unique<CallSymbol>(ctorUrl, m_classUrl, isHidden,
            lyric_object::CallMode::Constructor, /* isFinal= */ false, priv->classTemplate,
            priv->isDeclOnly, priv->classBlock.get(), m_state);
    } else {
        ctorSymbol = std::make_unique<CallSymbol>(ctorUrl, m_classUrl, isHidden,
            lyric_object::CallMode::Constructor, /* isFinal= */ false, priv->isDeclOnly,
            priv->classBlock.get(), m_state);
    }

    CallSymbol *ctorPtr;
    TU_ASSIGN_OR_RETURN (ctorPtr, m_state->appendCall(std::move(ctorSymbol)));
    TU_RETURN_IF_NOT_OK (priv->classBlock->putBinding(ctorPtr));
    priv->methods[name] = ctorPtr;

    // set allocator trap
    priv->allocatorTrap = std::move(allocatorTrap);

    return ctorPtr;
}

tempo_utils::Status
lyric_assembler::ClassSymbol::prepareCtor(const std::string &name, std::unique_ptr<AbstractCallable> &callable)
{
    lyric_common::SymbolPath ctorPath = lyric_common::SymbolPath(
        m_classUrl.getSymbolPath().getPath(), name);
    auto ctorUrl = lyric_common::SymbolUrl(m_classUrl.getModuleLocation(), ctorPath);

    AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(ctorUrl));
    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", ctorUrl.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    callable = std::make_unique<CtorConstructable>(callSymbol, this);
    return {};
}

bool
lyric_assembler::ClassSymbol::hasMethod(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

lyric_assembler::CallSymbol *
lyric_assembler::ClassSymbol::getMethod(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->methods.find(name);
    if (entry != priv->methods.cend())
        return entry->second;
    return nullptr;
}

absl::flat_hash_map<std::string,lyric_assembler::CallSymbol *>::const_iterator
lyric_assembler::ClassSymbol::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->methods.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::CallSymbol *>::const_iterator
lyric_assembler::ClassSymbol::methodsEnd() const
{
    auto *priv = getPriv();
    return priv->methods.cend();
}

tu_uint32
lyric_assembler::ClassSymbol::numMethods() const
{
    auto *priv = getPriv();
    return static_cast<tu_uint32>(priv->methods.size());
}

tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_assembler::ClassSymbol::declareMethod(
    const std::string &name,
    bool isHidden,
    bool isFinal,
    const std::vector<lyric_object::TemplateParameter> &templateParameters)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare method on imported class {}", m_classUrl.toString());

    auto *priv = getPriv();

    auto *existingOrOverridden = find_existing_or_overridden_class_binding(this, name);
    if (existingOrOverridden == this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "{} already defined for class {}", name, m_classUrl.toString());

    // determine the base symbol if it exists
    lyric_common::SymbolUrl baseUrl;
    if (existingOrOverridden != nullptr) {
        switch (existingOrOverridden->getSymbolType()) {
            case SymbolType::ACTION:
                baseUrl = existingOrOverridden->getSymbolUrl();
                break;
            case SymbolType::CALL: {
                auto *baseCall = cast_symbol_to_call(existingOrOverridden);
                if (baseCall->isFinal())
                    return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
                        "final method {} cannot be overridden by class {}",
                        existingOrOverridden->getSymbolUrl().toString(), m_classUrl.toString());
                baseUrl = existingOrOverridden->getSymbolUrl();
                break;
            }
            default:
                return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
                    "{} cannot be overridden by class {}",
                    existingOrOverridden->getSymbolUrl().toString(), m_classUrl.toString());
        }
    }

    // build reference path to function
    auto methodPath = m_classUrl.getSymbolPath().getPath();
    methodPath.push_back(name);
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));

    TemplateHandle *methodTemplate;

    // if template parameters were specified then construct a template for the method0
    if (!templateParameters.empty()) {
        auto *typeCache = m_state->typeCache();
        if (priv->classTemplate != nullptr) {
            TU_ASSIGN_OR_RETURN (methodTemplate, typeCache->makeTemplate(
                methodUrl, priv->classTemplate, templateParameters, priv->classBlock.get()));
        } else {
            TU_ASSIGN_OR_RETURN (methodTemplate, typeCache->makeTemplate(
                methodUrl, templateParameters, priv->classBlock.get()));
        }
    } else {
        methodTemplate = priv->classTemplate;
    }

    // construct call symbol
    std::unique_ptr<CallSymbol> callSymbol;
    if (methodTemplate != nullptr) {
        if (baseUrl.isValid()) {
            callSymbol = std::make_unique<CallSymbol>(methodUrl, m_classUrl, isHidden, baseUrl, isFinal,
                methodTemplate, priv->isDeclOnly, priv->classBlock.get(), m_state);
        } else {
            callSymbol = std::make_unique<CallSymbol>(methodUrl, m_classUrl, isHidden,
                lyric_object::CallMode::Normal, isFinal, methodTemplate, priv->isDeclOnly,
                priv->classBlock.get(), m_state);
        }
    } else {
        if (baseUrl.isValid()) {
            callSymbol = std::make_unique<CallSymbol>(methodUrl, m_classUrl, isHidden, baseUrl, isFinal,
                priv->isDeclOnly, priv->classBlock.get(), m_state);
        } else {
            callSymbol = std::make_unique<CallSymbol>(methodUrl, m_classUrl, isHidden,
                lyric_object::CallMode::Normal, isFinal, priv->isDeclOnly, priv->classBlock.get(),
                m_state);
        }
    }

    CallSymbol *callPtr;
    TU_ASSIGN_OR_RETURN (callPtr, m_state->appendCall(std::move(callSymbol)));
    TU_RETURN_IF_NOT_OK (priv->classBlock->putBinding(callPtr));
    priv->methods[name] = callPtr;

    return callPtr;
}

tempo_utils::Status
lyric_assembler::ClassSymbol::prepareMethod(
    const std::string &name,
    const lyric_common::TypeDef &receiverType,
    std::unique_ptr<AbstractCallable> &callable,
    bool thisReceiver) const
{
    auto *priv = getPriv();

    auto method = priv->methods.find(name);
    if (method != priv->methods.cend()) {
        auto *callSymbol = method->second;

        if (callSymbol->isHidden()) {
            if (!thisReceiver)
                return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                    "cannot access hidden method {} on {}", name, m_classUrl.toString());
        }

        if (!callSymbol->isBound())
            return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                "invalid call symbol {}", callSymbol->getSymbolUrl().toString());

        if (!callSymbol->hasBaseUrl()) {
            callable = std::make_unique<MethodCallable>(callSymbol, callSymbol->isInline());
            return {};
        }

        auto *symbolCache = m_state->symbolCache();

        AbstractSymbol *baseSymbol;
        TU_ASSIGN_OR_RETURN (baseSymbol, symbolCache->getOrImportSymbol(callSymbol->getBaseUrl()));
        switch (baseSymbol->getSymbolType()) {
            case SymbolType::ACTION:
                callable = std::make_unique<StubCallable>(cast_symbol_to_action(baseSymbol));
                return {};
            case SymbolType::CALL:
                callable = std::make_unique<MethodCallable>(cast_symbol_to_call(baseSymbol));
                return {};
            default:
                return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
                    "invalid base symbol {}", baseSymbol->getSymbolUrl().toString());
        }
    }

    auto stub = priv->stubs.find(name);
    if (stub != priv->stubs.cend()) {
        auto *actionSymbol = stub->second;

        if (actionSymbol->isHidden()) {
            if (!thisReceiver)
                return AssemblerStatus::forCondition(AssemblerCondition::kInvalidAccess,
                    "cannot access hidden method {} on {}", name, m_classUrl.toString());
        }

        callable = std::make_unique<StubCallable>(actionSymbol);
        return {};
    }

    if (priv->superClass == nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingMethod,
            "missing method {}", name);
    return priv->superClass->prepareMethod(name, receiverType, callable);
}

bool
lyric_assembler::ClassSymbol::hasStub(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->methods.contains(name);
}

lyric_assembler::ActionSymbol *
lyric_assembler::ClassSymbol::getStub(const std::string &name) const
{
    auto *priv = getPriv();
    auto entry = priv->stubs.find(name);
    if (entry != priv->stubs.cend())
        return entry->second;
    return nullptr;
}

absl::flat_hash_map<std::string,lyric_assembler::ActionSymbol *>::const_iterator
lyric_assembler::ClassSymbol::stubsBegin() const
{
    auto *priv = getPriv();
    return priv->stubs.cbegin();
}

absl::flat_hash_map<std::string,lyric_assembler::ActionSymbol *>::const_iterator
lyric_assembler::ClassSymbol::stubsEnd() const
{
    auto *priv = getPriv();
    return priv->stubs.cend();
}

tu_uint32
lyric_assembler::ClassSymbol::numStubs() const
{
    auto *priv = getPriv();
    return static_cast<tu_uint32>(priv->stubs.size());
}

tempo_utils::Result<lyric_assembler::ActionSymbol *>
lyric_assembler::ClassSymbol::declareStub(
    const std::string &name,
    bool isHidden,
    const std::vector<lyric_object::TemplateParameter> &templateParameters)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare stub on imported class {}", m_classUrl.toString());

    auto *priv = getPriv();

    auto *existingOrOverridden = find_existing_or_overridden_class_binding(this, name);
    if (existingOrOverridden == this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "{} already defined for class {}", name, m_classUrl.toString());
    if (existingOrOverridden != nullptr)
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "{} cannot be overridden", existingOrOverridden->getSymbolUrl().toString());

    // build reference path to stub
    auto stubPath = m_classUrl.getSymbolPath().getPath();
    stubPath.push_back(name);
    auto stubUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(stubPath));

    TemplateHandle *methodTemplate;

    // if template parameters were specified then construct a template for the stub
    if (!templateParameters.empty()) {
        auto *typeCache = m_state->typeCache();
        if (priv->classTemplate != nullptr) {
            TU_ASSIGN_OR_RETURN (methodTemplate, typeCache->makeTemplate(
                stubUrl, priv->classTemplate, templateParameters, priv->classBlock.get()));
        } else {
            TU_ASSIGN_OR_RETURN (methodTemplate, typeCache->makeTemplate(
                stubUrl, templateParameters, priv->classBlock.get()));
        }
    } else {
        methodTemplate = priv->classTemplate;
    }

    // construct call symbol
    std::unique_ptr<ActionSymbol> actionSymbol;
    if (methodTemplate != nullptr) {
        actionSymbol = std::make_unique<ActionSymbol>(stubUrl, m_classUrl, isHidden,
            methodTemplate, priv->isDeclOnly, priv->classBlock.get(), m_state);
    } else {
        actionSymbol = std::make_unique<ActionSymbol>(stubUrl, m_classUrl, isHidden,
            priv->isDeclOnly, priv->classBlock.get(), m_state);
    }

    ActionSymbol *actionPtr;
    TU_ASSIGN_OR_RETURN (actionPtr, m_state->appendAction(std::move(actionSymbol)));
    TU_RETURN_IF_NOT_OK (priv->classBlock->putBinding(actionPtr));
    priv->stubs[name] = actionPtr;

    return actionPtr;
}

bool
lyric_assembler::ClassSymbol::hasImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    auto *priv = getPriv();
    return priv->impls.contains(implType);
}

lyric_assembler::ImplHandle *
lyric_assembler::ClassSymbol::getImpl(const lyric_common::TypeDef &implType) const
{
    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return nullptr;
    auto *priv = getPriv();
    auto iterator = priv->impls.find(implType);
    if (iterator != priv->impls.cend())
        return iterator->second;
    return nullptr;
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ClassSymbol::implsBegin() const
{
    auto *priv = getPriv();
    return priv->impls.cbegin();
}

absl::flat_hash_map<lyric_common::TypeDef,lyric_assembler::ImplHandle *>::const_iterator
lyric_assembler::ClassSymbol::implsEnd() const
{
    auto *priv = getPriv();
    return priv->impls.cend();
}

tu_uint32
lyric_assembler::ClassSymbol::numImpls() const
{
    auto *priv = getPriv();
    return priv->impls.size();
}

tempo_utils::Result<lyric_assembler::ImplHandle *>
lyric_assembler::ClassSymbol::declareImpl(const lyric_common::TypeDef &implType)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare impl on imported class {}", m_classUrl.toString());

    auto *priv = getPriv();

    if (implType.getType() != lyric_common::TypeDefType::Concrete)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant,"invalid impl type {}", implType.toString());

    if (priv->impls.contains(implType))
        return AssemblerStatus::forCondition(AssemblerCondition::kSymbolAlreadyDefined,
            "impl {} already defined for class {}", implType.toString(), m_classUrl.toString());

    auto *implCache = m_state->implCache();

    ImplHandle *implHandle;
    if (priv->classTemplate != nullptr) {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(implType, m_classUrl,
            priv->classTemplate, priv->isDeclOnly, priv->classBlock.get()));
    } else {
        TU_ASSIGN_OR_RETURN (implHandle, implCache->makeImpl(implType, m_classUrl,
            priv->isDeclOnly, priv->classBlock.get()));
    }

    priv->impls[implType] = implHandle;

    return implHandle;
}

bool
lyric_assembler::ClassSymbol::hasSealedType(const lyric_common::TypeDef &sealedType) const
{
    auto *priv = getPriv();
    return priv->sealedTypes.contains(sealedType);
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ClassSymbol::sealedTypesBegin() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cbegin();
}

absl::flat_hash_set<lyric_common::TypeDef>::const_iterator
lyric_assembler::ClassSymbol::sealedTypesEnd() const
{
    auto *priv = getPriv();
    return priv->sealedTypes.cend();
}

tempo_utils::Status
lyric_assembler::ClassSymbol::putSealedType(const lyric_common::TypeDef &sealedType)
{
    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't put sealed type on imported class {}", m_classUrl.toString());

    auto *priv = getPriv();

    if (priv->derive != lyric_object::DeriveType::Sealed)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "class {} is not sealed", m_classUrl.toString());
    if (sealedType.getType() != lyric_common::TypeDefType::Concrete)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "invalid derived type {} for sealed class {}", sealedType.toString(), m_classUrl.toString());
    auto sealedUrl = sealedType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(sealedUrl));
    if (symbol->getSymbolType() != SymbolType::CLASS)
        return AssemblerStatus::forCondition(
            AssemblerCondition::kAssemblerInvariant,"invalid class symbol {}", sealedUrl.toString());

    if (cast_symbol_to_class(symbol)->superClass() != this)
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "{} does not derive from sealed class {}", sealedType.toString(), m_classUrl.toString());

    priv->sealedTypes.insert(sealedType);

    return {};
}
