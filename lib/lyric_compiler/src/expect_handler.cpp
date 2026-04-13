
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/expect_handler.h>
#include <lyric_compiler/form_handler.h>

lyric_compiler::ExpectHandler::ExpectHandler(
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
lyric_compiler::ExpectHandler::before(
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
lyric_compiler::ExpectHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    // get the result type of the expect operand
    auto successOrErrorType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    //
    auto *fundamentalCache = driver->getFundamentalCache();
    auto ErrorType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Error);

    auto *typeSystem = driver->getTypeSystem();

    lyric_common::TypeDef successType;
    std::vector<lyric_common::TypeDef> errorMembers;

    // determine the result type
    if (successOrErrorType.getType() == lyric_common::TypeDefType::Union) {
        std::vector<lyric_common::TypeDef> successMembers;
        for (const auto &memberType : successOrErrorType.getUnionMembers()) {
            lyric_runtime::TypeComparison cmp;
            TU_ASSIGN_OR_RETURN (cmp, typeSystem->compareAssignable(ErrorType, memberType));
            switch (cmp) {
                case lyric_runtime::TypeComparison::SUPER:
                    return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
                        "operand type member {} is ambiguous for expect expression", memberType.toString());
                case lyric_runtime::TypeComparison::EQUAL:
                case lyric_runtime::TypeComparison::EXTENDS:
                    errorMembers.push_back(memberType);
                    break;
                case lyric_runtime::TypeComparison::DISJOINT:
                    successMembers.push_back(memberType);
                    break;
            }
        }
        if (errorMembers.empty())
            return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
                "incompatible operand type {} for expect expression; no error type declared",
                successOrErrorType.toString());
        if (successMembers.empty())
            return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
                "incompatible operand type {} for expect expression; no success type declared",
                successOrErrorType.toString());
        TU_ASSIGN_OR_RETURN (successType, lyric_common::TypeDef::forUnion(successMembers));
    } else {
        return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
            "incompatible operand type {} for expect expression; operand must be a union type",
            successOrErrorType.toString());
    }

    auto *procHandle = block->blockProc();

    // add each error type as an exit type for the proc
    for (const auto &errorType : errorMembers) {
        procHandle->putExitType(errorType);
    }

    // store the operand in a temporary variable
    lyric_assembler::DataReference resultVar;
    TU_ASSIGN_OR_RETURN (resultVar, block->declareTemporary(successOrErrorType, /* isVariable= */ false));
    TU_RETURN_IF_NOT_OK (m_fragment->storeRef(resultVar, /* initialStore= */ true));

    // perform type comparison on the result
    TU_RETURN_IF_NOT_OK (m_fragment->loadRef(resultVar));
    TU_RETURN_IF_NOT_OK (m_fragment->invokeTypeOf());
    TU_RETURN_IF_NOT_OK (m_fragment->loadType(ErrorType));
    TU_RETURN_IF_NOT_OK (m_fragment->typeCompare());

    // if result type <= ErrorType (i.e. result is equal or extends) then return result
    lyric_assembler::JumpTarget jumpTarget;
    TU_ASSIGN_OR_RETURN (jumpTarget, m_fragment->jumpIfGreaterThan());
    TU_RETURN_IF_NOT_OK (m_fragment->loadRef(resultVar));
    TU_RETURN_IF_NOT_OK (m_fragment->returnToCaller());

    // otherwise result type > StatusType (i.e. result is super or disjoint)
    lyric_assembler::JumpLabel jumpLabel;
    TU_ASSIGN_OR_RETURN (jumpLabel, m_fragment->appendLabel());
    TU_RETURN_IF_NOT_OK (m_fragment->patchTarget(jumpTarget, jumpLabel));

    // if expression is not a side effect then push the result onto stack
    if (!m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (m_fragment->loadRef(resultVar));
        TU_RETURN_IF_NOT_OK (driver->pushResult(successType));
    }

    return {};
}
