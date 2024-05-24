
#include <lyric_assembler/argument_variable.h>
#include <lyric_assembler/class_symbol.h>
#include <lyric_assembler/code_builder.h>
#include <lyric_assembler/enum_symbol.h>
#include <lyric_assembler/instance_symbol.h>
#include <lyric_assembler/lexical_variable.h>
#include <lyric_assembler/local_variable.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_assembler/struct_symbol.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/internal/compile_assignment.h>
#include <lyric_compiler/internal/compile_deref.h>
#include <lyric_compiler/internal/compile_node.h>
#include <lyric_compiler/internal/compile_operator.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_typing/member_reifier.h>

static tempo_utils::Result<lyric_common::TypeDef>
compile_inplace_assignment(
    lyric_assembler::BlockHandle *block,
    const lyric_common::TypeDef &lhs,
    lyric_schema::LyricAstId operationId,
    const lyric_parser::NodeWalker &expression,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    auto exprResult = lyric_compiler::internal::compile_expression(block, expression, moduleEntry);
    if (exprResult.isStatus())
        return exprResult;

    std::vector<lyric_common::TypeDef> argList;
    argList.push_back(lhs);
    argList.push_back(exprResult.getResult());

    switch (operationId) {
        case lyric_schema::LyricAstId::InplaceAdd:
            operationId = lyric_schema::LyricAstId::Add;
            break;
        case lyric_schema::LyricAstId::InplaceSub:
            operationId = lyric_schema::LyricAstId::Sub;
            break;
        case lyric_schema::LyricAstId::InplaceMul:
            operationId = lyric_schema::LyricAstId::Mul;
            break;
        case lyric_schema::LyricAstId::InplaceDiv:
            operationId = lyric_schema::LyricAstId::Div;
            break;
        default:
            block->throwAssemblerInvariant("invalid assignment operation");
    }
    return lyric_compiler::internal::compile_operator_call(block, operationId, argList, moduleEntry);
}

static tempo_utils::Status
compile_set_name(
    lyric_assembler::BlockHandle *block,
    lyric_schema::LyricAstId assignmentId,
    const lyric_parser::NodeWalker &name,
    const lyric_parser::NodeWalker &expression,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (name.isValid());
    TU_ASSERT (expression.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();
    auto *state = moduleEntry.getState();

    std::string identifier;
    moduleEntry.parseAttrOrThrow(name, lyric_parser::kLyricAstIdentifier, identifier);

    auto resolveVariableResult = block->resolveReference(identifier);
    if (resolveVariableResult.isStatus())
        return resolveVariableResult.getStatus();
    auto ref = resolveVariableResult.getResult();
    if (ref.referenceType == lyric_assembler::ReferenceType::Value)
        return state->logAndContinue(lyric_compiler::CompilerCondition::kInvalidBinding,
            tempo_tracing::LogSeverity::kError,
            "value {} cannot be assigned to", identifier);
    auto targetType = ref.typeDef;

    lyric_assembler::AbstractSymbol *symbol;
    TU_ASSIGN_OR_RETURN (symbol, state->symbolCache()->getOrImportSymbol(ref.symbolUrl));
    symbol->touch();

    // evaluation of the rhs is expected to push the value onto the stack
    lyric_common::TypeDef rvalueType;
    if (assignmentId == lyric_schema::LyricAstId::Set) {
        // assignment type is a simple assignment
        auto exprResult = lyric_compiler::internal::compile_expression(block, expression, moduleEntry);
        if (exprResult.isStatus())
            return exprResult.getStatus();
        rvalueType = exprResult.getResult();
    } else {
        // load the lhs onto the stack, then perform the inplace assignment
        auto status = block->load(ref);
        if (!status.isOk())
            return status;
        auto inplaceResult = compile_inplace_assignment(block, targetType, assignmentId, expression, moduleEntry);
        if (inplaceResult.isStatus())
            return inplaceResult.getStatus();
        rvalueType = inplaceResult.getResult();
    }

    bool isAssignable;

    // check that the rhs is assignable to the target type
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(targetType, rvalueType));
    if (!isAssignable)
        return state->logAndContinue(lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "target does not match rvalue type {}", rvalueType.toString());

    // store the result of evaluating the rhs
    switch (symbol->getSymbolType()) {
        case lyric_assembler::SymbolType::ARGUMENT: {
            auto *argSymbol = lyric_assembler::cast_symbol_to_argument(symbol);
            return block->blockCode()->storeArgument(argSymbol->getOffset());
        }
        case lyric_assembler::SymbolType::LOCAL: {
            auto *localSymbol = lyric_assembler::cast_symbol_to_local(symbol);
            return block->blockCode()->storeLocal(localSymbol->getOffset());
        }
        case lyric_assembler::SymbolType::LEXICAL: {
            auto *lexicalSymbol = lyric_assembler::cast_symbol_to_lexical(symbol);
            return block->blockCode()->storeLexical(lexicalSymbol->getOffset());
        }
        case lyric_assembler::SymbolType::STATIC: {
            auto *staticSymbol = lyric_assembler::cast_symbol_to_static(symbol);
            return block->blockCode()->storeStatic(staticSymbol->getAddress());
        }
        default:
            break;
    }

    block->throwSyntaxError(name, "invalid symbol type for target");
}

static tempo_utils::Status
compile_set_member(
    lyric_assembler::BlockHandle *block,
    lyric_schema::LyricAstId assignmentId,
    const lyric_parser::NodeWalker &target,
    const lyric_parser::NodeWalker &expression,
    lyric_compiler::ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (target.isValid());
    TU_ASSERT (expression.isValid());
    auto *typeSystem = moduleEntry.getTypeSystem();

    moduleEntry.checkClassAndChildRangeOrThrow(target, lyric_schema::kLyricAstTargetClass, 2);

    lyric_common::TypeDef receiverType;
    bool thisReceiver = false;

    // deref initial target and push onto the top of the stack
    int i = 0;
    auto curr = target.getChild(i);
    lyric_schema::LyricAstId initialId;
    moduleEntry.parseIdOrThrow(curr, lyric_schema::kLyricAstVocabulary, initialId);

    switch (initialId) {

        case lyric_schema::LyricAstId::This: {
            auto resolveThisResult = lyric_compiler::internal::compile_deref_this(block, curr, moduleEntry);
            if (resolveThisResult.isStatus())
                return resolveThisResult.getStatus();
            receiverType = resolveThisResult.getResult();
            thisReceiver = true;
            break;
        }

        case lyric_schema::LyricAstId::Name: {
            auto resolveNameResult = lyric_compiler::internal::compile_deref_name(block, block, curr, moduleEntry);
            if (resolveNameResult.isStatus())
                return resolveNameResult.getStatus();
            receiverType = resolveNameResult.getResult();
            break;
        }

        default:
            block->throwSyntaxError(curr, "invalid target");
    }

    auto *code = block->blockCode();

    // dereference members until the receiver is on the top of the stack
    for (++i; i < target.numChildren() - 1; i++) {
        curr = target.getChild(i);
        moduleEntry.checkClassOrThrow(curr, lyric_schema::kLyricAstNameClass);

        auto resolveMemberResult = lyric_compiler::internal::compile_deref_member(
            block, receiverType, thisReceiver, curr, moduleEntry);
        if (resolveMemberResult.isStatus())
            return resolveMemberResult.getStatus();
        receiverType = resolveMemberResult.getResult();

        // drop the previous target from the stack
        auto status = code->rdropValue(1);
        if (!status.isOk())
            return status;

        thisReceiver = false;   // unset isReceiver after first iteration
    }

    // if assigning to this, then determine whether we are in a constructor
    bool isCtor = false;
    if (thisReceiver) {
        auto activationUrl = block->blockProc()->getActivation();
        lyric_assembler::AbstractSymbol *activationSym;
        TU_ASSIGN_OR_RETURN (activationSym, block->blockState()->symbolCache()->getOrImportSymbol(activationUrl));
        if (activationSym->getSymbolType() != lyric_assembler::SymbolType::CALL)
            block->throwAssemblerInvariant("invalid block definition");
        auto *activationCall = cast_symbol_to_call(activationSym);
        isCtor = activationCall->isCtor();
    }

    auto *state = moduleEntry.getState();
    auto receiverUrl = receiverType.getConcreteUrl();
    lyric_assembler::AbstractSymbol *receiver;
    TU_ASSIGN_OR_RETURN (receiver, state->symbolCache()->getOrImportSymbol(receiverUrl));

    curr = target.getChild(i);
    moduleEntry.checkClassOrThrow(curr, lyric_schema::kLyricAstNameClass);
    std::string identifier;
    moduleEntry.parseAttrOrThrow(curr, lyric_parser::kLyricAstIdentifier, identifier);

    // resolve member variable binding
    lyric_assembler::DataReference ref;
    bool isInitialized;
    switch (receiver->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            auto *classSymbol = cast_symbol_to_class(receiver);
            lyric_typing::MemberReifier reifier(typeSystem, receiverType, classSymbol->classTemplate());
            auto resolveMemberResult = classSymbol->resolveMember(
                identifier, reifier, receiverType, thisReceiver);
            if (resolveMemberResult.isStatus())
                return resolveMemberResult.getStatus();
            isInitialized = classSymbol->isMemberInitialized(identifier);
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
            isInitialized = instanceSymbol->isMemberInitialized(identifier);
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
            isInitialized = enumSymbol->isMemberInitialized(identifier);
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
            isInitialized = structSymbol->isMemberInitialized(identifier);
            ref = resolveMemberResult.getResult();
            break;
        }
        default:
            state->throwAssemblerInvariant("invalid deref receiver");
    }

    //
    switch (ref.referenceType) {
        case lyric_assembler::ReferenceType::Variable:
            break;
        case lyric_assembler::ReferenceType::Value: {
            if (isCtor && thisReceiver && !isInitialized)
                break;
            return state->logAndContinue(lyric_compiler::CompilerCondition::kInvalidBinding,
                tempo_tracing::LogSeverity::kError,
                "value {} cannot be assigned to", identifier);
        }
        default:
            block->throwAssemblerInvariant("invalid deref receiver");
    }

    lyric_common::TypeDef rvalueType;

    // evaluation of the rhs is expected to push the value onto the stack
    if (assignmentId == lyric_schema::LyricAstId::Set) {
        // assignment type is a simple assignment
        auto exprResult = lyric_compiler::internal::compile_expression(block, expression, moduleEntry);
        if (exprResult.isStatus())
            return exprResult.getStatus();
        rvalueType = exprResult.getResult();
    } else {
        // load the lhs onto the stack, then perform the inplace assignment
        auto status = block->load(ref);
        if (!status.isOk())
            return status;
        auto inplaceResult = compile_inplace_assignment(block, ref.typeDef, assignmentId, expression, moduleEntry);
        if (inplaceResult.isStatus())
            return status;
        rvalueType = inplaceResult.getResult();
    }

    bool isAssignable;

    // check that the rhs is assignable to the member type
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(ref.typeDef, rvalueType));
    if (!isAssignable)
        return state->logAndContinue(lyric_compiler::CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "member does not match rvalue type {}", rvalueType.toString());

    // store expression result
    auto status = block->store(ref);
    if (status.notOk())
        return status;

    // mark member as initialized if the member was not initialized before
    if (!isInitialized) {
        switch (receiver->getSymbolType()) {
            case lyric_assembler::SymbolType::CLASS: {
                auto *classReceiver = cast_symbol_to_class(receiver);
                return classReceiver->setMemberInitialized(identifier);
            }
            case lyric_assembler::SymbolType::INSTANCE: {
                auto *instanceReceiver = cast_symbol_to_instance(receiver);
                return instanceReceiver->setMemberInitialized(identifier);
            }
            case lyric_assembler::SymbolType::ENUM: {
                auto *enumReceiver = cast_symbol_to_enum(receiver);
                return enumReceiver->setMemberInitialized(identifier);
            }
            case lyric_assembler::SymbolType::STRUCT: {
                auto *structReceiver = cast_symbol_to_struct(receiver);
                return structReceiver->setMemberInitialized(identifier);
            }
            default:
                block->throwAssemblerInvariant("invalid deref receiver");
        }
    }

    return lyric_compiler::CompilerStatus::ok();
}

tempo_utils::Status
lyric_compiler::internal::compile_set(
    lyric_assembler::BlockHandle *block,
    const lyric_parser::NodeWalker &walker,
    ModuleEntry &moduleEntry)
{
    TU_ASSERT (block != nullptr);
    TU_ASSERT (walker.isValid());

    auto lvalue = walker.getChild(0);
    auto rvalue = walker.getChild(1);

    lyric_schema::LyricAstId assignmentId{};
    moduleEntry.parseIdOrThrow(walker, lyric_schema::kLyricAstVocabulary, assignmentId);
    lyric_schema::LyricAstId lvalueId{};
    moduleEntry.parseIdOrThrow(lvalue, lyric_schema::kLyricAstVocabulary, lvalueId);

    if (lvalueId == lyric_schema::LyricAstId::Target)
        return compile_set_member(block, assignmentId, lvalue, rvalue, moduleEntry);
    if (lvalueId == lyric_schema::LyricAstId::Name)
        return compile_set_name(block, assignmentId, lvalue, rvalue, moduleEntry);

    block->throwSyntaxError(lvalue, "invalid lvalue");
}
