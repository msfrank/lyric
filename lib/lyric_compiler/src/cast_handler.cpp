
#include <lyric_compiler/cast_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/form_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::CastHandler::CastHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::CastHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto op1 = std::make_unique<FormChoice>(FormType::Expression, m_fragment, block, driver);
    ctx.appendChoice(std::move(op1));
    return {};
}

tempo_utils::Status
lyric_compiler::CastHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec castSpec;
    TU_ASSIGN_OR_RETURN (castSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
    lyric_common::TypeDef castType;
    TU_ASSIGN_OR_RETURN (castType, typeSystem->resolveAssignable(block, castSpec));

    // get the target type of the cast
    auto targetType = driver->peekResult();

    lyric_runtime::TypeComparison cmp;
    TU_ASSIGN_OR_RETURN (cmp, typeSystem->compareAssignable(castType, targetType));
    switch (cmp) {
        case lyric_runtime::TypeComparison::EQUAL:
        case lyric_runtime::TypeComparison::EXTENDS:
            TU_RETURN_IF_NOT_OK (driver->popResult());
            TU_RETURN_IF_NOT_OK (driver->pushResult(castType));
            break;
        case lyric_runtime::TypeComparison::DISJOINT:
        case lyric_runtime::TypeComparison::SUPER:
            return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
                "cannot cast {} to {}", targetType.toString(), castType.toString());
    }

    // if expression is a side effect then pop the result off of the stack
    if (m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (m_fragment->popValue());
        TU_RETURN_IF_NOT_OK (driver->popResult());
    }

    return {};
}
