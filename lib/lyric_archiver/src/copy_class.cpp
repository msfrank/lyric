
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/archive_utils.h>
#include <lyric_archiver/copy_call.h>
#include <lyric_archiver/copy_class.h>
#include <lyric_archiver/copy_impl.h>
#include <lyric_archiver/copy_template.h>
#include <lyric_archiver/copy_type.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Result<lyric_assembler::ClassSymbol *>
lyric_archiver::copy_class(
    lyric_importer::ClassImport *classImport,
    const std::string &importHash,
    lyric_assembler::NamespaceSymbol *targetNamespace,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    auto *objectState = archiverState.objectState();
    auto *symbolCache = objectState->symbolCache();
    auto *typeCache = objectState->typeCache();
    auto *namespaceBlock = targetNamespace->namespaceBlock();

    auto importUrl = classImport->getSymbolUrl();
    auto namespaceUrl = targetNamespace->getSymbolUrl();
    auto classUrl = build_relative_url(importHash, importUrl);

    // if class is already present in the symbol cache then we are done
    if (symbolCache->hasSymbol(classUrl)) {
        auto *sym = symbolCache->getSymbolOrNull(classUrl);
        if (sym->getSymbolType() == lyric_assembler::SymbolType::CLASS)
            return cast_symbol_to_class(sym);
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "cannot archive {}; symbol is already defined", importUrl.toString());
    }

    auto access = classImport->getAccess();
    auto derive = classImport->getDerive();
    auto isAbstract = classImport->isAbstract();

    auto *classTemplateImport = classImport->getClassTemplate();
    lyric_assembler::TemplateHandle *classTemplate = nullptr;
    if (classTemplateImport != nullptr) {
        TU_ASSIGN_OR_RETURN (classTemplate, copy_template(classTemplateImport, classUrl, objectState));
    }

    auto superClassUrl = classImport->getSuperClass();
    lyric_assembler::ClassSymbol *superClass = nullptr;
    if (superClassUrl.isValid()) {
        lyric_assembler::AbstractSymbol *sym;
        TU_ASSIGN_OR_RETURN (sym, archiverState.getSymbol(superClassUrl));
        if (sym->getSymbolType() != lyric_assembler::SymbolType::CLASS)
            return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
                "cannot archive {}; missing superclass", importUrl.toString());
        superClass = cast_symbol_to_class(sym);
    }

   // create the type
    lyric_assembler::TypeHandle *classType;
    if (classTemplate) {
        TU_ASSIGN_OR_RETURN (classType, typeCache->declareSubType(
            classUrl, classTemplate->getPlaceholders(), superClass->getTypeDef()));
    } else {
        TU_ASSIGN_OR_RETURN (classType, typeCache->declareSubType(
            classUrl, {}, superClass->getTypeDef()));
    }

    // declare the class
    std::unique_ptr<lyric_assembler::ClassSymbol> classSymbol;

    if (classTemplate != nullptr) {
        classSymbol = std::make_unique<lyric_assembler::ClassSymbol>(
            classUrl, access, derive, isAbstract, classType, classTemplate, superClass,
            /* isDeclOnly= */ false, namespaceBlock, objectState);
    } else {
        classSymbol = std::make_unique<lyric_assembler::ClassSymbol>(
            classUrl, access, derive, isAbstract, classType, superClass,
            /* isDeclOnly= */ false, namespaceBlock, objectState);
    }

    // append the class to the object
    lyric_assembler::ClassSymbol *classSymbolPtr;
    TU_ASSIGN_OR_RETURN (classSymbolPtr, objectState->appendClass(std::move(classSymbol)));

    // add the class to the copied symbols map
    TU_RETURN_IF_NOT_OK (archiverState.putSymbol(importUrl, classSymbolPtr));

    // define the class members

    for (auto it = classImport->membersBegin(); it != classImport->membersEnd(); it++) {
        auto &name = it->first;
        lyric_importer::FieldImport *fieldImport;
        TU_ASSIGN_OR_RETURN (fieldImport, archiverState.importField(it->second));
        lyric_assembler::TypeHandle *memberTypeHandle;
        TU_ASSIGN_OR_RETURN (memberTypeHandle, copy_type(
            fieldImport->getFieldType(), importHash, targetNamespace, symbolReferenceSet, archiverState));
        lyric_assembler::FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RETURN (fieldSymbol, classSymbolPtr->declareMember(
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
            procHandle->putExitType(memberTypeHandle->getTypeDef());
            TU_RETURN_IF_STATUS (initializerHandle->finalizeInitializer());

            lyric_assembler::AbstractSymbol *initSymbol;
            TU_ASSIGN_OR_RETURN (initSymbol, archiverState.getSymbol(fieldSymbol->getInitializer()));
            TU_RETURN_IF_NOT_OK (archiverState.putSymbol(initializerUrl, initSymbol));
            TU_RETURN_IF_NOT_OK (put_pending_proc(initImport, procHandle, symbolReferenceSet, archiverState));
        }
    }

    lyric_common::SymbolUrl ctorUrl;

    // define the class methods

    for (auto it = classImport->methodsBegin(); it != classImport->methodsEnd(); it++) {
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
        TU_ASSIGN_OR_RETURN (callSymbol, classSymbolPtr->declareMethod(
            name, callImport->getAccess(), templateParameters));
        TU_RETURN_IF_NOT_OK (archiverState.putSymbol(it->second, callSymbol));
        TU_RETURN_IF_NOT_OK (define_call(
            callImport, callSymbol, importHash, targetNamespace, symbolReferenceSet, archiverState));
    }

    // define the class impls
    for (auto it = classImport->implsBegin(); it != classImport->implsEnd(); it++) {
        auto *implImport = it->second;
        lyric_assembler::TypeHandle *implType;
        TU_ASSIGN_OR_RETURN (implType, copy_type(
            implImport->getImplType(), importHash, targetNamespace, symbolReferenceSet, archiverState));
        lyric_assembler::ImplHandle *implHandle;
        TU_ASSIGN_OR_RETURN (implHandle, classSymbolPtr->declareImpl(implType->getTypeDef()));
        TU_RETURN_IF_NOT_OK (copy_impl(
            implImport, implHandle, importHash, targetNamespace, symbolReferenceSet, archiverState));
    }

    lyric_importer::CallImport *ctorImport;
    TU_ASSIGN_OR_RETURN (ctorImport, archiverState.importCall(ctorUrl));

    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, classSymbolPtr->declareCtor(ctorImport->getAccess(), classImport->getAllocator()));
    TU_RETURN_IF_NOT_OK (archiverState.putSymbol(ctorImport->getSymbolUrl(), ctorSymbol));
    TU_RETURN_IF_NOT_OK (define_call(
        ctorImport, ctorSymbol, importHash, targetNamespace, symbolReferenceSet, archiverState));

    // put sealed subclasses
    for (auto it = classImport->sealedTypesBegin(); it != classImport->sealedTypesEnd(); it++) {
        auto subclassUrl = it->getConcreteUrl();
        TU_ASSIGN_OR_RETURN (subclassUrl, archiverState.archiveSymbol(subclassUrl, symbolReferenceSet));
        lyric_assembler::AbstractSymbol *sym;
        TU_ASSIGN_OR_RETURN (sym, archiverState.getSymbol(subclassUrl));
        TU_RETURN_IF_NOT_OK (classSymbolPtr->putSealedType(sym->getTypeDef()));
    }

    return classSymbolPtr;
}