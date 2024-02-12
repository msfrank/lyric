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
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (conceptSymbol != nullptr);
    TU_ASSERT(walker.isValid());
    auto *state = moduleEntry.getState();
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
    auto parseAssignableResult = typeSystem->parseAssignable(conceptBlock, type);
    if (parseAssignableResult.isStatus())
        return parseAssignableResult.getStatus();
    auto returnSpec = parseAssignableResult.getResult();

    // compile the parameter list
    auto pack = walker.getChild(0);
    auto parsePackResult = typeSystem->parsePack(conceptBlock, pack);
    if (parsePackResult.isStatus())
        return parsePackResult.getStatus();
    auto packSpec = parsePackResult.getResult();

    // compile initializers
    absl::flat_hash_map<std::string,lyric_common::SymbolUrl> initializers;
    for (const auto &p : packSpec.parameterSpec) {
        if (!p.init.isEmpty()) {
            auto compileInitializerResult = lyric_compiler::internal::compile_default_initializer(conceptBlock,
                p.name, p.type, p.init.getValue(), moduleEntry);
            if (compileInitializerResult.isStatus())
                return compileInitializerResult.getStatus();
            initializers[p.name] = compileInitializerResult.getResult();
        }
    }

    lyric_object::AccessType accessType = absl::StartsWith(identifier, "_") ?
        lyric_object::AccessType::Private : lyric_object::AccessType::Public;

    // declare the action
    auto declareActionResult = conceptSymbol->declareAction(identifier, packSpec.parameterSpec,
        packSpec.restSpec, packSpec.ctxSpec, returnSpec, accessType);
    if (declareActionResult.isStatus())
        return declareActionResult.getStatus();
    auto actionUrl = declareActionResult.getResult();
    auto *sym = state->symbolCache()->getSymbol(actionUrl);
    if (sym == nullptr)
        conceptBlock->throwAssemblerInvariant("missing action symbol");
    if (sym->getSymbolType() != lyric_assembler::SymbolType::ACTION)
        conceptBlock->throwAssemblerInvariant("invalid action symbol");
    auto *actionSymbol = cast_symbol_to_action(sym);
    for (const auto &entry : initializers) {
        actionSymbol->putInitializer(entry.first, entry.second);
    }

    return lyric_compiler::CompilerStatus::ok();
}

tempo_utils::Status
lyric_compiler::internal::compile_defconcept(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT(walker.isValid());
    auto *state = moduleEntry.getState();

    // get concept name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // if concept is generic, then compile the template parameter list
    lyric_assembler::TemplateSpec templateSpec;
    if (walker.hasAttr(lyric_parser::kLyricAstGenericOffset)) {
        tu_uint32 genericOffset;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstGenericOffset, genericOffset);
        auto generic = walker.getNodeAtOffset(genericOffset);
        auto *typeSystem = moduleEntry.getTypeSystem();
        auto resolveTemplateResult = typeSystem->resolveTemplate(block, generic);
        if (resolveTemplateResult.isStatus())
            return resolveTemplateResult.getStatus();
        templateSpec = resolveTemplateResult.getResult();
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
    auto *superconceptSym = state->symbolCache()->getSymbol(fundamentalIdea);
    if (superconceptSym == nullptr)
        block->throwAssemblerInvariant("missing concept symbol");
    if (superconceptSym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        block->throwAssemblerInvariant("invalid concept symbol");
    auto *superConcept = cast_symbol_to_concept(superconceptSym);

    auto declConceptResult = block->declareConcept(
        identifier, superConcept, lyric_object::AccessType::Public,
        templateSpec.templateParameters);
    if (declConceptResult.isStatus())
        return declConceptResult.getStatus();
    auto conceptUrl = declConceptResult.getResult();

    auto *conceptSym = state->symbolCache()->getSymbol(conceptUrl);
    if (conceptSym == nullptr)
        block->throwAssemblerInvariant("missing concept symbol");
    if (conceptSym->getSymbolType() != lyric_assembler::SymbolType::CONCEPT)
        block->throwAssemblerInvariant("invalid concept symbol");
    auto *conceptSymbol = cast_symbol_to_concept(conceptSym);

    TU_LOG_INFO << "declared concept " << identifier << " with url " << conceptUrl;

    // compile actions
    for (const auto &def : defs) {
        auto status = compile_defconcept_def(conceptSymbol, def, moduleEntry);
        if (!status.isOk())
            return status;
    }

    return CompilerStatus::ok();
}
