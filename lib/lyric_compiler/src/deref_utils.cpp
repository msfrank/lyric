
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/constant_utils.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/resolve_utils.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_typing/member_reifier.h>

#include "lyric_assembler/argument_variable.h"
#include "lyric_assembler/concept_symbol.h"

bool
lyric_compiler::current_ref_is_this_receiver(
    const lyric_assembler::SymbolCache *symbolCache,
    const lyric_assembler::BlockHandle *bindingBlock,
    const std::vector<DerefEffect> &effects)
{
    TU_ASSERT (!effects.empty());

    const auto &definitionUrl = bindingBlock->getDefinition();
    auto *symbol = symbolCache->getSymbolOrNull(definitionUrl);
    TU_ASSERT (symbol && symbol->getSymbolType() == lyric_assembler::SymbolType::CALL);
    auto *definitionCall = lyric_assembler::cast_symbol_to_call(symbol);
    auto thisUrl = definitionCall->getReceiverUrl();

    const auto &current = effects.back();
    const auto &receiverType = current.receiverType;
    if (receiverType.getType() != lyric_common::TypeDefType::Concrete)
        return false;
    auto receiverUrl = receiverType.getConcreteUrl();
    return receiverUrl == thisUrl;
}

tempo_utils::Status
lyric_compiler::deref_literal(
    const lyric_parser::ArchetypeNode *node,
    std::vector<DerefEffect> &effects,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto *fundamentalCache = driver->getFundamentalCache();
    lyric_common::SymbolUrl symbolUrl;

    auto astId = resource->getId();
    switch (astId) {
        // deref constant
        case lyric_schema::LyricAstId::Nil:
            TU_RETURN_IF_NOT_OK (constant_nil(block, fragment, driver));
            symbolUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Nil);
            break;
        case lyric_schema::LyricAstId::Undef:
            TU_RETURN_IF_NOT_OK (constant_undef(block, fragment, driver));
            symbolUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Undef);
            break;
        case lyric_schema::LyricAstId::True:
            TU_RETURN_IF_NOT_OK (constant_true(block, fragment, driver));
            symbolUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Bool);
            break;
        case lyric_schema::LyricAstId::False:
            TU_RETURN_IF_NOT_OK (constant_false(block, fragment, driver));
            symbolUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Bool);
            break;
        case lyric_schema::LyricAstId::Integer:
            TU_RETURN_IF_NOT_OK (constant_integer(node, block, fragment, driver));
            symbolUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Int);
            break;
        case lyric_schema::LyricAstId::Float:
            TU_RETURN_IF_NOT_OK (constant_float(node, block, fragment, driver));
            symbolUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Float);
            break;
        case lyric_schema::LyricAstId::Char:
            TU_RETURN_IF_NOT_OK (constant_char(node, block, fragment, driver));
            symbolUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Char);
            break;
        case lyric_schema::LyricAstId::String:
            TU_RETURN_IF_NOT_OK (constant_string(node, block, fragment, driver));
            symbolUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::String);
            break;
        case lyric_schema::LyricAstId::Raw:
            TU_RETURN_IF_NOT_OK (constant_raw(node, block, fragment, driver));
            symbolUrl = fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Bytes);
            break;
        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid deref target node");
    }

    auto *symbolCache = driver->getSymbolCache();

    // append the deref effect
    DerefEffect effect;
    effect.receiverType = driver->peekResult();
    effect.derefSymbol = symbolCache->getSymbolOrNull(symbolUrl);
    effect.pushResult = true;
    effect.sideEffecting = false;
    effects.push_back(std::move(effect));

    return {};
}

/**
 * dereference 'this' keyword. this overload is intended to be used to compile a bare 'this'
 * expression or a data dereference expression with a single element.
 */
tempo_utils::Status
lyric_compiler::deref_this(
    std::vector<DerefEffect> &effects,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_NOTNULL (block);
    TU_NOTNULL (fragment);
    TU_NOTNULL (driver);

    if (!effects.empty())
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "invalid this deref; effects vector is not empty");

    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, block->resolveReference("$this"));

    auto *state = block->blockState();
    auto *symbolCache = state->symbolCache();

    lyric_assembler::AbstractSymbol *thisSymbol;

    // ensure symbol for $this is imported
    auto thisUrl = ref.typeDef.getConcreteUrl();
    TU_ASSIGN_OR_RETURN (thisSymbol, symbolCache->getOrImportSymbol(thisUrl));

    // load the receiver
    TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));

    // push result type for this
    TU_RETURN_IF_NOT_OK (driver->pushResult(ref.typeDef));

    // append the deref effect
    DerefEffect effect;
    effect.receiverType = ref.typeDef;
    effect.derefSymbol = thisSymbol;
    effect.pushResult = true;
    effect.sideEffecting = false;
    effects.push_back(std::move(effect));

    return {};
}

inline tempo_utils::Result<lyric_assembler::DataReference>
resolve_binding(
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block)
{
    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
    return block->resolveReference(identifier);
}

inline tempo_utils::Status
deref_symbol(
    lyric_assembler::AbstractSymbol *symbol,
    lyric_common::TypeDef &resultType,
    bool &pushResult,
    lyric_assembler::CodeFragment *fragment)
{
    switch (symbol->getSymbolType()) {

        // never push result for the following symbol types
        case lyric_assembler::SymbolType::CLASS:
        case lyric_assembler::SymbolType::CONCEPT:
        case lyric_assembler::SymbolType::NAMESPACE:
        case lyric_assembler::SymbolType::STRUCT:
            pushResult = false;
            break;

        // set push result based on whether enum can be instantiated
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = lyric_assembler::cast_symbol_to_enum(symbol);
            pushResult = !enumSymbol->isAbstract();
            break;
        }

        // set push result based on whether instance can be instantiated
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = lyric_assembler::cast_symbol_to_instance(symbol);
            pushResult = !instanceSymbol->isAbstract();
            break;
        }

        // always push result for the following symbol types
        case lyric_assembler::SymbolType::PROTOCOL:
        case lyric_assembler::SymbolType::STATIC:
            pushResult = true;
            break;

        // any other symbol type is invalid to deref as a descriptor
        default:
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kCompilerInvariant,
                "invalid deref target {}", symbol->getSymbolUrl().toString());
    }

    resultType = symbol->getTypeDef();
    if (pushResult) {
        TU_RETURN_IF_NOT_OK (fragment->loadData(symbol));
    }

    return {};
}

tempo_utils::Status
lyric_compiler::deref_name(
    const lyric_parser::ArchetypeNode *node,
    std::vector<DerefEffect> &effects,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_NOTNULL (node);
    TU_NOTNULL (block);
    TU_NOTNULL (fragment);
    TU_NOTNULL (driver);

    if (!effects.empty())
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "invalid name deref; effects vector is not empty");

    // resolve the name to a data reference
    lyric_assembler::DataReference ref;
    TU_ASSIGN_OR_RETURN (ref, resolve_binding(node, block));

    auto *symbolCache = driver->getSymbolCache();
    auto *symbol = symbolCache->getSymbolOrNull(ref.symbolUrl);

    lyric_common::TypeDef resultType;
    bool pushResult;

    switch (ref.referenceType) {

        // if ref is a val or var then load it onto the stack
        case lyric_assembler::ReferenceType::Value:
        case lyric_assembler::ReferenceType::Variable:
            TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));
            resultType = ref.typeDef;
            pushResult = true;
            break;

        // if ref is a descriptor or namespace then we may or may not load it
        case lyric_assembler::ReferenceType::Descriptor:
        case lyric_assembler::ReferenceType::Namespace:
            TU_RETURN_IF_NOT_OK (deref_symbol(symbol, resultType, pushResult, fragment));
            break;

        default:
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "invalid deref target {}", ref.symbolUrl.toString());
    }

    // if ref is an argument, local, or lexical then we lookup the symbol for the type
    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::ARGUMENT:
        case lyric_assembler::SymbolType::LOCAL:
        case lyric_assembler::SymbolType::LEXICAL:
        case lyric_assembler::SymbolType::STATIC:
            if (resultType.getType() == lyric_common::TypeDefType::Concrete) {
                auto concreteUrl = resultType.getConcreteUrl();
                symbol = symbolCache->getSymbolOrNull(concreteUrl);
            } else {
                // if type is not concrete then we can't deref any further so set symbol to null
                symbol = nullptr;
            }
            break;
        default:
            break;
    }

    // push result type for name
    TU_RETURN_IF_NOT_OK (driver->pushResult(resultType));

    // append the deref effect
    DerefEffect effect;
    effect.receiverType = resultType;
    effect.derefSymbol = symbol;
    effect.pushResult = pushResult;
    effect.sideEffecting = false;
    effects.push_back(std::move(effect));

    return {};
}

tempo_utils::Status
lyric_compiler::deref_member(
    const lyric_parser::ArchetypeNode *node,
    std::vector<DerefEffect> &effects,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_NOTNULL (node);
    TU_NOTNULL (fragment);
    TU_NOTNULL (driver);

    if (effects.empty())
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "invalid member deref; effects vector is empty");

    auto *state = driver->getState();
    auto *typeSystem = driver->getTypeSystem();
    auto *symbolCache = state->symbolCache();

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    auto thisReceiver = current_ref_is_this_receiver(symbolCache, block, effects);

    const auto &effect = effects.back();
    auto receiverType = effect.receiverType;
    auto *derefSymbol = effect.derefSymbol;

    if (derefSymbol == nullptr)
        return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
            "cannot dereference {}", receiverType.toString());

    lyric_typing::MemberReifier reifier(typeSystem);
    lyric_assembler::DataReference ref;

    switch (derefSymbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = lyric_assembler::cast_symbol_to_class(derefSymbol);
            TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType, classSymbol->classTemplate()));
            TU_ASSIGN_OR_RETURN (ref, classSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            break;
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = lyric_assembler::cast_symbol_to_enum(derefSymbol);
            TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType));
            TU_ASSIGN_OR_RETURN (ref, enumSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            break;
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = lyric_assembler::cast_symbol_to_instance(derefSymbol);
            TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType));
            TU_ASSIGN_OR_RETURN (ref, instanceSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = lyric_assembler::cast_symbol_to_struct(derefSymbol);
            TU_RETURN_IF_NOT_OK (reifier.initialize(receiverType));
            TU_ASSIGN_OR_RETURN (ref, structSymbol->resolveMember(identifier, reifier, receiverType, thisReceiver));
            break;
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kInvalidSymbol,
                "invalid receiver symbol {}", derefSymbol->getSymbolUrl().toString());
    }

    // drop the previous result from the stack
    TU_RETURN_IF_NOT_OK (driver->popResult());

    auto *memberSymbol = symbolCache->getSymbolOrNull(ref.symbolUrl);
    switch (memberSymbol->getSymbolType()) {
        case lyric_assembler::SymbolType::FIELD:
            break;
        case lyric_assembler::SymbolType::STATIC:
            TU_RETURN_IF_NOT_OK (fragment->popValue());
            break;
        default:
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "invalid member symbol {}", memberSymbol->getSymbolUrl().toString());
    }

    // load the reference onto the stack
    TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));

    // push member result
    TU_RETURN_IF_NOT_OK (driver->pushResult(ref.typeDef));

    // append the deref effect
    DerefEffect nextEffect;
    nextEffect.receiverType = ref.typeDef;
    nextEffect.derefSymbol = memberSymbol;
    nextEffect.pushResult = true;
    nextEffect.sideEffecting = false;
    effects.push_back(std::move(nextEffect));

    return {};
}

tempo_utils::Status
lyric_compiler::deref_global_member(
    const lyric_parser::ArchetypeNode *node,
    std::vector<DerefEffect> &effects,
    lyric_assembler::BlockHandle *block,
    lyric_assembler::CodeFragment *fragment,
    CompilerScanDriver *driver)
{
    TU_NOTNULL (node);
    TU_NOTNULL (block);
    TU_NOTNULL (fragment);
    TU_NOTNULL (driver);

    if (effects.empty())
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "invalid member deref; effects vector is empty");
    TU_ASSERT (effects.back().pushResult == false);

    auto *state = driver->getState();
    auto *symbolCache = state->symbolCache();

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    auto thisReceiver = current_ref_is_this_receiver(symbolCache, block, effects);

    const auto &effect = effects.back();
    auto *derefSymbol = effect.derefSymbol;
    auto receiverType = effect.receiverType;

    lyric_assembler::DataReference ref;

    switch (derefSymbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = lyric_assembler::cast_symbol_to_class(derefSymbol);
            TU_ASSIGN_OR_RETURN (ref, classSymbol->resolveGlobalMember(identifier, receiverType, thisReceiver));
            break;
        }
        case lyric_assembler::SymbolType::CONCEPT: {
            auto *conceptSymbol = lyric_assembler::cast_symbol_to_concept(derefSymbol);
            TU_ASSIGN_OR_RETURN (ref, conceptSymbol->resolveGlobalMember(identifier, receiverType, thisReceiver));
            break;
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = lyric_assembler::cast_symbol_to_enum(derefSymbol);
            TU_ASSIGN_OR_RETURN (ref, enumSymbol->resolveGlobalMember(identifier, receiverType, thisReceiver));
            break;
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = lyric_assembler::cast_symbol_to_instance(derefSymbol);
            TU_ASSIGN_OR_RETURN (ref, instanceSymbol->resolveGlobalMember(identifier, receiverType, thisReceiver));
            break;
        }
        case lyric_assembler::SymbolType::NAMESPACE: {
            auto *namespaceSymbol = lyric_assembler::cast_symbol_to_namespace(derefSymbol);
            TU_ASSIGN_OR_RETURN (ref, namespaceSymbol->resolveTargetMember(identifier, block));
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = lyric_assembler::cast_symbol_to_struct(derefSymbol);
            TU_ASSIGN_OR_RETURN (ref, structSymbol->resolveGlobalMember(identifier, receiverType, thisReceiver));
            break;
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kInvalidSymbol,
                "invalid receiver symbol {}", derefSymbol->getSymbolUrl().toString());
    }

    bool pushResult = false;

    auto *memberSymbol = symbolCache->getSymbolOrNull(ref.symbolUrl);
    switch (memberSymbol->getSymbolType()) {
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = lyric_assembler::cast_symbol_to_enum(memberSymbol);
            pushResult = !enumSymbol->isAbstract();
            break;
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = lyric_assembler::cast_symbol_to_instance(derefSymbol);
            pushResult = !instanceSymbol->isAbstract();
            break;
        }
        case lyric_assembler::SymbolType::NAMESPACE:
            pushResult = false;
            break;
        case lyric_assembler::SymbolType::STATIC:
            pushResult = true;
            break;
        default:
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "invalid member symbol {}", memberSymbol->getSymbolUrl().toString());
    }

    // load the reference onto the stack and push result if necessary
    if (pushResult) {
        TU_RETURN_IF_NOT_OK (fragment->loadRef(ref));
        TU_RETURN_IF_NOT_OK (driver->pushResult(ref.typeDef));
    }

    // append the deref effect
    DerefEffect nextEffect;
    nextEffect.receiverType = ref.typeDef;
    nextEffect.derefSymbol = memberSymbol;
    nextEffect.pushResult = pushResult;
    nextEffect.sideEffecting = false;
    effects.push_back(std::move(nextEffect));

    return {};
}