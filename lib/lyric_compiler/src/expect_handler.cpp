
#include <lyric_compiler/expect_handler.h>
#include <lyric_compiler/form_handler.h>

#include "lyric_assembler/fundamental_cache.h"
#include "lyric_compiler/compiler_result.h"

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
    auto resultOrStatusType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());

    //
    auto *fundamentalCache = driver->getFundamentalCache();
    auto StatusType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Status);

    auto *typeSystem = driver->getTypeSystem();

    // determine the result type
    lyric_common::TypeDef resultType;
    if (resultOrStatusType.getType() == lyric_common::TypeDefType::Union) {
        std::vector<lyric_common::TypeDef> resultMembers;
        bool canReturnStatus = false;
        for (const auto &memberType : resultOrStatusType.getUnionMembers()) {
            bool isAssignable;
            TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(StatusType, memberType));
            if (isAssignable) {
                canReturnStatus = true;
            } else {
                resultMembers.push_back(memberType);
            }
        }
        if (!canReturnStatus || resultMembers.empty())
            return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
                "incompatible operand type {} for expect expression", resultOrStatusType.toString());
        TU_ASSIGN_OR_RETURN (resultType, lyric_common::TypeDef::forUnion(resultMembers));
    } else {
        return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
            "incompatible operand type {} for expect expression", resultOrStatusType.toString());
    }

    // add Status as an exit type for the proc
    auto *procHandle = block->blockProc();
    procHandle->putExitType(StatusType);

    // store the operand in a temporary variable
    lyric_assembler::DataReference resultVar;
    TU_ASSIGN_OR_RETURN (resultVar, block->declareTemporary(resultOrStatusType, /* isVariable= */ false));
    TU_RETURN_IF_NOT_OK (m_fragment->storeRef(resultVar, /* initialStore= */ true));

    // perform type comparison on the result
    TU_RETURN_IF_NOT_OK (m_fragment->loadRef(resultVar));
    TU_RETURN_IF_NOT_OK (m_fragment->invokeTypeOf());
    TU_RETURN_IF_NOT_OK (m_fragment->loadType(StatusType));
    TU_RETURN_IF_NOT_OK (m_fragment->typeCompare());

    // if result type <= StatusType (i.e. result is equal or extends) then return result
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
        TU_RETURN_IF_NOT_OK (driver->pushResult(resultType));
    }

    return {};
}
