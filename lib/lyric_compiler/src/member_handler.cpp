
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/form_handler.h>
#include <lyric_compiler/internal/compiler_utils.h>
#include <lyric_compiler/member_handler.h>
#include <lyric_compiler/new_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::MemberHandler::MemberHandler(
    Member member,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_member(std::move(member))
{
    TU_ASSERT (m_member.fieldSymbol != nullptr);
}

tempo_utils::Status
lyric_compiler::MemberHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_INFO << "before MemberHandler@" << this;

    // if there is no initializer then we are done
    if (node->numChildren() == 0)
        return {};

    auto *block = getBlock();
    auto *driver = getDriver();
    auto memberType = m_member.fieldSymbol->getTypeDef();

    auto init = std::make_unique<MemberInit>(
        memberType, m_member.procHandle, block, driver);
    ctx.appendChoice(std::move(init));

    return {};
}

tempo_utils::Status
lyric_compiler::MemberHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_INFO << "after MemberHandler@" << this;

    if (m_member.procHandle == nullptr)
        return {};

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();
    auto *proc = m_member.procHandle;
    auto *code = proc->procCode();
    auto *fragment = code->rootFragment();

    auto initializerType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());
    proc->putExitType(initializerType);

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    auto memberType = m_member.fieldSymbol->getTypeDef();
    bool isReturnable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(memberType, initializerType));
    if (!isReturnable)
        return block->logAndContinue(CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "member initializer is incompatible with type {}", memberType.toString());

    // validate that each exit returns the expected type
    for (auto it = proc->exitTypesBegin(); it != proc->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(memberType, *it));
        if (!isReturnable)
            return block->logAndContinue(CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "member initializer is incompatible with type {}", memberType.toString());
    }

    return {};
}

lyric_compiler::MemberInit::MemberInit(
    const lyric_common::TypeDef &memberType,
    lyric_assembler::ProcHandle *procHandle,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_memberType(memberType),
      m_procHandle(procHandle)
{
    TU_ASSERT (m_memberType.isValid());
    TU_ASSERT (m_procHandle != nullptr);
}

tempo_utils::Status
lyric_compiler::MemberInit::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    TU_LOG_INFO << "decide ParamInit@" << this << ": "
                << resource->getNsUrl() << "#" << resource->getName();

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fragment = m_procHandle->procCode()->rootFragment();

    switch (astId) {

        // terminal forms
        case lyric_schema::LyricAstId::Nil:
        case lyric_schema::LyricAstId::Undef:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Url:
        case lyric_schema::LyricAstId::SymbolRef: {
            auto terminal = std::make_unique<TerminalFormBehavior>(
                /* isSideEffect */ false, fragment, block, driver);
            ctx.setBehavior(std::move(terminal));
            return {};
        }

            // FIXME: handle new expression
        case lyric_schema::LyricAstId::New: {
            auto new_ = std::make_unique<NewHandler>(
                m_memberType, /* isSideEffect */ false, fragment, block, driver);
            ctx.setGrouping(std::move(new_));
            return {};
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid parameter initializer");
    }
}
