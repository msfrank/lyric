
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>
#include <lyric_compiler/variable_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::VariableHandler::VariableHandler(
    bool isVariable,
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isVariable(isVariable),
      m_isSideEffect(isSideEffect),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::VariableHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto expression = std::make_unique<FormChoice>(
        FormType::Expression, m_fragment, block, driver);
    ctx.appendChoice(std::move(expression));
    return {};
}

tempo_utils::Status
lyric_compiler::VariableHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    auto *block = BaseGrouping::getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    lyric_parser::AccessType access;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstAccessType, access));

    // resolve the declaration type if it is present
    lyric_common::TypeDef declarationType;
    if (node->hasAttr(lyric_parser::kLyricAstTypeOffset)) {
        lyric_parser::ArchetypeNode *typeNode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
        lyric_typing::TypeSpec declarationSpec;
        TU_ASSIGN_OR_RETURN (declarationSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
        TU_ASSIGN_OR_RETURN (declarationType, typeSystem->resolveAssignable(block, declarationSpec));
    }

    // pop the expression result type
    auto expressionType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    // if val type is not explicitly declared, then use the derived type
    if (!declarationType.isValid()) {
        declarationType = expressionType;
    }

    // val type was explicitly declared and is disjoint from rvalue type
    bool isAssignable;
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(declarationType, expressionType));
    if (!isAssignable)
        return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
            "expression {} is incompatible with declared type {}",
            expressionType.toString(), declarationType.toString());

    lyric_assembler::DataReference var;
    TU_ASSIGN_OR_RETURN (var, block->declareVariable(
        identifier, convert_access_type(access), declarationType, m_isVariable));

    TU_RETURN_IF_NOT_OK (m_fragment->storeRef(var, /* initialStore= */ true));

    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (driver->pushResult(lyric_common::TypeDef::noReturn()));
    }

    return {};
}