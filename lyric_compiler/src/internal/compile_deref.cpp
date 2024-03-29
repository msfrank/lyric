
#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/call_invoker.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/code_builder.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_call.h>
#include <lyric_compiler/internal/compile_constant.h>
#include <lyric_compiler/internal/compile_deref.h>
#include <lyric_compiler/internal/compile_new.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/callsite_reifier.h>
#include <lyric_typing/member_reifier.h>

static tempo_utils::Result<lyric_assembler::SymbolBinding>
resolve_binding(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());

    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);
    auto resolveBindingResult = block->resolveBinding(identifier);
    if (resolveBindingResult.isStatus())
        return resolveBindingResult.getStatus();
    return resolveBindingResult.getResult();
}

static tempo_utils::Status
compile_deref_namespace(
    lyric_assembler::BlockHandle **block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry,
    int &index)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());
    auto *state = moduleEntry.getState();

    moduleEntry.checkClassOrThrow(walker, lyric_schema::kLyricAstDerefClass);

    auto *currBlock = *block;

    while (index < walker.numChildren() - 1) {
        auto element = walker.getChild(index);

        // if deref element is not a name, then we are done
        if (!element.isClass(lyric_schema::kLyricAstNameClass))
            return lyric_compiler::CompilerStatus::ok();

        auto resolveBindingResult = resolve_binding(currBlock, element, moduleEntry);
        if (resolveBindingResult.isStatus())
            return resolveBindingResult.getStatus();
        auto var = resolveBindingResult.getResult();

        // if deref element resolves to a symbol binding which is not a descriptor, then we are done
        if (var.binding != lyric_parser::BindingType::DESCRIPTOR)
            return lyric_compiler::CompilerStatus::ok();

        if (!state->symbolCache()->hasSymbol(var.symbol))
            return (*block)->logAndContinue(lyric_compiler::CompilerCondition::kMissingSymbol,
                tempo_tracing::LogSeverity::kError,
                "missing symbol {}", var.symbol.toString());
        auto *sym = state->symbolCache()->getSymbol(var.symbol);

        // if symbol binding is not a namespace, then we are done
        if (sym->getSymbolType() != lyric_assembler::SymbolType::NAMESPACE)
            return lyric_compiler::CompilerStatus::ok();

        currBlock = cast_symbol_to_namespace(sym)->namespaceBlock();

        index++;
        *block = currBlock;
    }

    return lyric_compiler::CompilerStatus::ok();
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_deref_this(
    lyric_assembler::BlockHandle *loadBlock,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());

    auto resolveVariableResult = loadBlock->resolveBinding("$this");
    if (resolveVariableResult.isStatus())
        return resolveVariableResult.getStatus();
    auto var = resolveVariableResult.getResult();
    auto *state = loadBlock->blockState();
    auto *symbol = state->symbolCache()->getSymbol(var.symbol);
    TU_ASSERT (symbol != nullptr);

    auto status = loadBlock->load(var);
    if (!status.isOk())
        return status;

    return var.type;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_deref_name(
    lyric_assembler::BlockHandle *bindingBlock,
    lyric_assembler::BlockHandle *loadBlock,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());
    auto *state = moduleEntry.getState();

    auto resolveVariableResult = resolve_binding(bindingBlock, walker, moduleEntry);
    if (resolveVariableResult.isStatus())
        return resolveVariableResult.getStatus();
    auto var = resolveVariableResult.getResult();
    auto *symbol = state->symbolCache()->getSymbol(var.symbol);
    if (symbol == nullptr)
        bindingBlock->throwAssemblerInvariant("missing symbol {}", var.symbol.toString());

    if (var.binding == lyric_parser::BindingType::DESCRIPTOR) {
        lyric_common::SymbolUrl ctorOrInitUrl;

        switch (symbol->getSymbolType()) {
            case lyric_assembler::SymbolType::ENUM:
                ctorOrInitUrl = cast_symbol_to_enum(symbol)->getCtor();
                break;
            case lyric_assembler::SymbolType::INSTANCE:
                ctorOrInitUrl = cast_symbol_to_instance(symbol)->getCtor();
                break;
            case lyric_assembler::SymbolType::STATIC:
                ctorOrInitUrl = cast_symbol_to_static(symbol)->getInitializer();
                break;
            default:
                return bindingBlock->logAndContinue(lyric_compiler::CompilerCondition::kMissingVariable,
                    tempo_tracing::LogSeverity::kError,
                    "cannot dereference symbol {}", var.symbol.toString());
        }

        // ensure that links are created for the symbol and its ctor/initializer
        symbol->touch();
        if (state->symbolCache()->hasSymbol(ctorOrInitUrl)) {
            state->symbolCache()->touchSymbol(ctorOrInitUrl);
        }
    }

    auto status = loadBlock->load(var);
    if (!status.isOk())
        return status;

    return var.type;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_deref_call(
    lyric_assembler::BlockHandle *bindingBlock,
    lyric_assembler::BlockHandle *loadBlock,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());

    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);
    return compile_function_call(bindingBlock, loadBlock, identifier, walker, moduleEntry);
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_deref_method(
    lyric_assembler::BlockHandle *block,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();

    // resolve receiver
    if (!receiverType.isValid())
        block->throwAssemblerInvariant("invalid receiver type {}", receiverType.toString());
    auto receiverUrl = receiverType.getConcreteUrl();

    if (!state->symbolCache()->hasSymbol(receiverUrl))
        return block->logAndContinue(CompilerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", receiverUrl.toString());
    auto *receiver = state->symbolCache()->getSymbol(receiverUrl);

    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    // invoke method on receiver
    switch (receiver->getSymbolType()) {

        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(receiver);
            auto resolveMethodResult = classSymbol->resolveMethod(identifier, receiverType, thisReceiver);
            if (resolveMethodResult.isStatus())
                return resolveMethodResult.getStatus();
            auto method = resolveMethodResult.getResult();

            lyric_typing::CallsiteReifier reifier(method.getParameters(), method.getRest(),
                method.getTemplateUrl(), method.getTemplateParameters(),
                method.getTemplateArguments(), typeSystem);

            auto status = compile_placement(block, block, method, reifier, walker, moduleEntry);
            if (!status.isOk())
                return status;

            auto invokeMethodResult = method.invoke(block, reifier);
            if (invokeMethodResult.isStatus())
                return invokeMethodResult.getStatus();
            return invokeMethodResult.getResult();
        }

        case lyric_assembler::SymbolType::CONCEPT: {
            auto *conceptSymbol = cast_symbol_to_concept(receiver);
            auto resolveActionResult = conceptSymbol->resolveAction(identifier, receiverType, thisReceiver);
            if (resolveActionResult.isStatus())
                return resolveActionResult.getStatus();
            auto action = resolveActionResult.getResult();

            lyric_typing::CallsiteReifier reifier(action.getParameters(), action.getRest(),
                action.getTemplateUrl(), action.getTemplateParameters(),
                action.getTemplateArguments(), typeSystem);

            auto status = compile_placement(block, block, action, reifier, walker, moduleEntry);
            if (!status.isOk())
                return status;

            auto invokeActionResult = action.invoke(block, reifier);
            if (invokeActionResult.isStatus())
                return invokeActionResult.getStatus();
            return invokeActionResult.getResult();
        }

        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(receiver);
            auto resolveMethodResult = enumSymbol->resolveMethod(identifier, receiverType, thisReceiver);
            if (resolveMethodResult.isStatus())
                return resolveMethodResult.getStatus();
            auto method = resolveMethodResult.getResult();

            lyric_typing::CallsiteReifier reifier(method.getParameters(), method.getRest(),
                method.getTemplateUrl(), method.getTemplateParameters(),
                method.getTemplateArguments(), typeSystem);

            auto status = compile_placement(block, block, method, reifier, walker, moduleEntry);
            if (!status.isOk())
                return status;

            auto invokeMethodResult = method.invoke(block, reifier);
            if (invokeMethodResult.isStatus())
                return invokeMethodResult.getStatus();
            return invokeMethodResult.getResult();
        }

        case lyric_assembler::SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = cast_symbol_to_existential(receiver);
            auto resolveMethodResult = existentialSymbol->resolveMethod(identifier, receiverType, thisReceiver);
            if (resolveMethodResult.isStatus())
                return resolveMethodResult.getStatus();
            auto method = resolveMethodResult.getResult();

            lyric_typing::CallsiteReifier reifier(method.getParameters(), method.getRest(),
                method.getTemplateUrl(), method.getTemplateParameters(),
                method.getTemplateArguments(), typeSystem);

            auto status = compile_placement(block, block, method, reifier, walker, moduleEntry);
            if (!status.isOk())
                return status;

            auto invokeMethodResult = method.invoke(block, reifier);
            if (invokeMethodResult.isStatus())
                return invokeMethodResult.getStatus();
            return invokeMethodResult.getResult();
        }

        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(receiver);
            auto resolveMethodResult = instanceSymbol->resolveMethod(identifier, receiverType, thisReceiver);
            if (resolveMethodResult.isStatus())
                return resolveMethodResult.getStatus();
            auto method = resolveMethodResult.getResult();

            lyric_typing::CallsiteReifier reifier(method.getParameters(), method.getRest(),
                method.getTemplateUrl(), method.getTemplateParameters(),
                method.getTemplateArguments(), typeSystem);

            auto status = compile_placement(block, block, method, reifier, walker, moduleEntry);
            if (!status.isOk())
                return status;

            auto invokeMethodResult = method.invoke(block, reifier);
            if (invokeMethodResult.isStatus())
                return invokeMethodResult.getStatus();
            return invokeMethodResult.getResult();
        }

        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(receiver);
            auto resolveMethodResult = structSymbol->resolveMethod(identifier, receiverType, thisReceiver);
            if (resolveMethodResult.isStatus())
                return resolveMethodResult.getStatus();
            auto method = resolveMethodResult.getResult();

            lyric_typing::CallsiteReifier reifier(method.getParameters(), method.getRest(),
                method.getTemplateUrl(), method.getTemplateParameters(),
                method.getTemplateArguments(), typeSystem);

            auto status = compile_placement(block, block, method, reifier, walker, moduleEntry);
            if (!status.isOk())
                return status;

            auto invokeMethodResult = method.invoke(block, reifier);
            if (invokeMethodResult.isStatus())
                return invokeMethodResult.getStatus();
            return invokeMethodResult.getResult();
        }

        default:
            block->throwAssemblerInvariant("invalid receiver symbol {}", receiverUrl.toString());
    }
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_deref_member(
    lyric_assembler::BlockHandle *block,
    const lyric_common::TypeDef &receiverType,
    bool thisReceiver,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();

    if (!receiverType.isValid())
        block->throwAssemblerInvariant("invalid receiver type {}", receiverType.toString());
    auto receiverUrl = receiverType.getConcreteUrl();

    if (!state->symbolCache()->hasSymbol(receiverUrl))
        return block->logAndContinue(CompilerCondition::kMissingSymbol,
            tempo_tracing::LogSeverity::kError,
            "missing symbol {}", receiverUrl.toString());
    auto *receiver = state->symbolCache()->getSymbol(receiverUrl);

    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    lyric_assembler::SymbolBinding var;
    switch (receiver->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType, classSymbol->classTemplate());
            auto resolveMemberResult = classSymbol->resolveMember(
                identifier, reifier, receiverType, thisReceiver);
            if (resolveMemberResult.isStatus())
                return resolveMemberResult.getStatus();
            var = resolveMemberResult.getResult();
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType);
            auto resolveMemberResult = structSymbol->resolveMember(
                identifier, reifier, receiverType, thisReceiver);
            if (resolveMemberResult.isStatus())
                return resolveMemberResult.getStatus();
            var = resolveMemberResult.getResult();
            break;
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType);
            auto resolveMemberResult = instanceSymbol->resolveMember(
                identifier, reifier, receiverType, thisReceiver);
            if (resolveMemberResult.isStatus())
                return resolveMemberResult.getStatus();
            var = resolveMemberResult.getResult();
            break;
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType);
            auto resolveMemberResult = enumSymbol->resolveMember(
                identifier, reifier, receiverType, thisReceiver);
            if (resolveMemberResult.isStatus())
                return resolveMemberResult.getStatus();
            var = resolveMemberResult.getResult();
            break;
        }
        default:
            block->throwAssemblerInvariant("invalid receiver symbol {}", receiverUrl.toString());
    }

    auto status = block->load(var);
    if (!status.isOk())
        return status;

    // drop the previous result from the stack
    // FIXME: swap receiver for member within opcode
    auto *code = block->blockCode();
    status = code->rdropValue(1);
    if (!status.isOk())
        return status;

    return var.type;
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::internal::compile_deref(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());

    moduleEntry.checkClassAndChildRangeOrThrow(walker, lyric_schema::kLyricAstDerefClass, 1);
    auto index = 0;

    //
    auto *nsBlock = block;
    auto status = compile_deref_namespace(&nsBlock, walker, moduleEntry, index);
    if (status.notOk())
        return status;

    lyric_common::TypeDef derefType;
    bool thisReceiver = false;

    auto initial = walker.getChild(index);
    lyric_schema::LyricAstId initialId{};
    moduleEntry.parseIdOrThrow(initial, lyric_schema::kLyricAstVocabulary, initialId);

    // load the initial deref element onto the top of the stack
    switch (initialId) {

        // this expression
        case lyric_schema::LyricAstId::This: {
            auto derefThisResult = compile_deref_this(block, initial, moduleEntry);
            if (derefThisResult.isStatus())
                return derefThisResult;
            derefType = derefThisResult.getResult();
            thisReceiver = true;
            index++;
            break;
        }

        // name expression
        case lyric_schema::LyricAstId::Name: {
            auto derefNameResult = compile_deref_name(nsBlock, block, initial, moduleEntry);
            if (derefNameResult.isStatus())
                return derefNameResult.getStatus();
            derefType = derefNameResult.getResult();
            index++;
            break;
        }

        // call expression
        case lyric_schema::LyricAstId::Call: {
            auto derefCallResult = compile_deref_call(nsBlock, block, initial, moduleEntry);
            if (derefCallResult.isStatus())
                return derefCallResult;
            derefType = derefCallResult.getResult();
            index++;
            break;
        }

        // grouping expression
        case lyric_schema::LyricAstId::Block: {
            auto derefGroupingResult = compile_block(block, initial, moduleEntry);
            if (derefGroupingResult.isStatus())
                return derefGroupingResult;
            derefType = derefGroupingResult.getResult();
            index++;
            break;
        }

        // new expression
        case lyric_schema::LyricAstId::New: {
            auto derefNewResult = compile_new(block, initial, moduleEntry);
            if (derefNewResult.isStatus())
                return derefNewResult;
            derefType = derefNewResult.getResult();
            index++;
            break;
        }

        // literal expression
        case lyric_schema::LyricAstId::Nil:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Url:
        case lyric_schema::LyricAstId::SymbolRef: {
            auto derefLiteralResult = compile_constant(block, initial, moduleEntry);
            if (derefLiteralResult.isStatus())
                return derefLiteralResult;
            derefType = derefLiteralResult.getResult();
            index++;
            break;
        }

        default:
            block->throwSyntaxError(initial, "invalid deref");
    }

    for (; index < walker.numChildren(); index++) {
        auto next = walker.getChild(index);
        lyric_schema::LyricAstId nextId{};
        moduleEntry.parseIdOrThrow(next, lyric_schema::kLyricAstVocabulary, nextId);
        switch (nextId) {

            case lyric_schema::LyricAstId::Name: {
                auto derefMemberResult = compile_deref_member(
                    block, derefType, thisReceiver, next, moduleEntry);
                if (derefMemberResult.isStatus())
                    return derefMemberResult;
                derefType = derefMemberResult.getResult();
                break;
            }

            case lyric_schema::LyricAstId::Call: {
                auto derefMethodResult = compile_deref_method(
                    block, derefType, thisReceiver, next, moduleEntry);
                if (derefMethodResult.isStatus())
                    return derefMethodResult;
                derefType = derefMethodResult.getResult();
                break;
            }

            default:
                block->throwSyntaxError(initial, "invalid deref");
        }
        thisReceiver = false; // unset isReceiver after first iteration
    }

    return derefType;
}
