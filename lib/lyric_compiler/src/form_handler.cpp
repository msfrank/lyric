
#include <lyric_compiler/assembler_handler.h>
#include <lyric_compiler/assignment_handler.h>
#include <lyric_compiler/binary_operation_handler.h>
#include <lyric_compiler/block_handler.h>
#include <lyric_compiler/compiler_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/conditional_handler.h>
#include <lyric_compiler/constant_utils.h>
#include <lyric_compiler/def_handler.h>
#include <lyric_compiler/defalias_handler.h>
#include <lyric_compiler/defclass_handler.h>
#include <lyric_compiler/defconcept_handler.h>
#include <lyric_compiler/defenum_handler.h>
#include <lyric_compiler/definstance_handler.h>
#include <lyric_compiler/defstatic_handler.h>
#include <lyric_compiler/defstruct_handler.h>
#include <lyric_compiler/data_deref_handler.h>
#include <lyric_compiler/deref_utils.h>
#include <lyric_compiler/form_handler.h>
#include <lyric_compiler/import_handler.h>
#include <lyric_compiler/iteration_handler.h>
#include <lyric_compiler/lambda_handler.h>
#include <lyric_compiler/match_handler.h>
#include <lyric_compiler/new_handler.h>
#include <lyric_compiler/symbol_deref_handler.h>
#include <lyric_compiler/type_utils.h>
#include <lyric_compiler/unary_operation_handler.h>
#include <lyric_compiler/using_handler.h>
#include <lyric_compiler/variable_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>
#include <lyric_schema/ast_schema.h>
#include <lyric_schema/compiler_schema.h>

lyric_compiler::TerminalFormBehavior::TerminalFormBehavior(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : m_isSideEffect(isSideEffect),
      m_fragment(fragment),
      m_driver(driver),
      m_block(block)
{
    TU_ASSERT (m_fragment != nullptr);
    TU_ASSERT (m_driver != nullptr);
    TU_ASSERT (m_block != nullptr);
}

tempo_utils::Status
lyric_compiler::TerminalFormBehavior::enter(
    CompilerScanDriver *driver,
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    EnterContext &ctx)
{
    if (m_isSideEffect) {
        TU_LOG_INFO << "ignoring terminal side effect";
        return {};
    }

    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    TU_LOG_INFO << "enter TerminalFormBehavior@" << this << ": "
                << resource->getNsUrl() << "#" << resource->getName();

    switch (astId) {
        case lyric_schema::LyricAstId::Nil:
            return constant_nil(m_block, m_fragment, m_driver);
        case lyric_schema::LyricAstId::Undef:
            return constant_undef(m_block, m_fragment, m_driver);
        case lyric_schema::LyricAstId::True:
            return constant_true(m_block, m_fragment, m_driver);
        case lyric_schema::LyricAstId::False:
            return constant_false(m_block, m_fragment, m_driver);
        case lyric_schema::LyricAstId::Integer:
            return constant_integer(node, m_block, m_fragment, m_driver);
        case lyric_schema::LyricAstId::Float:
            return constant_float(node, m_block, m_fragment, m_driver);
        case lyric_schema::LyricAstId::Char:
            return constant_char(node, m_block, m_fragment, m_driver);
        case lyric_schema::LyricAstId::String:
            return constant_string(node, m_block, m_fragment, m_driver);
        case lyric_schema::LyricAstId::Url:
            return constant_url(node, m_block, m_fragment, m_driver);
        case lyric_schema::LyricAstId::This: {
            lyric_assembler::DataReference unusedRef;
            return deref_this(unusedRef, m_block, m_fragment, m_driver);
        }
        case lyric_schema::LyricAstId::Name: {
            lyric_assembler::BlockHandle *currentBlock = m_block;
            lyric_assembler::DataReference unusedRef;
            return deref_name(node, unusedRef, &currentBlock, m_fragment, m_driver);
        }
        case lyric_schema::LyricAstId::TypeOf:
            return load_type(node, m_fragment, m_block, m_driver);
        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "expected terminal node");
    }
}

tempo_utils::Status
lyric_compiler::TerminalFormBehavior::exit(
    CompilerScanDriver *driver,
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_assembler::BlockHandle *block,
    ExitContext &ctx)
{
    if (m_isSideEffect)
        return {};

    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    TU_LOG_INFO << "exit TerminalFormBehavior@" << this << ": "
                << resource->getNsUrl() << "#" << resource->getName();
    return {};
}

inline tempo_utils::Status
node_is_valid_for_phrase(
    lyric_schema::LyricAstId astId,
    lyric_compiler::FormType type,
    bool &isExpression)
{
    switch (astId) {

        // expression phrase
        case lyric_schema::LyricAstId::Nil:
        case lyric_schema::LyricAstId::Undef:
        case lyric_schema::LyricAstId::True:
        case lyric_schema::LyricAstId::False:
        case lyric_schema::LyricAstId::Integer:
        case lyric_schema::LyricAstId::Float:
        case lyric_schema::LyricAstId::Char:
        case lyric_schema::LyricAstId::String:
        case lyric_schema::LyricAstId::Url:
        case lyric_schema::LyricAstId::Name:
        case lyric_schema::LyricAstId::This:
        case lyric_schema::LyricAstId::Call:
        case lyric_schema::LyricAstId::IsEq:
        case lyric_schema::LyricAstId::IsLt:
        case lyric_schema::LyricAstId::IsLe:
        case lyric_schema::LyricAstId::IsGt:
        case lyric_schema::LyricAstId::IsGe:
        case lyric_schema::LyricAstId::TypeOf:
        case lyric_schema::LyricAstId::Add:
        case lyric_schema::LyricAstId::Sub:
        case lyric_schema::LyricAstId::Mul:
        case lyric_schema::LyricAstId::Div:
        case lyric_schema::LyricAstId::Neg:
        case lyric_schema::LyricAstId::And:
        case lyric_schema::LyricAstId::Or:
        case lyric_schema::LyricAstId::Not:
        case lyric_schema::LyricAstId::DataDeref:
        case lyric_schema::LyricAstId::SymbolDeref:
        case lyric_schema::LyricAstId::New:
        case lyric_schema::LyricAstId::Block:
        case lyric_schema::LyricAstId::Cond:
        case lyric_schema::LyricAstId::Match:
        case lyric_schema::LyricAstId::LambdaFrom:
        case lyric_schema::LyricAstId::Lambda: {
            switch (type) {
                case lyric_compiler::FormType::Any:
                case lyric_compiler::FormType::Expression:
                case lyric_compiler::FormType::SideEffect:
                    isExpression = true;
                    return {};
                default:
                    return lyric_compiler::CompilerStatus::forCondition(
                        lyric_compiler::CompilerCondition::kCompilerInvariant, "expected expression node");
            }
        }

        // statement phrase
        case lyric_schema::LyricAstId::Val:
        case lyric_schema::LyricAstId::Var:
        case lyric_schema::LyricAstId::Def:
        case lyric_schema::LyricAstId::DefAlias:
        case lyric_schema::LyricAstId::DefClass:
        case lyric_schema::LyricAstId::DefConcept:
        case lyric_schema::LyricAstId::DefEnum:
        case lyric_schema::LyricAstId::DefInstance:
        case lyric_schema::LyricAstId::DefStruct:
        case lyric_schema::LyricAstId::DefStatic:
        case lyric_schema::LyricAstId::Namespace:
        case lyric_schema::LyricAstId::Generic:
        case lyric_schema::LyricAstId::Init:
        case lyric_schema::LyricAstId::Set:
        case lyric_schema::LyricAstId::InplaceAdd:
        case lyric_schema::LyricAstId::InplaceSub:
        case lyric_schema::LyricAstId::InplaceMul:
        case lyric_schema::LyricAstId::InplaceDiv:
        case lyric_schema::LyricAstId::If:
        case lyric_schema::LyricAstId::While:
        case lyric_schema::LyricAstId::For:
        case lyric_schema::LyricAstId::Return:
        case lyric_schema::LyricAstId::ImportModule:
        case lyric_schema::LyricAstId::ImportSymbols:
        case lyric_schema::LyricAstId::ImportAll:
        case lyric_schema::LyricAstId::Using: {
            switch (type) {
                case lyric_compiler::FormType::Any:
                case lyric_compiler::FormType::SideEffect:
                case lyric_compiler::FormType::Statement:
                    isExpression = false;
                    return {};
                default:
                    return lyric_compiler::CompilerStatus::forCondition(
                        lyric_compiler::CompilerCondition::kCompilerInvariant, "expected statement node");
            }
        }

        default:
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kCompilerInvariant, "expected form node");
    }
}

lyric_compiler::FormChoice::FormChoice(
    FormType type,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_type(type),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::FormChoice::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    lyric_compiler::DecideContext &ctx)
{
    // assembler forms
    if (node->isNamespace(lyric_schema::kLyricAssemblerNs)) {
        auto choice = std::make_unique<AssemblerChoice>(m_fragment, getBlock(), getDriver());
        ctx.setChoice(std::move(choice));
        return {};
    }

    // compiler forms
    if (node->isNamespace(lyric_schema::kLyricCompilerNs)) {
        auto choice = std::make_unique<CompilerChoice>(m_fragment, getBlock(), getDriver());
        ctx.setChoice(std::move(choice));
        return {};
    }

    auto *block = getBlock();
    auto *driver = getDriver();

    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return block->logAndContinue(CompilerCondition::kCompilerInvariant,
            tempo_tracing::LogSeverity::kError, "invalid form");

    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());
    auto astId = resource->getId();

    TU_LOG_INFO << "decide FormChoice@" << this << ": "
                << resource->getNsUrl() << "#" << resource->getName();

    bool isSideEffect = m_type == FormType::SideEffect;

    bool isExpression;
    TU_RETURN_IF_NOT_OK (node_is_valid_for_phrase(astId, m_type, isExpression));

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
        case lyric_schema::LyricAstId::This:
        case lyric_schema::LyricAstId::Name:
        case lyric_schema::LyricAstId::TypeOf:
        {
            auto terminal = std::make_unique<TerminalFormBehavior>(
                isSideEffect, m_fragment, block, driver);
            ctx.setBehavior(std::move(terminal));
            break;
        }

        // new form
        case lyric_schema::LyricAstId::New: {
            auto new_ = std::make_unique<NewHandler>(
                isSideEffect, m_fragment, block, driver);
            ctx.setGrouping(std::move(new_));
            break;
        }

        // deref forms
        case lyric_schema::LyricAstId::DataDeref: {
            auto deref = std::make_unique<DataDerefHandler>(
                isSideEffect, m_fragment, block, driver);
            ctx.setGrouping(std::move(deref));
            break;
        }
        case lyric_schema::LyricAstId::SymbolDeref: {
            auto deref = std::make_unique<SymbolDerefHandler>(
                isSideEffect, m_fragment, block, driver);
            ctx.setGrouping(std::move(deref));
            break;
        }

        // assignment forms
        case lyric_schema::LyricAstId::Set:
        case lyric_schema::LyricAstId::InplaceAdd:
        case lyric_schema::LyricAstId::InplaceSub:
        case lyric_schema::LyricAstId::InplaceMul:
        case lyric_schema::LyricAstId::InplaceDiv: {
            auto assignment = std::make_unique<AssignmentHandler>(
                isSideEffect, m_fragment, block, driver);
            ctx.setGrouping(std::move(assignment));
            break;
        }

        // grouping form
        case lyric_schema::LyricAstId::Block: {
            bool requiresResult = m_type == FormType::Expression;
            auto groupBlock = std::make_unique<lyric_assembler::BlockHandle>(
                block->blockProc(), block, block->blockState());
            auto group = std::make_unique<BlockHandler>(
                std::move(groupBlock), requiresResult, isSideEffect, m_fragment, driver);
            ctx.setGrouping(std::move(group));
            break;
        }

        case lyric_schema::LyricAstId::LambdaFrom: {
            auto choice = std::make_unique<LambdaFrom>(
                isSideEffect, m_fragment, block, driver);
            ctx.setChoice(std::move(choice));
            break;
        }

        case lyric_schema::LyricAstId::Lambda: {
            auto handler = std::make_unique<LambdaHandler>(
                isSideEffect, m_fragment, block, driver);
            ctx.setGrouping(std::move(handler));
            break;
        }

        // if form
        case lyric_schema::LyricAstId::If: {
            auto handler = std::make_unique<IfHandler>(
                isSideEffect, m_fragment, block, driver);
            ctx.setGrouping(std::move(handler));
            break;
        }

        // cond form
        case lyric_schema::LyricAstId::Cond: {
            auto handler = std::make_unique<CondHandler>(
                isSideEffect, m_fragment, block, driver);
            ctx.setGrouping(std::move(handler));
            break;
        }

        // while form
        case lyric_schema::LyricAstId::While: {
            auto handler = std::make_unique<WhileHandler>(
                isSideEffect, m_fragment, block, driver);
            ctx.setGrouping(std::move(handler));
            break;
        }

        // for form
        case lyric_schema::LyricAstId::For: {
            auto handler = std::make_unique<ForHandler>(
                isSideEffect, m_fragment, block, driver);
            ctx.setGrouping(std::move(handler));
            break;
        }

        // match form
        case lyric_schema::LyricAstId::Match: {
            auto handler = std::make_unique<MatchHandler>(
                isSideEffect, m_fragment, block, driver);
            ctx.setGrouping(std::move(handler));
            break;
        }

        // unary operation forms
        case lyric_schema::LyricAstId::Neg:
        case lyric_schema::LyricAstId::Not: {
            auto unary = std::make_unique<UnaryOperationHandler>(
                astId, isSideEffect, m_fragment, block, driver);
            ctx.setGrouping(std::move(unary));
            break;
        }

        // binary operation forms
        case lyric_schema::LyricAstId::IsEq:
        case lyric_schema::LyricAstId::IsGe:
        case lyric_schema::LyricAstId::IsGt:
        case lyric_schema::LyricAstId::IsLe:
        case lyric_schema::LyricAstId::IsLt:
        case lyric_schema::LyricAstId::Add:
        case lyric_schema::LyricAstId::Sub:
        case lyric_schema::LyricAstId::Mul:
        case lyric_schema::LyricAstId::Div:
        case lyric_schema::LyricAstId::And:
        case lyric_schema::LyricAstId::Or: {
            auto binary = std::make_unique<BinaryOperationHandler>(
                astId, isSideEffect, m_fragment, block, driver);
            ctx.setGrouping(std::move(binary));
            break;
        }

        // variable definition forms
        case lyric_schema::LyricAstId::Val:
        case lyric_schema::LyricAstId::Var: {
            bool isVariable = astId == lyric_schema::LyricAstId::Var;
            auto variable = std::make_unique<VariableHandler>(
                isVariable, isSideEffect, m_fragment, block, driver);
            ctx.setGrouping(std::move(variable));
            break;
        }

        // function definition form
        case lyric_schema::LyricAstId::Def: {
            auto def = std::make_unique<DefHandler>(isSideEffect, block, driver);
            ctx.setGrouping(std::move(def));
            break;
        }

        // class definition form
        case lyric_schema::LyricAstId::DefClass: {
            auto def = std::make_unique<DefClassHandler>(isSideEffect, block, driver);
            ctx.setGrouping(std::move(def));
            break;
        }

        // concept definition form
        case lyric_schema::LyricAstId::DefConcept: {
            auto def = std::make_unique<DefConceptHandler>(isSideEffect, block, driver);
            ctx.setGrouping(std::move(def));
            break;
        }

        // concept definition form
        case lyric_schema::LyricAstId::DefEnum: {
            auto def = std::make_unique<DefEnumHandler>(isSideEffect, block, driver);
            ctx.setGrouping(std::move(def));
            break;
        }

        // concept definition form
        case lyric_schema::LyricAstId::DefInstance: {
            auto def = std::make_unique<DefInstanceHandler>(isSideEffect, block, driver);
            ctx.setGrouping(std::move(def));
            break;
        }

        // global definition form
        case lyric_schema::LyricAstId::DefStatic: {
            auto def = std::make_unique<DefStaticHandler>(isSideEffect, block, driver);
            ctx.setGrouping(std::move(def));
            break;
        }

        // struct definition form
        case lyric_schema::LyricAstId::DefStruct: {
            auto def = std::make_unique<DefStructHandler>(isSideEffect, block, driver);
            ctx.setGrouping(std::move(def));
            break;
        }

        // alias definition form
        case lyric_schema::LyricAstId::DefAlias: {
            auto def = std::make_unique<DefAliasHandler>(isSideEffect, block, driver);
            ctx.setGrouping(std::move(def));
            break;
        }

        // using statement form
        case lyric_schema::LyricAstId::Using: {
            auto def = std::make_unique<UsingHandler>(isSideEffect, block, driver);
            ctx.setGrouping(std::move(def));
            break;
        }

        case lyric_schema::LyricAstId::ImportAll:
        case lyric_schema::LyricAstId::ImportModule:
        case lyric_schema::LyricAstId::ImportSymbols: {
            auto import = std::make_unique<ImportHandler>(isSideEffect, block, driver);
            ctx.setGrouping(std::move(import));
            break;
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid form node");
    }

    return {};
}