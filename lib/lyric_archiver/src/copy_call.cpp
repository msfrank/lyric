
#include <lyric_archiver/archive_utils.h>
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/copy_call.h>
#include <lyric_archiver/copy_type.h>
#include <lyric_archiver/scan_proc.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/template_import.h>
#include "lyric_archiver/copy_template.h"


tempo_utils::Result<lyric_assembler::CallSymbol *>
lyric_archiver::copy_call(
    lyric_importer::CallImport *callImport,
    const std::string &importHash,
    lyric_assembler::NamespaceSymbol *targetNamespace,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    auto *objectState = archiverState.objectState();
    auto *symbolCache = objectState->symbolCache();
    auto *namespaceBlock = targetNamespace->namespaceBlock();

    auto importUrl = callImport->getSymbolUrl();
    auto namespaceUrl = targetNamespace->getSymbolUrl();
    auto callUrl = build_relative_url(importHash, importUrl);

    // if call is already present in the symbol cache then we are done
    if (symbolCache->hasSymbol(callUrl)) {
        auto *sym = symbolCache->getSymbolOrNull(callUrl);
        if (sym->getSymbolType() == lyric_assembler::SymbolType::CALL)
            return cast_symbol_to_call(sym);
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "cannot archive {}; symbol is already defined", importUrl.toString());
    }

    // if call has a receiver then we expect that it is already copied
    if (callImport->getReceiverUrl().isValid()) {
        auto receiverUrl = build_relative_url(importHash, callImport->getReceiverUrl());
        if (!symbolCache->hasSymbol(receiverUrl))
            return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
                "cannot archive {}; missing receiver", importUrl.toString());
    }

    //
    auto callMode = callImport->getCallMode();
    switch (callMode) {
        case lyric_object::CallMode::Normal:
        case lyric_object::CallMode::Inline:
            break;
        default:
            return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
                "cannot archive {}; unexpected call mode", importUrl.toString());
    }

    auto *templateImport = callImport->getCallTemplate();
    lyric_assembler::TemplateHandle *callTemplate = nullptr;
    if (templateImport != nullptr) {
        TU_ASSIGN_OR_RETURN (callTemplate, copy_template(templateImport, callUrl, objectState));
    }

    auto access = callImport->getAccess();

    // declare the function
    std::unique_ptr<lyric_assembler::CallSymbol> callSymbol;

    if (callTemplate != nullptr) {
        callSymbol = std::make_unique<lyric_assembler::CallSymbol>(
            callUrl, access, callMode, callTemplate, /* isDeclOnly= */ false, namespaceBlock, objectState);

    } else {
        callSymbol = std::make_unique<lyric_assembler::CallSymbol>(
            callUrl, access, callMode, /* isDeclOnly= */ false, namespaceBlock, objectState);
    }

    // define the call
    TU_RETURN_IF_NOT_OK (define_call(
        callImport, callSymbol.get(), importHash, targetNamespace, symbolReferenceSet, archiverState));

    // append the call to the object
    TU_RETURN_IF_NOT_OK (objectState->appendCall(callSymbol.get()));
    auto *callSymbolPtr = callSymbol.release();

    // add the call to the copied symbols map
    TU_RETURN_IF_NOT_OK (archiverState.putSymbol(importUrl, callSymbolPtr));

    return callSymbolPtr;
}

tempo_utils::Status
lyric_archiver::define_call(
    lyric_importer::CallImport *callImport,
    lyric_assembler::CallSymbol *callSymbol,
    const std::string &importHash,
    lyric_assembler::NamespaceSymbol *targetNamespace,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    auto importUrl = callImport->getSymbolUrl();

    lyric_assembler::ParameterPack parameterPack;

    for (auto it = callImport->listParametersBegin(); it != callImport->listParametersEnd(); it++) {
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

    for (auto it = callImport->namedParametersBegin(); it != callImport->namedParametersEnd(); it++) {
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

    if (callImport->hasRestParameter()) {
        auto rest = callImport->getRestParameter();

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
            callImport->getReturnType(), importHash, targetNamespace, symbolReferenceSet, archiverState));

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, returnTypeHandle->getTypeDef()));

    auto moduleImport = callImport->getModuleImport();
    auto object = moduleImport->getObject();

    //
    for (auto it = callImport->initializersBegin(); it != callImport->initializersEnd(); it++) {
        const auto &name = it->first;
        auto initializerPath = it->second.getSymbolPath();
        auto initializer = object.getObject().findSymbol(initializerPath);
        if (!initializer.isValid() || initializer.getLinkageSection() != lyric_object::LinkageSection::Call)
            return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
                "'{}' parameter has invalid initializer", name);
        auto *initializerImport = moduleImport->getCall(initializer.getLinkageIndex());

        lyric_assembler::CallSymbol *initializerSymbol;
        TU_ASSIGN_OR_RETURN (initializerSymbol, copy_call(
            initializerImport, importHash, targetNamespace, symbolReferenceSet, archiverState));
        TU_RETURN_IF_NOT_OK (callSymbol->putInitializer(name, initializerSymbol->getSymbolUrl()));
    }

    return put_pending_proc(callImport, procHandle, symbolReferenceSet, archiverState);
}

tempo_utils::Status
lyric_archiver::put_pending_proc(
    lyric_importer::CallImport *callImport,
    lyric_assembler::ProcHandle *procHandle,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    auto importUrl = callImport->getSymbolUrl();

    auto moduleImport = callImport->getModuleImport();
    auto location = moduleImport->getObjectLocation();
    auto object = moduleImport->getObject();
    auto plugin = moduleImport->getPlugin();
    auto call = object.getObject().getCall(callImport->getCallOffset());
    auto header = call.getProcHeader();
    auto code = call.getBytecodeIterator();

    TU_RETURN_IF_NOT_OK (scan_proc(location, object, code, symbolReferenceSet, archiverState));

    return archiverState.putPendingProc(importUrl, object, plugin, header, code, procHandle);
}