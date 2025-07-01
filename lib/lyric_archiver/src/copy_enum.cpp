
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/archive_utils.h>
#include <lyric_archiver/copy_call.h>
#include <lyric_archiver/copy_enum.h>
#include <lyric_archiver/copy_impl.h>
#include <lyric_archiver/copy_template.h>
#include <lyric_archiver/copy_type.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Result<lyric_assembler::EnumSymbol *>
lyric_archiver::copy_enum(
    lyric_importer::EnumImport *enumImport,
    const std::string &importHash,
    lyric_assembler::NamespaceSymbol *targetNamespace,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    auto *objectState = archiverState.objectState();
    auto *symbolCache = objectState->symbolCache();
    auto *typeCache = objectState->typeCache();
    auto *namespaceBlock = targetNamespace->namespaceBlock();

    auto importUrl = enumImport->getSymbolUrl();
    auto namespaceUrl = targetNamespace->getSymbolUrl();
    auto enumUrl = build_relative_url(importHash, importUrl);

    // if enum is already present in the symbol cache then we are done
    if (symbolCache->hasSymbol(enumUrl)) {
        auto *sym = symbolCache->getSymbolOrNull(enumUrl);
        if (sym->getSymbolType() == lyric_assembler::SymbolType::ENUM)
            return cast_symbol_to_enum(sym);
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "cannot archive {}; symbol is already defined", importUrl.toString());
    }

    auto access = enumImport->getAccess();
    auto derive = enumImport->getDerive();
    auto isAbstract = enumImport->isAbstract();

    auto superEnumUrl = enumImport->getSuperEnum();
    lyric_assembler::EnumSymbol *superEnum = nullptr;
    if (superEnumUrl.isValid()) {
        lyric_assembler::AbstractSymbol *sym;
        TU_ASSIGN_OR_RETURN (sym, archiverState.getSymbol(superEnumUrl));
        if (sym->getSymbolType() != lyric_assembler::SymbolType::ENUM)
            return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
                "cannot archive {}; missing superenum", importUrl.toString());
        superEnum = cast_symbol_to_enum(sym);
    }

   // create the type
    lyric_assembler::TypeHandle *enumType;
    TU_ASSIGN_OR_RETURN (enumType, typeCache->declareSubType(enumUrl, {}, superEnum->getTypeDef()));

    // declare the enum
    std::unique_ptr<lyric_assembler::EnumSymbol> enumSymbol;
    enumSymbol = std::make_unique<lyric_assembler::EnumSymbol>(
            enumUrl, access, derive, isAbstract, enumType, superEnum,
            /* isDeclOnly= */ false, namespaceBlock, objectState);

    // append the enum to the object
    lyric_assembler::EnumSymbol *enumSymbolPtr;
    TU_ASSIGN_OR_RETURN (enumSymbolPtr, objectState->appendEnum(std::move(enumSymbol)));

    // add the enum to the copied symbols map
    TU_RETURN_IF_NOT_OK (archiverState.putSymbol(importUrl, enumSymbolPtr));

    // define the enum members

    for (auto it = enumImport->membersBegin(); it != enumImport->membersEnd(); it++) {
        auto &name = it->first;
        lyric_importer::FieldImport *fieldImport;
        TU_ASSIGN_OR_RETURN (fieldImport, archiverState.importField(it->second));
        lyric_assembler::TypeHandle *memberTypeHandle;
        TU_ASSIGN_OR_RETURN (memberTypeHandle, copy_type(
            fieldImport->getFieldType(), importHash, targetNamespace, symbolReferenceSet, archiverState));
        lyric_assembler::FieldSymbol *fieldSymbol;
        TU_ASSIGN_OR_RETURN (fieldSymbol, enumSymbolPtr->declareMember(
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

    // define the enum methods

    for (auto it = enumImport->methodsBegin(); it != enumImport->methodsEnd(); it++) {
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
        TU_ASSIGN_OR_RETURN (callSymbol, enumSymbolPtr->declareMethod(name, callImport->getAccess()));
        TU_RETURN_IF_NOT_OK (archiverState.putSymbol(it->second, callSymbol));
        TU_RETURN_IF_NOT_OK (define_call(
            callImport, callSymbol, importHash, targetNamespace, symbolReferenceSet, archiverState));
    }

    // define the enum impls
    for (auto it = enumImport->implsBegin(); it != enumImport->implsEnd(); it++) {
        auto *implImport = it->second;
        lyric_assembler::TypeHandle *implType;
        TU_ASSIGN_OR_RETURN (implType, copy_type(
            implImport->getImplType(), importHash, targetNamespace, symbolReferenceSet, archiverState));
        lyric_assembler::ImplHandle *implHandle;
        TU_ASSIGN_OR_RETURN (implHandle, enumSymbolPtr->declareImpl(implType->getTypeDef()));
        TU_RETURN_IF_NOT_OK (copy_impl(
            implImport, implHandle, importHash, targetNamespace, symbolReferenceSet, archiverState));
    }

    lyric_importer::CallImport *ctorImport;
    TU_ASSIGN_OR_RETURN (ctorImport, archiverState.importCall(ctorUrl));

    lyric_assembler::CallSymbol *ctorSymbol;
    TU_ASSIGN_OR_RETURN (ctorSymbol, enumSymbolPtr->declareCtor(ctorImport->getAccess(), enumImport->getAllocator()));
    TU_RETURN_IF_NOT_OK (archiverState.putSymbol(ctorImport->getSymbolUrl(), ctorSymbol));
    TU_RETURN_IF_NOT_OK (define_call(
        ctorImport, ctorSymbol, importHash, targetNamespace, symbolReferenceSet, archiverState));

    // put sealed subenums
    for (auto it = enumImport->sealedTypesBegin(); it != enumImport->sealedTypesEnd(); it++) {
        auto subenumUrl = it->getConcreteUrl();
        TU_ASSIGN_OR_RETURN (subenumUrl, archiverState.archiveSymbol(subenumUrl, symbolReferenceSet));
        lyric_assembler::AbstractSymbol *sym;
        TU_ASSIGN_OR_RETURN (sym, archiverState.getSymbol(subenumUrl));
        TU_RETURN_IF_NOT_OK (enumSymbolPtr->putSealedType(sym->getTypeDef()));
    }

    return enumSymbolPtr;
}