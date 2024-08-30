
#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/function_callable.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/concept_symbol.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/existential_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_block.h>
#include <lyric_compiler/internal/compile_call.h>
#include <lyric_compiler/internal/compile_constant.h>
#include <lyric_compiler/internal/compile_deref.h>
#include <lyric_compiler/internal/compile_new.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/callsite_reifier.h>
#include <lyric_typing/member_reifier.h>

static tempo_utils::Result<lyric_assembler::DataReference>
resolve_binding(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (walker.isValid());

    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);
    auto resolveBindingResult = block->resolveReference(identifier);
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
        auto ref = resolveBindingResult.getResult();

        // if deref element resolves to a symbol binding which is not a descriptor, then we are done
        if (ref.referenceType != lyric_assembler::ReferenceType::Descriptor)
            return lyric_compiler::CompilerStatus::ok();

        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(ref.symbolUrl));

        // if symbol binding is not a namespace, then we are done
        if (symbol->getSymbolType() != lyric_assembler::SymbolType::NAMESPACE)
            return lyric_compiler::CompilerStatus::ok();

        currBlock = cast_symbol_to_namespace(symbol)->namespaceBlock();

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

    auto resolveVariableResult = loadBlock->resolveReference("$this");
    if (resolveVariableResult.isStatus())
        return resolveVariableResult.getStatus();
    auto ref = resolveVariableResult.getResult();
    auto *state = loadBlock->blockState();

    // ensure symbol for $this is loaded
    TU_RETURN_IF_STATUS(state->symbolCache()->getOrImportSymbol(ref.symbolUrl));

    auto status = loadBlock->load(ref);
    if (!status.isOk())
        return status;

    return ref.typeDef;
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
    auto ref = resolveVariableResult.getResult();
    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(ref.symbolUrl));

//    if (ref.referenceType == lyric_assembler::ReferenceType::Descriptor) {
//        lyric_common::SymbolUrl ctorOrInitUrl;
//
//        switch (symbol->getSymbolType()) {
//            case lyric_assembler::SymbolType::ENUM:
//                ctorOrInitUrl = cast_symbol_to_enum(symbol)->getCtor();
//                break;
//            case lyric_assembler::SymbolType::INSTANCE:
//                ctorOrInitUrl = cast_symbol_to_instance(symbol)->getCtor();
//                break;
//            case lyric_assembler::SymbolType::STATIC:
//                ctorOrInitUrl = cast_symbol_to_static(symbol)->getInitializer();
//                break;
//            default:
//                return bindingBlock->logAndContinue(lyric_compiler::CompilerCondition::kMissingVariable,
//                    tempo_tracing::LogSeverity::kError,
//                    "cannot dereference symbol {}", ref.symbolUrl.toString());
//        }
//
//        // ensure that links are created for the symbol and its ctor/initializer
//        symbol->touch();
//        if (state->symbolCache()->hasSymbol(ctorOrInitUrl)) {
//            state->symbolCache()->touchSymbol(ctorOrInitUrl);
//        }
//    }
//
//    auto status = loadBlock->load(ref);
//    if (!status.isOk())
//        return status;

    auto *blockCode = loadBlock->blockCode();
    auto *fragment = blockCode->rootFragment();
    TU_RETURN_IF_NOT_OK (fragment->loadData(symbol));

    return ref.typeDef;
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
    if (receiverType.getType() != lyric_common::TypeDefType::Concrete)
        block->throwAssemblerInvariant("invalid receiver type {}", receiverType.toString());
    auto receiverUrl = receiverType.getConcreteUrl();

    // extract the type arguments from the receiver type
    auto concreteArguments = receiverType.getConcreteArguments();
    std::vector<lyric_common::TypeDef> callsiteArguments(concreteArguments.begin(), concreteArguments.end());

    // if type arguments are specified at the callsite then append them
    if (walker.hasAttr(lyric_parser::kLyricAstTypeArgumentsOffset)) {
        lyric_parser::NodeWalker typeArgumentsNode;
        moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstTypeArgumentsOffset, typeArgumentsNode);
        std::vector<lyric_typing::TypeSpec> typeArgumentsSpec;
        TU_ASSIGN_OR_RETURN (typeArgumentsSpec, typeSystem->parseTypeArguments(block, typeArgumentsNode));
        std::vector<lyric_common::TypeDef> typeArguments;
        TU_ASSIGN_OR_RETURN (typeArguments, typeSystem->resolveTypeArguments(block, typeArgumentsSpec));
        callsiteArguments.insert(callsiteArguments.end(), typeArguments.cbegin(), typeArguments.cend());
    }

    // get the symbol for the receiver
    lyric_assembler::AbstractSymbol *receiver;
    TU_ASSIGN_OR_RETURN (receiver, state->symbolCache()->getOrImportSymbol(receiverUrl));

    // get the method name
    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    lyric_assembler::CallableInvoker invoker;

    // invoke method on receiver
    switch (receiver->getSymbolType()) {

        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(receiver);
            TU_RETURN_IF_NOT_OK (classSymbol->prepareMethod(identifier, receiverType, invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::CONCEPT: {
            auto *conceptSymbol = cast_symbol_to_concept(receiver);
            TU_RETURN_IF_NOT_OK (conceptSymbol->prepareAction(identifier, receiverType, invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(receiver);
            TU_RETURN_IF_NOT_OK (enumSymbol->prepareMethod(identifier, receiverType, invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::EXISTENTIAL: {
            auto *existentialSymbol = cast_symbol_to_existential(receiver);
            TU_RETURN_IF_NOT_OK (existentialSymbol->prepareMethod(identifier, receiverType, invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(receiver);
            TU_RETURN_IF_NOT_OK (instanceSymbol->prepareMethod(identifier, receiverType, invoker, thisReceiver));
            break;
        }

        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(receiver);
            TU_RETURN_IF_NOT_OK (structSymbol->prepareMethod(identifier, receiverType, invoker, thisReceiver));
            break;
        }

        default:
            block->throwAssemblerInvariant("invalid receiver symbol {}", receiverUrl.toString());
    }

    lyric_typing::CallsiteReifier reifier(typeSystem);
    TU_RETURN_IF_NOT_OK (reifier.initialize(invoker, callsiteArguments));
    TU_RETURN_IF_NOT_OK (compile_placement(invoker.getCallable(), block, block, reifier, walker, moduleEntry));
    return invoker.invoke(block, reifier);
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

    lyric_assembler::AbstractSymbol *receiver;
    TU_ASSIGN_OR_RETURN (receiver, state->symbolCache()->getOrImportSymbol(receiverUrl));

    std::string identifier;
    moduleEntry.parseAttrOrThrow(walker, lyric_parser::kLyricAstIdentifier, identifier);

    lyric_assembler::DataReference ref;
    switch (receiver->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType, classSymbol->classTemplate());
            auto resolveMemberResult = classSymbol->resolveMember(
                identifier, reifier, receiverType, thisReceiver);
            if (resolveMemberResult.isStatus())
                return resolveMemberResult.getStatus();
            ref = resolveMemberResult.getResult();
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            auto *structSymbol = cast_symbol_to_struct(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType);
            auto resolveMemberResult = structSymbol->resolveMember(
                identifier, reifier, receiverType, thisReceiver);
            if (resolveMemberResult.isStatus())
                return resolveMemberResult.getStatus();
            ref = resolveMemberResult.getResult();
            break;
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            auto *instanceSymbol = cast_symbol_to_instance(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType);
            auto resolveMemberResult = instanceSymbol->resolveMember(
                identifier, reifier, receiverType, thisReceiver);
            if (resolveMemberResult.isStatus())
                return resolveMemberResult.getStatus();
            ref = resolveMemberResult.getResult();
            break;
        }
        case lyric_assembler::SymbolType::ENUM: {
            auto *enumSymbol = cast_symbol_to_enum(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType);
            auto resolveMemberResult = enumSymbol->resolveMember(
                identifier, reifier, receiverType, thisReceiver);
            if (resolveMemberResult.isStatus())
                return resolveMemberResult.getStatus();
            ref = resolveMemberResult.getResult();
            break;
        }
        default:
            block->throwAssemblerInvariant("invalid receiver symbol {}", receiverUrl.toString());
    }

    auto status = block->load(ref);
    if (!status.isOk())
        return status;

    auto *procCode = block->blockCode();
    auto *fragment = procCode->rootFragment();

    // drop the previous result from the stack
    // FIXME: swap receiver for member within opcode
    TU_RETURN_IF_NOT_OK (fragment->rdropValue(1));

    return ref.typeDef;
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
            block->throwAssemblerInvariant("invalid deref");
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
                block->throwAssemblerInvariant("invalid deref");
        }
        thisReceiver = false; // unset isReceiver after first iteration
    }

    return derefType;
}
