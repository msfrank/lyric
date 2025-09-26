
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/archive_utils.h>
#include <lyric_archiver/copy_call.h>
#include <lyric_archiver/copy_struct.h>
#include <lyric_archiver/copy_impl.h>
#include <lyric_archiver/copy_template.h>
#include <lyric_archiver/copy_type.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Result<lyric_assembler::StructSymbol *>
lyric_archiver::copy_struct(
    lyric_importer::StructImport *structImport,
    const std::string &importHash,
    lyric_assembler::NamespaceSymbol *targetNamespace,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    auto *objectState = archiverState.objectState();
    auto *symbolCache = objectState->symbolCache();
    auto *typeCache = objectState->typeCache();
    auto *namespaceBlock = targetNamespace->namespaceBlock();

    auto importUrl = structImport->getSymbolUrl();
    auto namespaceUrl = targetNamespace->getSymbolUrl();
    auto structUrl = build_relative_url(importHash, importUrl);

    // if struct is already present in the symbol cache then we are done
    if (symbolCache->hasSymbol(structUrl)) {
        auto *sym = symbolCache->getSymbolOrNull(structUrl);
        if (sym->getSymbolType() == lyric_assembler::SymbolType::STRUCT)
            return cast_symbol_to_struct(sym);
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "cannot archive {}; symbol is already defined", importUrl.toString());
    }

    auto isHidden = structImport->isHidden();
    auto derive = structImport->getDerive();

    auto superStructUrl = structImport->getSuperStruct();
    lyric_assembler::StructSymbol *superStruct = nullptr;
    if (superStructUrl.isValid()) {
        lyric_assembler::AbstractSymbol *sym;
        TU_ASSIGN_OR_RETURN (sym, archiverState.getSymbol(superStructUrl));
        if (sym->getSymbolType() != lyric_assembler::SymbolType::STRUCT)
            return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
                "cannot archive {}; missing superstruct", importUrl.toString());
        superStruct = cast_symbol_to_struct(sym);
    }

   // create the type
    lyric_assembler::TypeHandle *structType;
    TU_ASSIGN_OR_RETURN (structType, typeCache->declareSubType(structUrl, {}, superStruct->getTypeDef()));

    // declare the struct
    std::unique_ptr<lyric_assembler::StructSymbol> structSymbol;
    structSymbol = std::make_unique<lyric_assembler::StructSymbol>(
            structUrl, isHidden, derive, structType, superStruct,
            /* isDeclOnly= */ false, namespaceBlock, objectState);

    // append the struct to the object
    lyric_assembler::StructSymbol *structSymbolPtr;
    TU_ASSIGN_OR_RETURN (structSymbolPtr, objectState->appendStruct(std::move(structSymbol)));

    // add the struct to the copied symbols map
    TU_RETURN_IF_NOT_OK (archiverState.putSymbol(importUrl, structSymbolPtr));

    // define the struct members

    for (auto it = structImport->membersBegin(); it != structImport->membersEnd(); it++) {
        auto &name = it->first;
        lyric_importer::FieldImport *fieldImport;
        TU_ASSIGN_OR_RETURN (fieldImport, archiverState.importField(it->second));
        lyric_assembler::TypeHandle *memberTypeHandle;
        TU_ASSIGN_OR_RETURN (memberTypeHandle, copy_type(
            fieldImport->getFieldType(), importHash, targetNamespace, symbolReferenceSet, archiverState));
        lyric_assembler::FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RETURN (fieldSymbol, structSymbolPtr->declareMember(
            name, memberTypeHandle->getTypeDef(), fieldImport->isHidden()));
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
            procHandle->putExitType(memberTypeHandle->getTypeDef());
            TU_RETURN_IF_STATUS (initializerHandle->finalizeInitializer());

            lyric_assembler::AbstractSymbol *initSymbol;
            TU_ASSIGN_OR_RETURN (initSymbol, archiverState.getSymbol(fieldSymbol->getInitializer()));
            TU_RETURN_IF_NOT_OK (archiverState.putSymbol(initializerUrl, initSymbol));
            TU_RETURN_IF_NOT_OK (put_pending_proc(initImport, procHandle, symbolReferenceSet, archiverState));
        }
    }

    absl::flat_hash_map<std::string,lyric_importer::CallImport *> ctors;

    // define the struct methods

    for (auto it = structImport->methodsBegin(); it != structImport->methodsEnd(); it++) {
        auto &name = it->first;

        lyric_importer::CallImport *callImport;
        TU_ASSIGN_OR_RETURN (callImport, archiverState.importCall(it->second));
        if (callImport->getCallMode() == lyric_object::CallMode::Constructor) {
            ctors[name] = callImport;
            // delay constructor definition until the end
            continue;
        }

        std::vector<lyric_object::TemplateParameter> templateParameters;
        auto *templateImport = callImport->getCallTemplate();
        if (templateImport != nullptr) {
            TU_ASSIGN_OR_RETURN (templateParameters, parse_template_parameters(templateImport));
        }
        lyric_assembler::CallSymbol *callSymbol;
        TU_ASSIGN_OR_RETURN (callSymbol, structSymbolPtr->declareMethod(name, callImport->isHidden()));
        TU_RETURN_IF_NOT_OK (archiverState.putSymbol(it->second, callSymbol));
        TU_RETURN_IF_NOT_OK (define_call(
            callImport, callSymbol, importHash, targetNamespace, symbolReferenceSet, archiverState));
    }

    // define the struct impls

    for (auto it = structImport->implsBegin(); it != structImport->implsEnd(); it++) {
        auto *implImport = it->second;
        lyric_assembler::TypeHandle *implType;
        TU_ASSIGN_OR_RETURN (implType, copy_type(
            implImport->getImplType(), importHash, targetNamespace, symbolReferenceSet, archiverState));
        lyric_assembler::ImplHandle *implHandle;
        TU_ASSIGN_OR_RETURN (implHandle, structSymbolPtr->declareImpl(implType->getTypeDef()));
        TU_RETURN_IF_NOT_OK (copy_impl(
            implImport, implHandle, importHash, targetNamespace, symbolReferenceSet, archiverState));
    }

    // define struct ctors

    for (auto it = ctors.cbegin(); it != ctors.cend(); it++) {
        auto &name = it->first;
        auto *ctorImport = it->second;

        lyric_assembler::CallSymbol *ctorSymbol;
        TU_ASSIGN_OR_RETURN (ctorSymbol, structSymbolPtr->declareCtor(
            name, ctorImport->isHidden(), structImport->getAllocator()));
        TU_RETURN_IF_NOT_OK (archiverState.putSymbol(ctorImport->getSymbolUrl(), ctorSymbol));
        TU_RETURN_IF_NOT_OK (define_call(
            ctorImport, ctorSymbol, importHash, targetNamespace, symbolReferenceSet, archiverState));
    }

    // put sealed substructs

    for (auto it = structImport->sealedTypesBegin(); it != structImport->sealedTypesEnd(); it++) {
        auto substructUrl = it->getConcreteUrl();
        TU_ASSIGN_OR_RETURN (substructUrl, archiverState.archiveSymbol(substructUrl, symbolReferenceSet));
        lyric_assembler::AbstractSymbol *sym;
        TU_ASSIGN_OR_RETURN (sym, archiverState.getSymbol(substructUrl));
        TU_RETURN_IF_NOT_OK (structSymbolPtr->putSealedType(sym->getTypeDef()));
    }

    return structSymbolPtr;
}