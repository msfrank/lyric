
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/import_cache.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_call.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/callsite_reifier.h>

tempo_utils::Status
lyric_compiler::internal::compile_placement(
    lyric_assembler::BlockHandle *bindingBlock,
    lyric_assembler::BlockHandle *invokeBlock,
    const lyric_assembler::AbstractInvoker &invoker,
    lyric_typing::CallsiteReifier &reifier,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (bindingBlock != nullptr);
    TU_ASSERT (invokeBlock != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();
    auto *symbolCache = state->symbolCache();

    auto placementBegin = invoker.placementBegin();
    auto placementEnd = invoker.placementEnd();

    int offset = reifier.numArguments();    // number of preloaded arguments

    int numpos = 0;                         // count of positional arguments
    for (int i = 0; i < offset; i++) {
        const auto &param = *placementBegin++;
        if (param.placement == lyric_object::PlacementType::List)
            numpos++;
    }

    std::vector<lyric_parser::NodeWalker> positionalArgs;
    absl::flat_hash_map<std::string,lyric_parser::NodeWalker> keywordArgs;
    absl::flat_hash_set<lyric_common::SymbolUrl> usedEvidence;

    // separate call arguments into positional and keyword
    for (int i = 0; i < walker.numChildren(); i++) {
        auto arg = walker.getChild(i);
        if (arg.isClass(lyric_schema::kLyricAstKeywordClass)) {
            std::string label;
            moduleEntry.parseAttrOrThrow(arg, lyric_parser::kLyricAstIdentifier, label);
            if (arg.numChildren() != 1)
                bindingBlock->throwSyntaxError(arg, "invalid keyword");
            keywordArgs[label] = arg.getChild(0);
        } else {
            positionalArgs.push_back(arg);
        }
    }

    // push argument values onto the stack, record their types
    for (auto param = placementBegin; param != placementEnd; param++) {

        switch (param->placement) {

            // evaluate the positional parameter
            case lyric_object::PlacementType::List: {
                if (std::cmp_less_equal(positionalArgs.size(), numpos - offset))
                    return state->logAndContinue(CompilerCondition::kUnexpectedArgument,
                        tempo_tracing::LogSeverity::kError,
                        "unexpected positional argument");
                const auto &arg = positionalArgs[numpos - offset];
                if (!arg.isValid())
                    bindingBlock->throwAssemblerInvariant(arg, "invalid positional argument");
                auto result = compile_expression(invokeBlock, arg, moduleEntry);
                if (result.isStatus())
                    return result.getStatus();
                auto status = reifier.reifyNextArgument(result.getResult());
                if (!status.isOk())
                    return status;
                numpos++;
                break;
            }

            // evaluate the named parameter
            case lyric_object::PlacementType::Named: {
                if (!keywordArgs.contains(param->name))
                    return state->logAndContinue(CompilerCondition::kMissingArgument,
                        tempo_tracing::LogSeverity::kError,
                        "missing keyword argument {}", param->name);
                const auto &arg = keywordArgs.at(param->name);
                auto result = compile_expression(invokeBlock, arg, moduleEntry);
                if (result.isStatus())
                    return result.getStatus();
                auto status = reifier.reifyNextArgument(result.getResult());
                if (!status.isOk())
                    return status;
                break;
            }

            // evaluate the optional parameter
            case lyric_object::PlacementType::Opt: {
                if (!keywordArgs.contains(param->name)) {
                    if (!invoker.hasInitializer(param->name))
                        bindingBlock->throwAssemblerInvariant("invalid parameter");
                    auto initializerUrl = invoker.getInitializer(param->name);

                    lyric_assembler::AbstractSymbol *symbol;
                    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(initializerUrl));
                    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
                        bindingBlock->throwAssemblerInvariant("invalid initializer {}", initializerUrl.toString());
                    lyric_assembler::CallInvoker call(cast_symbol_to_call(symbol));

                    lyric_typing::CallsiteReifier initReifier(call.getParameters(), call.getRest(),
                        call.getTemplateUrl(), call.getTemplateParameters(), reifier.getCallsiteArguments(),
                        typeSystem);
                    TU_RETURN_IF_NOT_OK (initReifier.initialize());
                    auto invokeInitializerResult = call.invoke(invokeBlock, initReifier);
                    if (invokeInitializerResult.isStatus())
                        return invokeInitializerResult.getStatus();
                    auto status = reifier.reifyNextArgument(invokeInitializerResult.getResult());
                    if (!status.isOk())
                        return status;
                } else {
                    const auto &arg = keywordArgs.at(param->name);
                    auto result = compile_expression(invokeBlock, arg, moduleEntry);
                    if (result.isStatus())
                        return result.getStatus();
                    auto status = reifier.reifyNextArgument(result.getResult());
                    if (!status.isOk())
                        return status;
                }
                break;
            }

            // evaluate the ctx parameter
            case lyric_object::PlacementType::Ctx: {

                auto reifyContextResult = reifier.reifyNextContext();
                if (reifyContextResult.isStatus())
                    return reifyContextResult.getStatus();
                auto contextType = reifyContextResult.getResult();

                // if ctx argument has been specified explicitly
                if (keywordArgs.contains(param->name)) {
                    const auto &arg = keywordArgs[param->name];
                    if (arg.isValid()) {
                        auto result = compile_expression(invokeBlock, arg, moduleEntry);
                        if (result.isStatus())
                            return result.getStatus();
                        auto ctxUrl = result.getResult().getConcreteUrl();
                        if (!symbolCache->hasSymbol(ctxUrl))
                            bindingBlock->throwAssemblerInvariant("missing ctx symbol {}", ctxUrl.toString());
                        lyric_assembler::AbstractSymbol *ctxSym;
                        TU_ASSIGN_OR_RETURN (ctxSym, symbolCache->getOrImportSymbol(ctxUrl));
                        if (ctxSym->getSymbolType() != lyric_assembler::SymbolType::INSTANCE)
                            bindingBlock->throwAssemblerInvariant("invalid ctx symbol {}", ctxUrl.toString());
                        auto *ctxInstance = cast_symbol_to_instance(ctxSym);
                        if (!ctxInstance->hasImpl(contextType))
                            return CompilerStatus::forCondition(CompilerCondition::kMissingImpl);
                    } else {
                        bindingBlock->throwAssemblerInvariant("invalid ctx argument {}", param->name);
                    }
                }
                // otherwise use implicit resolution to find ctx
                else {
                    auto resolveInstanceResult = bindingBlock->resolveImpl(contextType);
                    if (resolveInstanceResult.isStatus())
                        return resolveInstanceResult.getStatus();
                    auto evUrl = resolveInstanceResult.getResult();
                    lyric_assembler::AbstractSymbol *evSym;
                    TU_ASSIGN_OR_RETURN (evSym, symbolCache->getOrImportSymbol(evUrl));
                    if (evSym->getSymbolType() != lyric_assembler::SymbolType::INSTANCE)
                        bindingBlock->throwAssemblerInvariant("invalid ctx symbol {}", evUrl.toString());
                    auto *instance = cast_symbol_to_instance(evSym);
                    instance->touch();
                    auto *code = invokeBlock->blockCode();
                    auto status = code->loadInstance(instance->getAddress());
                    if (!status.isOk())
                            return status;
                    usedEvidence.insert(evUrl);
                }
                break;
            }

            default:
                bindingBlock->throwAssemblerInvariant("invalid call parameter");
        }
    }

    // after the placement, evaluate any remaining arguments
    for (; std::cmp_less(numpos - offset, positionalArgs.size()); numpos++) {
        auto result = compile_expression(invokeBlock, positionalArgs[numpos - offset], moduleEntry);
        if (result.isStatus())
            return result.getStatus();
        auto status = reifier.reifyNextArgument(result.getResult());
        if (!status.isOk())
            return status;
    }

    return CompilerStatus::ok();
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_function_call(
    lyric_assembler::BlockHandle *bindingBlock,
    lyric_assembler::BlockHandle *invokeBlock,
    const std::string &functionName,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();

    auto resolveFunctionResult = bindingBlock->resolveFunction(functionName);
    if (resolveFunctionResult.isStatus())
        return resolveFunctionResult.getStatus();
    auto function = resolveFunctionResult.getResult();

    // FIXME: support callsite type arguments on function
    lyric_typing::CallsiteReifier reifier(function.getParameters(), function.getRest(),
        function.getTemplateUrl(), function.getTemplateParameters(), {},
        typeSystem);

    TU_RETURN_IF_NOT_OK (reifier.initialize());

    auto status = compile_placement(bindingBlock, invokeBlock, function, reifier, walker, moduleEntry);
    if (!status.isOk())
        return status;

    auto invokeCallResult = function.invoke(invokeBlock, reifier);
    if (invokeCallResult.isStatus())
        return invokeCallResult.getStatus();
    return invokeCallResult.getResult();
}
