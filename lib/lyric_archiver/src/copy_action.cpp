
#include <lyric_archiver/archive_utils.h>
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/copy_action.h>
#include <lyric_archiver/copy_call.h>
#include <lyric_archiver/copy_template.h>
#include <lyric_archiver/copy_type.h>
#include <lyric_archiver/scan_proc.h>
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/template_import.h>


tempo_utils::Result<lyric_assembler::ActionSymbol *>
lyric_archiver::copy_action(
    lyric_importer::ActionImport *actionImport,
    const std::string &importHash,
    lyric_assembler::NamespaceSymbol *targetNamespace,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    auto *objectState = archiverState.objectState();
    auto *symbolCache = objectState->symbolCache();
    auto *namespaceBlock = targetNamespace->namespaceBlock();

    auto importUrl = actionImport->getSymbolUrl();
    auto namespaceUrl = targetNamespace->getSymbolUrl();
    auto actionUrl = build_relative_url(importHash, importUrl);

    // if action is already present in the symbol cache then we are done
    if (symbolCache->hasSymbol(actionUrl)) {
        auto *sym = symbolCache->getSymbolOrNull(actionUrl);
        if (sym->getSymbolType() == lyric_assembler::SymbolType::ACTION)
            return cast_symbol_to_action(sym);
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "cannot archive {}; symbol is already defined", importUrl.toString());
    }

    // we expect that the action receiver is already copied
    auto receiverUrl = build_relative_url(importHash, actionImport->getReceiverUrl());
    if (!symbolCache->hasSymbol(receiverUrl))
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "cannot archive {}; missing receiver", importUrl.toString());

    auto *templateImport = actionImport->getActionTemplate();
    lyric_assembler::TemplateHandle *actionTemplate = nullptr;
    if (templateImport != nullptr) {
        TU_ASSIGN_OR_RETURN (actionTemplate, copy_template(templateImport, actionUrl, objectState));
    }

    auto isHidden = actionImport->isHidden();

    // declare the action
    std::unique_ptr<lyric_assembler::ActionSymbol> actionSymbol;

    if (actionTemplate != nullptr) {
        actionSymbol = std::make_unique<lyric_assembler::ActionSymbol>(
            actionUrl, receiverUrl, isHidden, actionTemplate, /* isDeclOnly= */ false, namespaceBlock, objectState);

    } else {
        actionSymbol = std::make_unique<lyric_assembler::ActionSymbol>(
            actionUrl, receiverUrl, isHidden, /* isDeclOnly= */ false, namespaceBlock, objectState);
    }

    // define the action
    TU_RETURN_IF_NOT_OK (define_action(
        actionImport, actionSymbol.get(), importHash, targetNamespace, symbolReferenceSet, archiverState));

    // append the action to the object
    lyric_assembler::ActionSymbol *actionSymbolPtr;
    TU_ASSIGN_OR_RETURN (actionSymbolPtr, objectState->appendAction(std::move(actionSymbol)));

    // add the action to the copied symbols map
    TU_RETURN_IF_NOT_OK (archiverState.putSymbol(importUrl, actionSymbolPtr));

    return actionSymbolPtr;
}

tempo_utils::Status
lyric_archiver::define_action(
    lyric_importer::ActionImport *actionImport,
    lyric_assembler::ActionSymbol *actionSymbol,
    const std::string &importHash,
    lyric_assembler::NamespaceSymbol *targetNamespace,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    auto importUrl = actionImport->getSymbolUrl();

    lyric_assembler::ParameterPack parameterPack;

    for (auto it = actionImport->listParametersBegin(); it != actionImport->listParametersEnd(); it++) {
        lyric_assembler::TypeHandle *paramTypeHandle;
        TU_ASSIGN_OR_RETURN (paramTypeHandle, copy_type(
            it->type, importHash, targetNamespace, symbolReferenceSet, archiverState));

        lyric_assembler::Parameter p;
        p.index = it->index;
        p.name = it->name;
        p.typeDef = paramTypeHandle->getTypeDef();
        p.isVariable = it->isVariable;
        p.placement = it->placement;
        parameterPack.listParameters.push_back(std::move(p));
    }

    for (auto it = actionImport->namedParametersBegin(); it != actionImport->namedParametersEnd(); it++) {
        lyric_assembler::TypeHandle *paramTypeHandle;
        TU_ASSIGN_OR_RETURN (paramTypeHandle, copy_type(
            it->type, importHash, targetNamespace, symbolReferenceSet, archiverState));

        lyric_assembler::Parameter p;
        p.index = it->index;
        p.name = it->name;
        p.typeDef = paramTypeHandle->getTypeDef();
        p.isVariable = it->isVariable;
        p.placement = it->placement;
        parameterPack.namedParameters.push_back(std::move(p));
    }

    if (actionImport->hasRestParameter()) {
        auto rest = actionImport->getRestParameter();

        lyric_assembler::TypeHandle *restTypeHandle;
        TU_ASSIGN_OR_RETURN (restTypeHandle, copy_type(
            rest.type, importHash, targetNamespace, symbolReferenceSet, archiverState));

        lyric_assembler::Parameter p;
        p.index = rest.index;
        p.name = rest.name;
        p.typeDef = restTypeHandle->getTypeDef();
        p.isVariable = rest.isVariable;
        p.placement = rest.placement;
        parameterPack.restParameter = Option(p);
    }

    lyric_assembler::TypeHandle *returnTypeHandle;
    TU_ASSIGN_OR_RETURN (returnTypeHandle, copy_type(
            actionImport->getReturnType(), importHash, targetNamespace, symbolReferenceSet, archiverState));

    TU_RETURN_IF_NOT_OK (actionSymbol->defineAction(parameterPack, returnTypeHandle->getTypeDef()));

    return {};
}