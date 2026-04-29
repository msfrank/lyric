
#include <lyric_assembler/call_symbol.h>
#include <lyric_assembler/object_root.h>
#include <lyric_assembler/static_symbol.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/global_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/ast_schema.h>

#include "lyric_compiler/new_handler.h"

lyric_compiler::GlobalHandler::GlobalHandler(
    Global *global,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_global(global)
{
    TU_NOTNULL (m_global);
}

tempo_utils::Status
lyric_compiler::GlobalHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before GlobalHandler@" << this;

    auto *block = getBlock();
    auto *driver = getDriver();

    if (!node->isClass(lyric_schema::kLyricAstGlobalClass))
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "expected Global node");

    for (auto it = node->childrenBegin(); it != node->childrenEnd(); it++) {
        auto *child = *it;

        lyric_schema::LyricAstId astId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, astId));
        switch (astId) {
            case lyric_schema::LyricAstId::DefStatic: {
                auto member = m_global->members.at(child);
                auto handler = std::make_unique<GlobalMemberHandler>(member, block, driver);
                ctx.appendGrouping(std::move(handler));
                break;
            }
            default:
                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                    "unexpected AST node");
        }
    }

    return {};
}

tempo_utils::Status
lyric_compiler::GlobalHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    return {};
}

lyric_compiler::GlobalMemberHandler::GlobalMemberHandler(
    GlobalMember member,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_member(member)
{
    TU_ASSERT (m_member.staticSymbol != nullptr);
}

tempo_utils::Status
lyric_compiler::GlobalMemberHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    TU_LOG_VV << "before GlobalMemberHandler@" << this;

    // if there is no initializer then we are done
    if (node->numChildren() == 0)
        return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
            "invalid DefStatic node");

    auto *block = getBlock();
    auto *driver = getDriver();
    auto memberType = m_member.staticSymbol->getTypeDef();

    auto init = std::make_unique<GlobalMemberInit>(
        memberType, m_member.initializerHandle, block, driver);
    ctx.appendChoice(std::move(init));

    return {};
}

tempo_utils::Status
lyric_compiler::GlobalMemberHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    TU_LOG_VV << "after GlobalMemberHandler@" << this;

    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();
    auto *procHandle = m_member.initializerHandle->initializerProc();
    auto *fragment = procHandle->procFragment();

    auto initializerType = driver->peekResult();
    TU_RETURN_IF_NOT_OK (driver->popResult());
    procHandle->putExitType(initializerType);

    // add return instruction
    TU_RETURN_IF_NOT_OK (fragment->returnToCaller());

    // finalize the call
    lyric_common::TypeDef returnType;
    TU_ASSIGN_OR_RETURN (returnType, m_member.initializerHandle->finalizeInitializer());

    auto memberType = m_member.staticSymbol->getTypeDef();
    bool isReturnable;

    // validate that body returns the expected type
    TU_ASSIGN_OR_RETURN (isReturnable, typeSystem->isAssignable(memberType, returnType));
    if (!isReturnable)
        return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
            "member initializer is incompatible with type {}", memberType.toString());

    return {};
}

lyric_compiler::GlobalMemberInit::GlobalMemberInit(
    const lyric_common::TypeDef &memberType,
    lyric_assembler::InitializerHandle *initializerHandle,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_memberType(memberType),
      m_initializerHandle(initializerHandle)
{
    TU_ASSERT (m_memberType.isValid());
    TU_ASSERT (m_initializerHandle != nullptr);
}

tempo_utils::Status
lyric_compiler::GlobalMemberInit::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    TU_LOG_VV << "decide GlobalMemberInit@" << this << ": "
              << resource->getNsUrl() << "#" << resource->getName();

    auto *block = getBlock();
    auto *driver = getDriver();
    auto *procHandle = m_initializerHandle->initializerProc();
    auto *fragment = procHandle->procFragment();

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
        case lyric_schema::LyricAstId::Raw:
        case lyric_schema::LyricAstId::SymbolRef: {
            auto terminal = std::make_unique<TerminalFormBehavior>(
                /* isSideEffect */ false, fragment, block, driver);
            ctx.setBehavior(std::move(terminal));
            return {};
        }

        case lyric_schema::LyricAstId::New: {
            auto handler = std::make_unique<NewHandler>(
                m_memberType, /* isSideEffect */ false, fragment, block, driver);
            ctx.setGrouping(std::move(handler));
            return {};
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid global member initializer");
    }
}

tempo_utils::Status
lyric_compiler::declare_global_block(
    const lyric_parser::ArchetypeNode *node,
    Global *global,
    lyric_typing::TypeSystem *typeSystem)
{
    TU_NOTNULL (node);
    TU_NOTNULL (global);
    TU_NOTNULL (typeSystem);

    auto *definitionBlock = global->definitionBlock;

    for (auto it = node->childrenBegin(); it != node->childrenEnd(); ++it) {
        auto *child = *it;

        lyric_schema::LyricAstId astId;
        TU_RETURN_IF_NOT_OK (child->parseId(lyric_schema::kLyricAstVocabulary, astId));
        switch (astId) {

            case lyric_schema::LyricAstId::DefStatic: {
                std::string identifier;
                TU_RETURN_IF_NOT_OK (child->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

                lyric_parser::ArchetypeNode *typeNode;
                TU_RETURN_IF_NOT_OK (child->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
                lyric_typing::TypeSpec memberSpec;
                TU_ASSIGN_OR_RETURN (memberSpec, typeSystem->parseAssignable(definitionBlock, typeNode->getArchetypeNode()));
                lyric_common::TypeDef memberType;
                TU_ASSIGN_OR_RETURN (memberType, typeSystem->resolveAssignable(definitionBlock, memberSpec));

                bool isVariable;
                TU_RETURN_IF_NOT_OK (child->parseAttr(lyric_parser::kLyricAstIsVariable, isVariable));

                bool isHidden;
                TU_RETURN_IF_NOT_OK (child->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

                GlobalMember member;
                TU_ASSIGN_OR_RETURN (member.staticSymbol, definitionBlock->declareStatic(
                    identifier, isHidden, memberType, isVariable));
                TU_ASSIGN_OR_RETURN (member.initializerHandle, member.staticSymbol->defineInitializer());

                global->members[child] = member;
                return {};
            }

            case lyric_schema::LyricAstId::Def: {
                std::string identifier;
                TU_RETURN_IF_NOT_OK (child->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));

                bool isHidden;
                TU_RETURN_IF_NOT_OK (child->parseAttr(lyric_parser::kLyricAstIsHidden, isHidden));

                // if method is generic, then parse the template parameter list
                lyric_typing::TemplateSpec templateSpec;
                if (node->hasAttr(lyric_parser::kLyricAstGenericOffset)) {
                    lyric_parser::ArchetypeNode *genericNode;
                    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstGenericOffset, genericNode));
                    auto *rootBlock = definitionBlock->blockState()->objectRoot()->rootBlock();
                    TU_ASSIGN_OR_RETURN (templateSpec, typeSystem->parseTemplate(rootBlock, genericNode->getArchetypeNode()));
                }

                GlobalMethod method;
                TU_ASSIGN_OR_RETURN (method.callSymbol, definitionBlock->declareFunction(
                    identifier, isHidden, templateSpec.templateParameters));

                global->methods[child] = method;
                return {};
            }

            default:
                return CompilerStatus::forCondition(CompilerCondition::kSyntaxError,
                    "unexpected AST node");
        }
    }

    return {};
}
