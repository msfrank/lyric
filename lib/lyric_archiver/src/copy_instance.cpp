
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/archive_utils.h>
#include <lyric_archiver/copy_call.h>
#include <lyric_archiver/copy_instance.h>
#include <lyric_archiver/copy_impl.h>
#include <lyric_archiver/copy_template.h>
#include <lyric_archiver/copy_type.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Result<lyric_assembler::InstanceSymbol *>
lyric_archiver::copy_instance(
    lyric_importer::InstanceImport *instanceImport,
    const std::string &importHash,
    lyric_assembler::NamespaceSymbol *targetNamespace,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    auto *objectState = archiverState.objectState();
    auto *symbolCache = objectState->symbolCache();
    auto *typeCache = objectState->typeCache();
    auto *namespaceBlock = targetNamespace->namespaceBlock();

    auto importUrl = instanceImport->getSymbolUrl();
    auto namespaceUrl = targetNamespace->getSymbolUrl();
    auto instanceUrl = build_relative_url(importHash, importUrl);

    // if instance is already present in the symbol cache then we are done
    if (symbolCache->hasSymbol(instanceUrl)) {
        auto *sym = symbolCache->getSymbolOrNull(instanceUrl);
        if (sym->getSymbolType() == lyric_assembler::SymbolType::INSTANCE)
            return cast_symbol_to_instance(sym);
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "cannot archive {}; symbol is already defined", importUrl.toString());
    }

    auto access = instanceImport->getAccess();
    auto derive = instanceImport->getDerive();
    auto isAbstract = instanceImport->isAbstract();

    auto superInstanceUrl = instanceImport->getSuperInstance();
    lyric_assembler::InstanceSymbol *superInstance = nullptr;
    if (superInstanceUrl.isValid()) {
        lyric_assembler::AbstractSymbol *sym;
        TU_ASSIGN_OR_RETURN (sym, archiverState.getSymbol(superInstanceUrl));
        if (sym->getSymbolType() != lyric_assembler::SymbolType::INSTANCE)
            return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
                "cannot archive {}; missing superinstance", importUrl.toString());
        superInstance = cast_symbol_to_instance(sym);
    }

   // create the type
    lyric_assembler::TypeHandle *instanceType;
    TU_ASSIGN_OR_RETURN (instanceType, typeCache->declareSubType(instanceUrl, {}, superInstance->getTypeDef()));

    // declare the instance
    std::unique_ptr<lyric_assembler::InstanceSymbol> instanceSymbol;
    instanceSymbol = std::make_unique<lyric_assembler::InstanceSymbol>(
            instanceUrl, access, derive, isAbstract, instanceType, superInstance,
            /* isDeclOnly= */ false, namespaceBlock, objectState);

    // append the instance to the object
    lyric_assembler::InstanceSymbol *instanceSymbolPtr;
    TU_ASSIGN_OR_RETURN (instanceSymbolPtr, objectState->appendInstance(std::move(instanceSymbol)));

    // add the instance to the copied symbols map
    TU_RETURN_IF_NOT_OK (archiverState.putSymbol(importUrl, instanceSymbolPtr));

    // define the instance members

    for (auto it = instanceImport->membersBegin(); it != instanceImport->membersEnd(); it++) {
        auto &name = it->first;
        lyric_importer::FieldImport *fieldImport;
        TU_ASSIGN_OR_RETURN (fieldImport, archiverState.importField(it->second));
        lyric_assembler::TypeHandle *memberTypeHandle;
        TU_ASSIGN_OR_RETURN (memberTypeHandle, copy_type(
            fieldImport->getFieldType(), importHash, targetNamespace, symbolReferenceSet, archiverState));
        lyric_assembler::FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RETURN (fieldSymbol, instanceSymbolPtr->declareMember(
            name, memberTypeHandle->getTypeDef(), fieldImport->isVariable(), fieldImport->getAccess()));
        TU_RETURN_IF_NOT_OK (archiverState.putSymbol(fieldImport->getSymbolUrl(), fieldSymbol));
        auto initializerUrl = fieldImport->getInitializer();
        if (initializerUrl.isValid()) {
            lyric_importer::CallImport *initImport;
            TU_ASSIGN_OR_RETURN (initImport, archiverState.importCall(initializerUrl));
            std::vector<lyric_object::TemplateParameter> templateParameters;
            auto *templateImport = initImport->getCallTemplate();
            if (templateImport != nullptr) {
                TU_ASSIGN_OR_RETURN (templateParameters, parse_template_parameters(templateImport));
            }
            lyric_assembler::InitializerHandle *initializerHandle;
            TU_ASSIGN_OR_RETURN (initializerHandle, fieldSymbol->defineInitializer());
            auto *procHandle = initializerHandle->initializerProc();
            lyric_assembler::AbstractSymbol *initSymbol;
            TU_ASSIGN_OR_RETURN (initSymbol, archiverState.getSymbol(fieldSymbol->getInitializer()));
            TU_RETURN_IF_NOT_OK (archiverState.putSymbol(initializerUrl, initSymbol));
            TU_RETURN_IF_NOT_OK (put_pending_proc(initImport, procHandle, symbolReferenceSet, archiverState));
        }
    }

    lyric_common::SymbolUrl ctorUrl;

    // define the instance methods

    for (auto it = instanceImport->methodsBegin(); it != instanceImport->methodsEnd(); it++) {
        auto &name = it->first;
        // delay constructor definition until the end
        if (name == "$ctor") {
            ctorUrl = it->second;
            continue;
        }
        lyric_importer::CallImport *callImport;
        TU_ASSIGN_OR_RETURN (callImport, archiverState.importCall(it->second));
        std::vector<lyric_object::TemplateParameter> templateParameters;
        auto *templateImport = callImport->getCallTemplate();
        if (templateImport != nullptr) {
            TU_ASSIGN_OR_RETURN (templateParameters, parse_template_parameters(templateImport));
        }
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, instanceSymbolPtr->declareMethod(name, callImport->getAccess()));
        TU_RETURN_IF_NOT_OK (archiverState.putSymbol(it->second, callSymbol));
        TU_RETURN_IF_NOT_OK (define_call(
            callImport, callSymbol, importHash, targetNamespace, symbolReferenceSet, archiverState));
    }

    // define the instance impls
    for (auto it = instanceImport->implsBegin(); it != instanceImport->implsEnd(); it++) {
        auto *implImport = it->second;
        lyric_assembler::TypeHandle *implType;
        TU_ASSIGN_OR_RETURN (implType, copy_type(
            implImport->getImplType(), importHash, targetNamespace, symbolReferenceSet, archiverState));
        lyric_assembler::ImplHandle *implHandle;
        TU_ASSIGN_OR_RETURN (implHandle, instanceSymbolPtr->declareImpl(implType->getTypeDef()));
        TU_RETURN_IF_NOT_OK (copy_impl(
            implImport, implHandle, importHash, targetNamespace, symbolReferenceSet, archiverState));
    }

    lyric_importer::CallImport *ctorImport;
    TU_ASSIGN_OR_RETURN (ctorImport, archiverState.importCall(ctorUrl));

    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, instanceSymbolPtr->declareCtor(ctorImport->getAccess(), instanceImport->getAllocator()));
    TU_RETURN_IF_NOT_OK (archiverState.putSymbol(ctorImport->getSymbolUrl(), ctorSymbol));
    TU_RETURN_IF_NOT_OK (define_call(
        ctorImport, ctorSymbol, importHash, targetNamespace, symbolReferenceSet, archiverState));

    // put sealed subinstances
    for (auto it = instanceImport->sealedTypesBegin(); it != instanceImport->sealedTypesEnd(); it++) {
        auto subinstanceUrl = it->getConcreteUrl();
        TU_ASSIGN_OR_RETURN (subinstanceUrl, archiverState.archiveSymbol(subinstanceUrl, symbolReferenceSet));
        lyric_assembler::AbstractSymbol *sym;
        TU_ASSIGN_OR_RETURN (sym, archiverState.getSymbol(subinstanceUrl));
        TU_RETURN_IF_NOT_OK (instanceSymbolPtr->putSealedType(sym->getTypeDef()));
    }

    return instanceSymbolPtr;
}