
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/constructor_handler.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/pack_handler.h>
#include <lyric_compiler/proc_handler.h>
#include <lyric_parser/ast_attrs.h>

#include "lyric_assembler/class_symbol.h"
#include "lyric_assembler/enum_symbol.h"
#include "lyric_assembler/instance_symbol.h"
#include "lyric_assembler/struct_symbol.h"

lyric_compiler::ConstructorHandler::ConstructorHandler(
    Constructor constructor,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_constructor(std::move(constructor))
{
    TU_ASSERT (m_constructor.superSymbol != nullptr);
    TU_ASSERT (m_constructor.callSymbol != nullptr);
    TU_ASSERT (m_constructor.procHandle != nullptr);
}

tempo_utils::Status
lyric_compiler::ConstructorHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before ConstructorHandler@" << this;

    auto *classBlock = getBlock();
    auto *driver = getDriver();
    auto *ctorBlock = m_constructor.procHandle->procBlock();
    auto *procBuilder = m_constructor.procHandle->procCode();
    auto *fragment = procBuilder->rootFragment();

    auto pack = std::make_unique<PackHandler>(m_constructor.callSymbol, classBlock, driver);
    ctx.appendGrouping(std::move(pack));

    auto super = std::make_unique<InitBase>(&m_constructor, fragment, ctorBlock, driver);
    ctx.appendGrouping(std::move(super));

    auto proc = std::make_unique<ProcHandler>(
        m_constructor.procHandle, /* requiresResult= */ false, ctorBlock, getDriver());
    ctx.appendGrouping(std::move(proc));

    return {};
}

tempo_utils::Status
lyric_compiler::ConstructorHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after ConstructorHandler@" << this;

    auto *procBuilder = m_constructor.procHandle->procCode();
    auto *fragment = procBuilder->rootFragment();

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    // finalize the call
    TU_RETURN_IF_STATUS (m_constructor.callSymbol->finalizeCall());

    return {};
}

lyric_compiler::InitBase::InitBase(
    Constructor *constructor,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseInvokableHandler(block, block, fragment, driver),
      m_constructor(constructor)
{
    TU_ASSERT (m_constructor != nullptr);
}

tempo_utils::Status
lyric_compiler::InitBase::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *fragment = getFragment();

    std::string ctorName;
    if (node->hasAttr(lyric_parser::kLyricAstIdentifier)) {
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIdentifier, ctorName));
    } else {
        ctorName = lyric_object::kCtorSpecialSymbol;
    }

    bool thisBase;
    if (node->hasAttr(lyric_parser::kLyricAstThisBase)) {
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstThisBase, thisBase));
    } else {
        thisBase = false;
    }

    m_invoker = std::make_unique<lyric_assembler::ConstructableInvoker>();

    switch (m_constructor->superSymbol->getSymbolType()) {
        case lyric_assembler::SymbolType::CLASS: {
            lyric_assembler::ClassSymbol *baseClass;
            if (thisBase) {
                baseClass = lyric_assembler::cast_symbol_to_class(m_constructor->thisSymbol);
            } else {
                baseClass = lyric_assembler::cast_symbol_to_class(m_constructor->superSymbol);
            }
            TU_RETURN_IF_NOT_OK (baseClass->prepareCtor(ctorName, *m_invoker));
            break;
        }
        case lyric_assembler::SymbolType::ENUM: {
            lyric_assembler::EnumSymbol *baseEnum;
            if (thisBase) {
                baseEnum = lyric_assembler::cast_symbol_to_enum(m_constructor->thisSymbol);
            } else {
                baseEnum = lyric_assembler::cast_symbol_to_enum(m_constructor->superSymbol);
            }
            TU_RETURN_IF_NOT_OK (baseEnum->prepareCtor(*m_invoker));
            break;
        }
        case lyric_assembler::SymbolType::INSTANCE: {
            lyric_assembler::InstanceSymbol *baseInstance;
            if (thisBase) {
                baseInstance = lyric_assembler::cast_symbol_to_instance(m_constructor->thisSymbol);
            } else {
                baseInstance = lyric_assembler::cast_symbol_to_instance(m_constructor->superSymbol);
            }
            TU_RETURN_IF_NOT_OK (baseInstance->prepareCtor(*m_invoker));
            break;
        }
        case lyric_assembler::SymbolType::STRUCT: {
            lyric_assembler::StructSymbol *baseStruct;
            if (thisBase) {
                baseStruct = lyric_assembler::cast_symbol_to_struct(m_constructor->thisSymbol);
            } else {
                baseStruct = lyric_assembler::cast_symbol_to_struct(m_constructor->superSymbol);
            }
            TU_RETURN_IF_NOT_OK (baseStruct->prepareCtor(ctorName, *m_invoker));
            break;
        }
        default:
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "invalid super symbol");
    }

    // load the uninitialized receiver onto the top of the stack
    TU_RETURN_IF_NOT_OK (fragment->loadThis());

    return BaseInvokableHandler::before(state, node, ctx);
}

tempo_utils::Status
lyric_compiler::InitBase::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();
    auto *fragment = getFragment();

    lyric_typing::CallsiteReifier reifier(typeSystem);

    TU_RETURN_IF_NOT_OK (reifier.initialize(*m_invoker));

    auto *constructable = m_invoker->getConstructable();
    TU_RETURN_IF_NOT_OK (placeArguments(constructable, reifier, fragment));

    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, m_invoker->invoke(getInvokeBlock(), reifier, fragment, /* flags= */ 0));

    return driver->pushResult(returnType);
}