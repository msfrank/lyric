
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/new_handler.h>
#include <lyric_compiler/pack_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>
#include "lyric_compiler/form_handler.h"

lyric_compiler::PackHandler::PackHandler(
    lyric_assembler::CallSymbol *callSymbol,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_callSymbol(callSymbol)
{
    TU_ASSERT (m_callSymbol != nullptr);
}

tempo_utils::Status
lyric_compiler::PackHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    //
    for (int i = 0; i < node->numChildren(); i++) {
        auto param = std::make_unique<PackParam>(m_callSymbol, block, driver);
        ctx.appendGrouping(std::move(param));
    }

    return {};
}

lyric_compiler::PackParam::PackParam(
    lyric_assembler::CallSymbol *callSymbol,
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_callSymbol(callSymbol)
{
    TU_ASSERT (m_callSymbol != nullptr);
}

tempo_utils::Status
lyric_compiler::PackParam::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
{
    // if initializer was not specified then we are done
    if (!node->hasAttr(lyric_parser::kLyricAstDefaultOffset))
        return {};

    auto *block = getBlock();
    auto *driver = getDriver();

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // define the initializer
    TU_ASSIGN_OR_RETURN (m_procHandle, m_callSymbol->defineInitializer(identifier));

    // find the parameter
    m_param = m_callSymbol->getParameter(identifier);

    auto init = std::make_unique<ParamInit>(m_param.typeDef, m_procHandle, block, driver);
    ctx.appendChoice(std::move(init));

    return {};
}

tempo_utils::Status
lyric_compiler::PackParam::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::AfterContext &ctx)
{
    if (m_procHandle == nullptr)
        return {};

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();
    auto *code = m_procHandle->procCode();
    auto *fragment = code->rootFragment();

    auto initializerType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());
    m_procHandle->putExitType(initializerType);

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    auto paramType = m_param.typeDef;
    bool isAssignable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(paramType, initializerType));
    if (!isAssignable)
        return block->logAndContinue(CompilerCondition::kIncompatibleType,
            tempo_tracing::LogSeverity::kError,
            "parameter initializer is incompatible with type {}", paramType.toString());

    // validate that each exit returns the expected type
    for (auto it = m_procHandle->exitTypesBegin(); it != m_procHandle->exitTypesEnd(); it++) {
        TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(paramType, *it));
        if (!isAssignable)
            return block->logAndContinue(CompilerCondition::kIncompatibleType,
                tempo_tracing::LogSeverity::kError,
                "parameter initializer is incompatible with type {}", paramType.toString());
    }

    return {};
}

lyric_compiler::ParamInit::ParamInit(
    const lyric_common::TypeDef &paramType,
    lyric_assembler::ProcHandle *procHandle,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_paramType(paramType),
      m_procHandle(procHandle)
{
    TU_ASSERT (m_procHandle != nullptr);
}

tempo_utils::Status
lyric_compiler::ParamInit::decide(
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
                m_paramType, /* isSideEffect */ false, fragment, block, driver);
            ctx.setGrouping(std::move(new_));
            return {};
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid parameter initializer");
    }
}