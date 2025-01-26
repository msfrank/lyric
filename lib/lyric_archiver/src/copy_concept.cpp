
#include <lyric_archiver/archiver_result.h>
#include <lyric_archiver/archive_utils.h>
#include <lyric_archiver/copy_action.h>
#include <lyric_archiver/copy_call.h>
#include <lyric_archiver/copy_concept.h>
#include <lyric_archiver/copy_impl.h>
#include <lyric_archiver/copy_template.h>
#include <lyric_archiver/copy_type.h>
#include <lyric_assembler/action_symbol.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/type_cache.h>

tempo_utils::Result<lyric_assembler::ConceptSymbol *>
lyric_archiver::copy_concept(
    lyric_importer::ConceptImport *conceptImport,
    const std::string &importHash,
    lyric_assembler::NamespaceSymbol *targetNamespace,
    SymbolReferenceSet &symbolReferenceSet,
    ArchiverState &archiverState)
{
    auto *objectState = archiverState.objectState();
    auto *symbolCache = objectState->symbolCache();
    auto *typeCache = objectState->typeCache();
    auto *namespaceBlock = targetNamespace->namespaceBlock();

    auto importUrl = conceptImport->getSymbolUrl();
    auto namespaceUrl = targetNamespace->getSymbolUrl();
    auto conceptUrl = build_relative_url(importHash, importUrl);

    // if concept is already present in the symbol cache then we are done
    if (symbolCache->hasSymbol(conceptUrl)) {
        auto *sym = symbolCache->getSymbolOrNull(conceptUrl);
        if (sym->getSymbolType() == lyric_assembler::SymbolType::CONCEPT)
            return cast_symbol_to_concept(sym);
        return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
            "cannot archive {}; symbol is already defined", importUrl.toString());
    }

    auto access = conceptImport->getAccess();
    auto derive = conceptImport->getDerive();

    auto *conceptTemplateImport = conceptImport->getConceptTemplate();
    lyric_assembler::TemplateHandle *conceptTemplate = nullptr;
    if (conceptTemplateImport != nullptr) {
        TU_ASSIGN_OR_RETURN (conceptTemplate, copy_template(conceptTemplateImport, conceptUrl, objectState));
    }

    auto superConceptUrl = conceptImport->getSuperConcept();
    lyric_assembler::ConceptSymbol *superConcept = nullptr;
    if (superConceptUrl.isValid()) {
        lyric_assembler::AbstractSymbol *sym;
        TU_ASSIGN_OR_RETURN (sym, archiverState.getSymbol(superConceptUrl));
        if (sym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
            return ArchiverStatus::forCondition(ArchiverCondition::kArchiverInvariant,
                "cannot archive {}; missing superconcept", importUrl.toString());
        superConcept = cast_symbol_to_concept(sym);
    }

   // create the type
    lyric_assembler::TypeHandle *conceptType;
    if (conceptTemplate) {
        TU_ASSIGN_OR_RETURN (conceptType, typeCache->declareSubType(
            conceptUrl, conceptTemplate->getPlaceholders(), superConcept->getTypeDef()));
    } else {
        TU_ASSIGN_OR_RETURN (conceptType, typeCache->declareSubType(
            conceptUrl, {}, superConcept->getTypeDef()));
    }

    // declare the concept
    std::unique_ptr<lyric_assembler::ConceptSymbol> conceptSymbol;

    if (conceptTemplate != nullptr) {
        conceptSymbol = std::make_unique<lyric_assembler::ConceptSymbol>(
            conceptUrl, access, derive, conceptType, conceptTemplate, superConcept,
            /* isDeclOnly= */ false, namespaceBlock, objectState);
    } else {
        conceptSymbol = std::make_unique<lyric_assembler::ConceptSymbol>(
            conceptUrl, access, derive, conceptType, superConcept,
            /* isDeclOnly= */ false, namespaceBlock, objectState);
    }

    // append the concept to the object
    TU_RETURN_IF_NOT_OK (objectState->appendConcept(conceptSymbol.get()));
    auto *conceptSymbolPtr = conceptSymbol.release();

    // add the concept to the copied symbols map
    TU_RETURN_IF_NOT_OK (archiverState.putSymbol(importUrl, conceptSymbolPtr));

    // define the concept methods

    for (auto it = conceptImport->actionsBegin(); it != conceptImport->actionsEnd(); it++) {
        auto &name = it->first;
        lyric_importer::ActionImport *actionImport;
        TU_ASSIGN_OR_RETURN (actionImport, archiverState.importAction(it->second));
        std::vector<lyric_object::TemplateParameter> templateParameters;
        auto *templateImport = actionImport->getActionTemplate();
        if (templateImport != nullptr) {
            TU_ASSIGN_OR_RETURN (templateParameters, parse_template_parameters(templateImport));
        }
        lyric_assembler::ActionSymbol *actionSymbol;
        TU_ASSIGN_OR_RETURN (actionSymbol, conceptSymbolPtr->declareAction(name, actionImport->getAccess()));
        TU_RETURN_IF_NOT_OK (archiverState.putSymbol(it->second, actionSymbol));
        TU_RETURN_IF_NOT_OK (define_action(
            actionImport, actionSymbol, importHash, targetNamespace, symbolReferenceSet, archiverState));
    }

    // define the concept impls
    for (auto it = conceptImport->implsBegin(); it != conceptImport->implsEnd(); it++) {
        auto *implImport = it->second;
        lyric_assembler::TypeHandle *implType;
        TU_ASSIGN_OR_RETURN (implType, copy_type(
            implImport->getImplType(), importHash, targetNamespace, symbolReferenceSet, archiverState));
        lyric_assembler::ImplHandle *implHandle;
        TU_ASSIGN_OR_RETURN (implHandle, conceptSymbolPtr->declareImpl(implType->getTypeDef()));
        TU_RETURN_IF_NOT_OK (copy_impl(
            implImport, implHandle, importHash, targetNamespace, symbolReferenceSet, archiverState));
    }

    // put sealed subconcepts
    for (auto it = conceptImport->sealedTypesBegin(); it != conceptImport->sealedTypesEnd(); it++) {
        auto subconceptUrl = it->getConcreteUrl();
        TU_ASSIGN_OR_RETURN (subconceptUrl, archiverState.archiveSymbol(subconceptUrl, symbolReferenceSet));
        lyric_assembler::AbstractSymbol *sym;
        TU_ASSIGN_OR_RETURN (sym, archiverState.getSymbol(subconceptUrl));
        TU_RETURN_IF_NOT_OK (conceptSymbolPtr->putSealedType(sym->getTypeDef()));
    }

    return conceptSymbolPtr;
}
