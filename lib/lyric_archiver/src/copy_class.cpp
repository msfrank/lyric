
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/archive_utils.h>
#include <lyric_archiver/copy_call.h>
#include <lyric_archiver/copy_class.h>
#include <lyric_archiver/copy_impl.h>
#include <lyric_archiver/copy_template.h>
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
    auto *importCache = objectState->importCache();
    auto *symbolCache = objectState->symbolCache();
    auto *typeCache = objectState->typeCache();
    auto *namespaceBlock = targetNamespace->namespaceBlock();

    auto importUrl = classImport->getSymbolUrl();
    auto namespaceUrl = targetNamespace->getSymbolUrl();
    auto classUrl = build_relative_url(importHash, importUrl);

    // if call is already present in the symbol cache then we are done
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
        TU_ASSIGN_OR_RAISE (superClass, importCache->importClass(superClassUrl));
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
    TU_RETURN_IF_NOT_OK (objectState->appendClass(classSymbol.get()));
    auto *classSymbolPtr = classSymbol.release();

    // define the class members

    for (auto it = classImport->membersBegin(); it != classImport->membersEnd(); it++) {
        auto &name = it->first;
        lyric_importer::FieldImport *fieldImport;
        TU_ASSIGN_OR_RETURN (fieldImport, archiverState.importField(it->second));
        auto memberType = fieldImport->getFieldType()->getTypeDef();
        lyric_assembler::FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RETURN (fieldSymbol, classSymbolPtr->declareMember(
            name, memberType, fieldImport->isVariable(), fieldImport->getAccess()));
        auto initializerUrl = fieldImport->getInitializer();
        if (initializerUrl.isValid()) {
            lyric_importer::CallImport *initImport;
            TU_ASSIGN_OR_RETURN (initImport, archiverState.importCall(initializerUrl));
            std::vector<lyric_object::TemplateParameter> templateParameters;
            auto *templateImport = initImport->getCallTemplate();
            if (templateImport != nullptr) {
                TU_ASSIGN_OR_RETURN (templateParameters, parse_template_parameters(templateImport));
            }
            lyric_assembler::ProcHandle *procHandle;
            TU_ASSIGN_OR_RETURN (procHandle, fieldSymbol->defineInitializer());
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
        TU_RETURN_IF_NOT_OK (define_call(
            callImport, callSymbol, importHash, targetNamespace, symbolReferenceSet, archiverState));
    }

    // define the class impls
    for (auto it = classImport->implsBegin(); it != classImport->implsEnd(); it++) {
        auto implType = it->first;
        auto *implImport = it->second;
        lyric_assembler::ImplHandle *implHandle;
        TU_ASSIGN_OR_RETURN (implHandle, classSymbolPtr->declareImpl(implType));
        TU_RETURN_IF_NOT_OK (copy_impl(implImport, implHandle, symbolReferenceSet, archiverState));
    }

    // put sealed subclasses
    for (auto it = classImport->sealedTypesBegin(); it != classImport->sealedTypesEnd(); it++) {
        auto sealedType = *it;
        TU_RETURN_IF_NOT_OK (classSymbolPtr->putSealedType(sealedType));
    }

    lyric_importer::CallImport *ctorImport;
    TU_ASSIGN_OR_RETURN (ctorImport, archiverState.importCall(ctorUrl));

    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, classSymbolPtr->declareCtor(ctorImport->getAccess(), classImport->getAllocator()));
    TU_RETURN_IF_NOT_OK (define_call(
        ctorImport, ctorSymbol, importHash, targetNamespace, symbolReferenceSet, archiverState));

    return classSymbolPtr;
}