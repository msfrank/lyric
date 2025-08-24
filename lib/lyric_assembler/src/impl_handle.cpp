
#include "lyric_assembler/call_symbol.h"
#include "lyric_assembler/class_symbol.h"
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/extension_callable.h>
#include <lyric_assembler/impl_handle.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/type_cache.h>

lyric_assembler::ImplHandle::ImplHandle(
    const std::string &name,
    TypeHandle *implType,
    ConceptSymbol *implConcept,
    const lyric_common::SymbolUrl &receiverUrl,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : BaseHandle<ImplHandlePriv>(new ImplHandlePriv()),
      m_state(state)
{
    TU_ASSERT (m_state != nullptr);

    auto *priv = getPriv();
    priv->name = name;
    priv->isDeclOnly = isDeclOnly;
    priv->implType = implType;
    priv->implConcept = implConcept;
    priv->receiverUrl = receiverUrl;
    priv->implBlock =std::make_unique<BlockHandle>(receiverUrl, parentBlock);

    TU_ASSERT (!priv->name.empty());
    TU_ASSERT (priv->implType != nullptr);
    TU_ASSERT (priv->implConcept != nullptr);
    TU_ASSERT (priv->receiverUrl.isValid());

    priv->ref = { priv->receiverUrl, priv->implType->getTypeDef() };
}

lyric_assembler::ImplHandle::ImplHandle(
    const std::string &name,
    TypeHandle *implType,
    ConceptSymbol *implConcept,
    const lyric_common::SymbolUrl &receiverUrl,
    TemplateHandle *receiverTemplate,
    bool isDeclOnly,
    BlockHandle *parentBlock,
    ObjectState *state)
    : ImplHandle(
        name,
        implType,
        implConcept,
        receiverUrl,
        isDeclOnly,
        parentBlock,
        state)
{
    auto *priv = getPriv();
    priv->receiverTemplate = receiverTemplate;
    TU_ASSERT(priv->receiverTemplate != nullptr);
}

lyric_assembler::ImplHandle::ImplHandle(lyric_importer::ImplImport *implImport, ObjectState *state)
    : m_implImport(implImport),
      m_state(state)
{
    TU_ASSERT (m_implImport != nullptr);
    TU_ASSERT (m_state != nullptr);
}

//lyric_assembler::ImplOffset
//lyric_assembler::ImplHandle::getOffset() const
//{
//    auto *priv = getPriv();
//    return priv->offset;
//}

lyric_assembler::ImplRef
lyric_assembler::ImplHandle::getRef() const
{
    auto *priv = getPriv();
    return priv->ref;
}

std::string
lyric_assembler::ImplHandle::getName() const
{
    auto *priv = getPriv();
    return priv->name;
}

bool
lyric_assembler::ImplHandle::isDeclOnly() const
{
    auto *priv = getPriv();
    return priv->isDeclOnly;
}

lyric_assembler::TypeHandle *
lyric_assembler::ImplHandle::implType() const
{
    auto *priv = getPriv();
    return priv->implType;
}

lyric_assembler::ConceptSymbol *
lyric_assembler::ImplHandle::implConcept() const
{
    auto *priv = getPriv();
    return priv->implConcept;
}

lyric_assembler::BlockHandle *
lyric_assembler::ImplHandle::implBlock() const
{
    auto *priv = getPriv();
    return priv->implBlock.get();
}

lyric_common::SymbolUrl
lyric_assembler::ImplHandle::getReceiverUrl() const
{
    auto *priv = getPriv();
    return priv->receiverUrl;
}

bool
lyric_assembler::ImplHandle::hasExtension(const std::string &name) const
{
    auto *priv = getPriv();
    return priv->extensions.contains(name);
}

Option<lyric_assembler::ExtensionMethod>
lyric_assembler::ImplHandle::getExtension(const std::string &name) const
{
    auto *priv = getPriv();
    auto iterator = priv->extensions.find(name);
    if (iterator == priv->extensions.cend())
        return Option<lyric_assembler::ExtensionMethod>();
    return Option(iterator->second);
}

absl::flat_hash_map<std::string, lyric_assembler::ExtensionMethod>::const_iterator
lyric_assembler::ImplHandle::methodsBegin() const
{
    auto *priv = getPriv();
    return priv->extensions.cbegin();
}

absl::flat_hash_map<std::string, lyric_assembler::ExtensionMethod>::const_iterator
lyric_assembler::ImplHandle::methodsEnd() const
{
    auto *priv = getPriv();
    return priv->extensions.cend();
}

tu_uint32
lyric_assembler::ImplHandle::numExtensions() const
{
    auto *priv = getPriv();
    return priv->extensions.size();
}

tempo_utils::Result<lyric_assembler::ProcHandle *>
lyric_assembler::ImplHandle::defineExtension(
    const std::string &name,
    const ParameterPack &parameterPack,
    const lyric_common::TypeDef &returnType)
{
    auto *priv = getPriv();

    if (isImported())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "can't declare extension on impl {} from imported receiver {}",
            priv->implType->getTypeDef().toString(), priv->receiverUrl.toString());

    if (priv->extensions.contains(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "extension {} already defined on impl {} in receiver {}",
            name, priv->implType->getTypeDef().toString(), priv->receiverUrl.toString());

    auto actionOption = priv->implConcept->getAction(name);
    if (actionOption.isEmpty())
        return AssemblerStatus::forCondition(AssemblerCondition::kSyntaxError,
            "no such action {} for concept {}",
            name, priv->implConcept->getSymbolUrl().toString());
    auto actionUrl = actionOption.getValue().methodAction;

    // build reference path to function
    auto methodPath = priv->receiverUrl.getSymbolPath().getPath();
    methodPath.push_back(absl::StrCat(priv->name, "$", name));
    auto methodUrl = lyric_common::SymbolUrl(lyric_common::SymbolPath(methodPath));

    TemplateHandle *conceptTemplate = priv->implConcept->conceptTemplate();

    // construct call symbol
    std::unique_ptr<CallSymbol> callSymbol;
    if (conceptTemplate != nullptr) {
        callSymbol = std::make_unique<CallSymbol>(methodUrl, priv->receiverUrl, /* isHidden= */ false,
            lyric_object::CallMode::Normal, /* isFinal= */ true, conceptTemplate, priv->isDeclOnly,
            priv->implBlock.get(), m_state);
    } else {
        callSymbol = std::make_unique<CallSymbol>(methodUrl, priv->receiverUrl, /* isHidden= */ false,
            lyric_object::CallMode::Normal, /* isFinal= */ true, priv->isDeclOnly,
            priv->implBlock.get(), m_state);
    }

    CallSymbol *callPtr;
    TU_ASSIGN_OR_RETURN (callPtr, m_state->appendCall(std::move(callSymbol)));

    // TODO: validate that the parameter types and return type match the action

    ProcHandle *extensionProc;
    TU_ASSIGN_OR_RETURN (extensionProc, callPtr->defineCall(parameterPack, returnType));

    // add extension method
    ExtensionMethod extension;
    extension.methodAction = actionUrl;
    extension.methodCall = methodUrl;
    priv->extensions[name] = std::move(extension);

    return extensionProc;
}

tempo_utils::Status
lyric_assembler::ImplHandle::prepareExtension(
    const std::string &name,
    const DataReference &ref,
    CallableInvoker &invoker)
{
    auto *priv = getPriv();

    if (!priv->extensions.contains(name))
        return AssemblerStatus::forCondition(AssemblerCondition::kMissingMethod,
            "missing extension {}", name);

    auto refUrl = ref.typeDef.getConcreteUrl();
    if (refUrl != priv->receiverUrl)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "ref {} is not an instance of impl receiver {}",
            ref.symbolUrl.toString(), priv->receiverUrl.toString());

    const auto &extension = priv->extensions.at(name);
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, m_state->symbolCache()->getOrImportSymbol(extension.methodCall));
    if (symbol->getSymbolType() != SymbolType::CALL)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid call symbol {}", extension.methodCall.toString());
    auto *callSymbol = cast_symbol_to_call(symbol);

    auto isHidden = callSymbol->isHidden();
    if (isHidden)
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "extension method {} must be public", name);

    if (callSymbol->isInline()) {
        auto callable = std::make_unique<ExtensionCallable>(callSymbol);
        return invoker.initialize(std::move(callable));
    }

    if (!callSymbol->isBound())
        return AssemblerStatus::forCondition(AssemblerCondition::kAssemblerInvariant,
            "invalid extension call {}", extension.methodCall.toString());

    auto callable = std::make_unique<ExtensionCallable>(callSymbol, ref);
    return invoker.initialize(std::move(callable));
}

bool
lyric_assembler::ImplHandle::isCompletelyDefined() const
{
    auto *priv = getPriv();

    auto *conceptSymbol = priv->implConcept;
    for (auto it = conceptSymbol->actionsBegin(); it != conceptSymbol->actionsEnd(); it++) {
        const auto &name = it->first;
        if (!priv->extensions.contains(name))
            return false;
    }
    return  true;
}

lyric_assembler::ImplHandlePriv *
lyric_assembler::ImplHandle::load()
{
    auto *importCache = m_state->importCache();
    auto *typeCache = m_state->typeCache();

    auto priv = std::make_unique<ImplHandlePriv>();

    priv->isDeclOnly = m_implImport->isDeclOnly();

    auto *implType = m_implImport->getImplType();
    TU_ASSIGN_OR_RAISE (priv->implType, typeCache->importType(implType));
    TU_ASSIGN_OR_RAISE (priv->implConcept, importCache->importConcept(m_implImport->getImplConcept()));
    priv->receiverUrl = m_implImport->getReceiverUrl();

    priv->ref = { priv->receiverUrl, priv->implType->getTypeDef() };

    for (auto iterator = m_implImport->extensionsBegin(); iterator != m_implImport->extensionsEnd(); iterator++) {
        auto &extension = iterator->second;

        TU_RAISE_IF_STATUS(importCache->importAction(extension.actionUrl));
        TU_RAISE_IF_STATUS(importCache->importCall(extension.callUrl));

        ExtensionMethod method;
        method.methodAction = extension.actionUrl;
        method.methodCall = extension.callUrl;
        priv->extensions[iterator->first] = method;
    }

    return priv.release();
}