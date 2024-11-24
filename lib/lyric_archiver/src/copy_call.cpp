
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/copy_call.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/namespace_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/template_handle.h>
#include <lyric_assembler/type_cache.h>
#include <lyric_importer/template_import.h>
#include "lyric_archiver/copy_template.h"

inline lyric_common::SymbolUrl
build_url(
    const lyric_common::ModuleLocation &location,
    const std::string &importHash,
    const lyric_common::SymbolUrl &symbolUrl)
{
    std::vector<std::string> parts{importHash};
    auto symbolPath = symbolUrl.getSymbolPath().getPath();
    parts.insert(parts.end(), symbolPath.cbegin(), symbolPath.cend());
    return lyric_common::SymbolUrl(location, lyric_common::SymbolPath(parts));
}

inline lyric_common::SymbolUrl
build_url(
    const lyric_common::ModuleLocation &location,
    const std::string &importHash,
    const lyric_assembler::AbstractSymbol *symbol)
{
    return build_url(location, importHash, symbol->getSymbolUrl());
}

tempo_utils::Result<lyric_common::SymbolUrl>
lyric_archiver::copy_call(
    lyric_importer::CallImport *callImport,
    const std::string &importHash,
    lyric_assembler::NamespaceSymbol *targetNamespace,
    ArchiverState &archiverState)
{
    auto *objectState = archiverState.objectState();
    auto *symbolCache = objectState->symbolCache();
    auto *namespaceBlock = targetNamespace->namespaceBlock();

    auto importUrl = callImport->getSymbolUrl();
    auto namespaceUrl = targetNamespace->getSymbolUrl();
    auto callUrl = build_url(namespaceUrl.getModuleLocation(), importHash, importUrl);

    // if call is already present in the symbol cache then we are done
    if (symbolCache->hasSymbol(callUrl))
        return callUrl;

    if (callImport->getReceiverUrl().isValid())
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "cannot archive {}; unexpected receiver for function", importUrl.toString());

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

    lyric_assembler::ParameterPack parameterPack;

    for (auto it = callImport->listParametersBegin(); it != callImport->listParametersEnd(); it++) {
        lyric_assembler::Parameter p;
        p.index = it->index;
        p.name = it->name;
        p.typeDef = it->type->getTypeDef();
        p.isVariable = it->isVariable;
        p.placement = it->placement;
        parameterPack.listParameters.push_back(std::move(p));
    }

    for (auto it = callImport->namedParametersBegin(); it != callImport->namedParametersEnd(); it++) {
        lyric_assembler::Parameter p;
        p.index = it->index;
        p.name = it->name;
        p.typeDef = it->type->getTypeDef();
        p.isVariable = it->isVariable;
        p.placement = it->placement;
        parameterPack.namedParameters.push_back(std::move(p));
    }

    if (callImport->hasRestParameter()) {
        auto rest = callImport->getRestParameter();
        lyric_assembler::Parameter p;
        p.index = rest.index;
        p.name = rest.name;
        p.typeDef = rest.type->getTypeDef();
        p.isVariable = rest.isVariable;
        p.placement = rest.placement;
        parameterPack.restParameter = Option(p);
    }

    lyric_assembler::ProcHandle *procHandle;
    TU_ASSIGN_OR_RETURN (procHandle, callSymbol->defineCall(parameterPack, callImport->getReturnType()->getTypeDef()));

    // append the call to the object
    TU_RETURN_IF_NOT_OK (objectState->appendCall(callSymbol.get()));
    auto *callSymbolPtr = callSymbol.release();

    auto moduleImport = callImport->getModuleImport();
    auto object = moduleImport->getObject();
    auto call = object.getObject().getCall(callImport->getCallOffset());
    auto header = call.getProcHeader();
    auto code = call.getBytecodeIterator();

    TU_RETURN_IF_NOT_OK (archiverState.putPendingProc(importUrl, object, header, code, procHandle));

    //
    for (auto it = callImport->initializersBegin(); it != callImport->initializersEnd(); it++) {
        const auto &name = it->first;
        auto initializerPath = it->second.getSymbolPath();
        auto initializer = object.getObject().findSymbol(initializerPath);
        if (!initializer.isValid() || initializer.getLinkageSection() != lyric_object::LinkageSection::Call)
            return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
                                                "");
        auto *initializerImport = moduleImport->getCall(initializer.getLinkageIndex());

        lyric_common::SymbolUrl initializerUrl;
        TU_ASSIGN_OR_RETURN (initializerUrl, copy_call(initializerImport, importHash, targetNamespace, archiverState));
        TU_RETURN_IF_NOT_OK (callSymbolPtr->putInitializer(name, initializerUrl));
    }

    return callUrl;
}
