#include <absl/strings/match.h>

#include <lyric_assembler/block_handle.h>
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/proc_handle.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_assembler/synthetic_symbol.h>
#include <lyric_compiler/internal/compile_call.h>
#include <lyric_compiler/internal/compile_defconcept.h>
#include <lyric_compiler/internal/compile_initializer.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <tempo_utils/log_stream.h>

static tempo_utils::Status
compile_defconcept_def(
    lyric_assembler::ConceptSymbol *conceptSymbol,
    const std::vector<lyric_object::TemplateParameter> &templateParameters,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (conceptSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDefClass, 1);

    auto *conceptBlock = conceptSymbol->conceptBlock();

    // get action name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // get action return type
    tu_uint32 typeOffset;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeOffset, typeOffset);
    auto type = walker.getNodeAtOffset(typeOffset);
    lyric_typing::TypeSpec returnSpec;
    TU_ASSIGN_OR_RETURN (returnSpec, typeSystem->parseAssignable(conceptBlock, type));

    // parse the parameter list
    auto pack = walker.getChild(0);
    lyric_typing::PackSpec packSpec;
    TU_ASSIGN_OR_RETURN (packSpec, typeSystem->parsePack(conceptBlock, pack));

    lyric_object::AccessType accessType = absl::StartsWith(identifier, "_") ?
        lyric_object::AccessType::Private : lyric_object::AccessType::Public;

    // declare the concept action
    lyric_assembler::ActionSymbol *actionSymbol;
    TU_ASSIGN_OR_RETURN (actionSymbol, conceptSymbol->declareAction(identifier, accessType));

    auto *resolver = actionSymbol->actionResolver();

    // resolve the parameter pack
    lyric_assembler::ParameterPack parameterPack;
    TU_ASSIGN_OR_RETURN (parameterPack, typeSystem->resolvePack(resolver, packSpec));

    // resolve the return type
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, typeSystem->resolveAssignable(resolver, returnSpec));

    // compile list parameter initializers
    for (const auto &p : parameterPack.listParameters) {
        auto entry = packSpec.initializers.find(p.name);
        if (entry != packSpec.initializers.cend()) {
            lyric_common::SymbolUrl initializerUrl;
            TU_ASSIGN_OR_RETURN (initializerUrl, lyric_compiler::internal::compile_param_initializer(
                conceptBlock, entry->second, p, templateParameters, moduleEntry));
            actionSymbol->putInitializer(p.name, initializerUrl);
        }
    }

    // compile named parameter initializers
    for (const auto &p : parameterPack.namedParameters) {
        auto entry = packSpec.initializers.find(p.name);
        if (entry != packSpec.initializers.cend()) {
            lyric_common::SymbolUrl initializerUrl;
            TU_ASSIGN_OR_RETURN (initializerUrl, lyric_compiler::internal::compile_param_initializer(
                conceptBlock, entry->second, p, templateParameters, moduleEntry));
            actionSymbol->putInitializer(p.name, initializerUrl);
        }
    }

    TU_RETURN_IF_NOT_OK (actionSymbol->defineAction(parameterPack, returnType));

    return {};
}

tempo_utils::Status
lyric_compiler::internal::compile_defconcept(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry,
    lyric_assembler::ConceptSymbol **conceptSymbolPtr)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *state = moduleEntry.getState();

    // get concept name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // if concept is generic, then compile the template parameter list
    lyric_typing::TemplateSpec templateSpec;
    if (walker.hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        tu_uint32 genericOffset;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstGenericOffset, genericOffset);
        auto generic = walker.getNodeAtOffset(genericOffset);
        auto *typeSystem = moduleEntry.getTypeSystem();
        TU_ASSIGN_OR_RETURN (templateSpec, typeSystem->parseTemplate(block, generic));
    }

    std::vector<lyric_parser::NodeWalker> defs;

    // make initial pass over concept body
    for (int i = 0; i < walker.numChildren(); i++) {
        auto child = walker.getChild(i);
        lyric_schema::LyricAstId childId{};
        moduleEntry.parseIdOrThrow(child, lyric_schema::kLyricAstVocabulary, childId);
        switch (childId) {
            case lyric_schema::LyricAstId::Def:
                defs.emplace_back(child);
                break;
            default:
                block->throwSyntaxError(child, "expected concept body");
        }
    }

    //
    auto fundamentalIdea = state->fundamentalCache()->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Idea);
    lyric_assembler::AbstractSymbol *superconceptSym;
    TU_ASSIGN_OR_RETURN (superconceptSym, state->symbolCache()->getOrImportSymbol(fundamentalIdea));
    if (superconceptSym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        block->throwAssemblerInvariant("invalid concept symbol");
    auto *superConcept = cast_symbol_to_concept(superconceptSym);

    auto declConceptResult = block->declareConcept(
        identifier, superConcept, lyric_object::AccessType::Public,
        templateSpec.templateParameters);
    if (declConceptResult.isStatus())
        return declConceptResult.getStatus();
    auto conceptUrl = declConceptResult.getResult();

    lyric_assembler::AbstractSymbol *conceptSym;
    TU_ASSIGN_OR_RETURN (conceptSym, state->symbolCache()->getOrImportSymbol(conceptUrl));
    if (conceptSym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        block->throwAssemblerInvariant("invalid concept symbol");
    auto *conceptSymbol = cast_symbol_to_concept(conceptSym);

    TU_LOG_INFO << "declared concept " << identifier << " with url " << conceptUrl;

    // compile actions
    for (const auto &def : defs) {
        TU_RETURN_IF_NOT_OK (compile_defconcept_def(
            conceptSymbol, templateSpec.templateParameters, def, moduleEntry));
    }

    if (conceptSymbolPtr != nullptr) {
        *conceptSymbolPtr = conceptSymbol;
    }

    return CompilerStatus::ok();
}
