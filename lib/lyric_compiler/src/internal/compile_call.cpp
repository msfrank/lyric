
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
    const lyric_assembler::AbstractPlacement *placement,
    lyric_assembler::BlockHandle *bindingBlock,
    lyric_assembler::BlockHandle *invokeBlock,
    lyric_typing::CallsiteReifier &reifier,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (placement != nullptr);
    TU_ASSERT (bindingBlock != nullptr);
    TU_ASSERT (invokeBlock != nullptr);
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();
    auto *symbolCache = state->symbolCache();

//    auto listPlacementBegin = invoker.listPlacementBegin();
//    auto placementEnd = invoker.placementEnd();
//
//    // offset is the number of preloaded arguments
//    int offset = reifier.numArguments();
//
//    // count the remaining positional arguments, including default initialized
//    int numpos = 0;
//    for (int i = 0; i < offset; i++) {
//        const auto &param = *placementBegin++;
//        switch (param.placement) {
//            case lyric_object::PlacementType::List:
//            case lyric_object::PlacementType::ListOpt:
//                numpos++;
//                break;
//            default:
//                break;
//        }
//    }

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

    size_t currpos = 0;
    ssize_t firstListOptArgument = -1;

    // push list arguments onto the stack, and record their types
    for (auto param = placement->listPlacementBegin(); param != placement->listPlacementEnd(); param++) {

        switch (param->placement) {

            // evaluate the list parameter
            case lyric_object::PlacementType::List: {
                if (positionalArgs.size() <= currpos)
                    return state->logAndContinue(CompilerCondition::kUnexpectedArgument,
                        tempo_tracing::LogSeverity::kError,
                        "unexpected list argument {}", currpos);
                if (firstListOptArgument >= 0)
                    return state->logAndContinue(CompilerCondition::kUnexpectedArgument,
                        tempo_tracing::LogSeverity::kError,
                        "unexpected list argument {}; missing default initializer", currpos);
                const auto &arg = positionalArgs[currpos];
                if (!arg.isValid())
                    bindingBlock->throwAssemblerInvariant(arg, "invalid positional parameter {}", currpos);
                lyric_common::TypeDef resultType;
                TU_ASSIGN_OR_RETURN (resultType, compile_expression(invokeBlock, arg, moduleEntry));
                TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(resultType));
                currpos++;
                break;
            }

                // evaluate the list optional parameter
            case lyric_object::PlacementType::ListOpt: {
                if (positionalArgs.size() <= currpos) {
                    if (!placement->hasInitializer(param->name))
                        bindingBlock->throwAssemblerInvariant("invalid list parameter {}", param->name);
                    auto initializerUrl = placement->getInitializer(param->name);

                    lyric_assembler::AbstractSymbol *symbol;
                    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(initializerUrl));
                    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
                        bindingBlock->throwAssemblerInvariant("invalid initializer {}", initializerUrl.toString());
                    auto *initSymbol = cast_symbol_to_call(symbol);

                    auto callable = std::make_unique<lyric_assembler::FunctionCallable>(initSymbol);
                    lyric_assembler::CallableInvoker invoker;
                    TU_RETURN_IF_NOT_OK (invoker.initialize(std::move(callable)));

                    lyric_typing::CallsiteReifier initReifier(typeSystem);
                    TU_RETURN_IF_NOT_OK (initReifier.initialize(invoker, reifier.getCallsiteArguments()));

                    lyric_common::TypeDef initializerType;
                    TU_ASSIGN_OR_RETURN (initializerType, invoker.invoke(invokeBlock, initReifier));
                    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(initializerType));
                } else {
                    const auto &arg = positionalArgs[currpos];
                    if (!arg.isValid())
                        bindingBlock->throwAssemblerInvariant(arg, "invalid positional parameter {}", currpos);
                    lyric_common::TypeDef resultType;
                    TU_ASSIGN_OR_RETURN (resultType, compile_expression(invokeBlock, arg, moduleEntry));
                    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(resultType));
                }
                if (firstListOptArgument < 0) {
                    firstListOptArgument = currpos;
                }
                currpos++;
                break;
            }

            default:
                bindingBlock->throwAssemblerInvariant("invalid call parameter");
        }
    }

    // push argument values onto the stack, record their types
    for (auto param = placement->namedPlacementBegin(); param != placement->namedPlacementEnd(); param++) {

        switch (param->placement) {

            // evaluate the named parameter
            case lyric_object::PlacementType::Named: {
                if (!keywordArgs.contains(param->name))
                    return state->logAndContinue(CompilerCondition::kMissingArgument,
                        tempo_tracing::LogSeverity::kError,
                        "missing keyword argument {}", param->name);
                const auto &arg = keywordArgs.at(param->name);
                lyric_common::TypeDef resultType;
                TU_ASSIGN_OR_RETURN (resultType, compile_expression(invokeBlock, arg, moduleEntry));
                TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(resultType));
                break;
            }

            // evaluate the named optional parameter
            case lyric_object::PlacementType::NamedOpt: {
                if (!keywordArgs.contains(param->name)) {
                    if (!placement->hasInitializer(param->name))
                        bindingBlock->throwAssemblerInvariant("invalid named parameter {}", param->name);
                    auto initializerUrl = placement->getInitializer(param->name);

                    lyric_assembler::AbstractSymbol *symbol;
                    TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(initializerUrl));
                    if (symbol->getSymbolType() != lyric_assembler::SymbolType::CALL)
                        bindingBlock->throwAssemblerInvariant("invalid initializer {}", initializerUrl.toString());
                    auto *initSymbol = cast_symbol_to_call(symbol);

                    auto callable = std::make_unique<lyric_assembler::FunctionCallable>(initSymbol);
                    lyric_assembler::CallableInvoker invoker;
                    TU_RETURN_IF_NOT_OK (invoker.initialize(std::move(callable)));

                    lyric_typing::CallsiteReifier initReifier(typeSystem);
                    TU_RETURN_IF_NOT_OK (initReifier.initialize(invoker, reifier.getCallsiteArguments()));

                    lyric_common::TypeDef initializerType;
                    TU_ASSIGN_OR_RETURN (initializerType, invoker.invoke(invokeBlock, initReifier));
                    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(initializerType));
                } else {
                    const auto &arg = keywordArgs.at(param->name);
                    lyric_common::TypeDef resultType;
                    TU_ASSIGN_OR_RETURN (resultType, compile_expression(invokeBlock, arg, moduleEntry));
                    TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(resultType));
                }
                break;
            }

            // evaluate the ctx parameter
            case lyric_object::PlacementType::Ctx: {
                lyric_common::TypeDef contextType;
                TU_ASSIGN_OR_RETURN (contextType, reifier.reifyNextContext());

                // if ctx argument has been specified explicitly
                if (keywordArgs.contains(param->name)) {
                    const auto &arg = keywordArgs[param->name];
                    if (arg.isValid()) {
                        lyric_common::TypeDef ctxType;
                        TU_ASSIGN_OR_RETURN (ctxType, compile_expression(invokeBlock, arg, moduleEntry));
                        auto ctxUrl = ctxType.getConcreteUrl();
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

    // after explicit parameter placement, evaluate any remaining arguments as part of the rest parameter
    for (; currpos < positionalArgs.size(); currpos++) {
        lyric_common::TypeDef resultType;
        TU_ASSIGN_OR_RETURN (resultType, compile_expression(invokeBlock, positionalArgs[currpos], moduleEntry));
        TU_RETURN_IF_NOT_OK (reifier.reifyNextArgument(resultType));
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

    lyric_assembler::CallableInvoker invoker;
    TU_RETURN_IF_NOT_OK (bindingBlock->prepareFunction(functionName, invoker));

    lyric_typing::CallsiteReifier reifier(typeSystem);

    // FIXME: support callsite type arguments on function
    TU_RETURN_IF_NOT_OK (reifier.initialize(invoker));

    //
    TU_RETURN_IF_NOT_OK (compile_placement(
        invoker.getCallable(), bindingBlock, invokeBlock, reifier, walker, moduleEntry));

    return invoker.invoke(invokeBlock, reifier);
}
