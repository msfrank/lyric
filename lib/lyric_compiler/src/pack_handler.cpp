
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

lyric_compiler::PackHandler::PackHandler(
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_callSymbol(nullptr)
{
}

tempo_utils::Status
lyric_compiler::PackHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    for (int i = 0; i < node->numChildren(); i++) {
        std::unique_ptr<PackParam> param;
        if (m_callSymbol) {
            param = std::make_unique<PackParam>(m_callSymbol, block, driver);
        } else {
            param = std::make_unique<PackParam>(block, driver);
        }
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

lyric_compiler::PackParam::PackParam(
    lyric_assembler::BlockHandle *block,
    lyric_compiler::CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_callSymbol(nullptr)
{
}

tempo_utils::Status
lyric_compiler::PackParam::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();

    bool hasDefault = node->hasAttr(lyric_parser::kLyricAstDefaultOffset);

    // check if decl param has initializer
    if (m_callSymbol == nullptr && hasDefault)
        return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
            "unexpected param initializer");

    // otherwise if initializer was not specified then we are done
    if (!hasDefault)
        return {};

    std::string identifier;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

    // define the initializer
    TU_ASSIGN_OR_RETURN (m_initializerHandle, m_callSymbol->defineInitializer(identifier));

    // find the parameter
    m_param = m_callSymbol->getParameter(identifier);

    auto init = std::make_unique<ParamInit>(m_param.typeDef, m_initializerHandle, block, driver);
    ctx.appendChoice(std::move(init));

    return {};
}

tempo_utils::Status
lyric_compiler::PackParam::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    if (m_initializerHandle == nullptr)
        return {};

    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();
    auto *procHandle = m_initializerHandle->initializerProc();
    auto *code = procHandle->procCode();
    auto *fragment = code->rootFragment();

    auto initializerType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());
    procHandle->putExitType(initializerType);

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    // finialize the call
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, m_initializerHandle->finalizeInitializer());

    auto paramType = m_param.typeDef;
    bool isAssignable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(paramType, initializerType));
    if (!isAssignable)
        return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
            "parameter initializer is incompatible with type {}", paramType.toString());

    return {};
}

lyric_compiler::ParamInit::ParamInit(
    const lyric_common::TypeDef &paramType,
    lyric_assembler::InitializerHandle *initializerHandle,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_paramType(paramType),
      m_initializerHandle(initializerHandle)
{
    TU_ASSERT (m_initializerHandle != nullptr);
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

    auto *procHandle = m_initializerHandle->initializerProc();
    auto *fragment = procHandle->procCode()->rootFragment();

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