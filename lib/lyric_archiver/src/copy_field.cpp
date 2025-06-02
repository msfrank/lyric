
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/archive_utils.h>
#include <lyric_archiver/copy_call.h>
#include <lyric_archiver/copy_field.h>
#include <lyric_archiver/copy_type.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Result<lyric_assembler::FieldSymbol *>
lyric_archiver::copy_field(
    lyric_importer::FieldImport *fieldImport,
    const std::string &importHash,
    lyric_assembler::NamespaceSymbol *targetNamespace,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    auto *objectState = archiverState.objectState();
    auto *symbolCache = objectState->symbolCache();
    auto *namespaceBlock = targetNamespace->namespaceBlock();

    auto importUrl = fieldImport->getSymbolUrl();
    auto namespaceUrl = targetNamespace->getSymbolUrl();
    auto fieldUrl = build_relative_url(importHash, importUrl);

    // if field is already present in the symbol cache then we are done
    if (symbolCache->hasSymbol(fieldUrl)) {
        auto *sym = symbolCache->getSymbolOrNull(fieldUrl);
        if (sym->getSymbolType() == lyric_assembler::SymbolType::FIELD)
            return cast_symbol_to_field(sym);
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "cannot archive {}; symbol is already defined", importUrl.toString());
    }

    auto access = fieldImport->getAccess();
    auto isVariable = fieldImport->isVariable();

    lyric_assembler::TypeHandle *fieldType;
    TU_ASSIGN_OR_RETURN (fieldType, copy_type(
        fieldImport->getFieldType(), importHash, targetNamespace, symbolReferenceSet, archiverState));

    // declare the field
    auto fieldSymbol = std::make_unique<lyric_assembler::FieldSymbol>(
        fieldUrl, access, isVariable, fieldType, /* isDeclOnly= */ false, namespaceBlock, objectState);

    // append the field to the object
    lyric_assembler::FieldSymbol *fieldSymbolPtr;
    TU_ASSIGN_OR_RETURN (fieldSymbolPtr, objectState->appendField(std::move(fieldSymbol)));

    // add the field to the copied symbols map
    TU_RETURN_IF_NOT_OK (archiverState.putSymbol(fieldUrl, fieldSymbolPtr));

    // define the initializer if it exists
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

    return fieldSymbolPtr;
}
