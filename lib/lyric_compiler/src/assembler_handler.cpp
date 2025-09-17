
#include <lyric_assembler/assembler_attrs.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/object_plugin.h>
#include <lyric_assembler/symbol_cache.h>
#include <lyric_compiler/assembler_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/data_deref_handler.h>
#include <lyric_compiler/symbol_deref_handler.h>
#include <lyric_compiler/target_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>

#include "lyric_compiler/resolve_utils.h"

lyric_compiler::AssemblerChoice::AssemblerChoice(
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

class OpHandler : public lyric_compiler::BaseGrouping {
public:
    OpHandler(
        lyric_assembler::CodeFragment *fragment,
        lyric_assembler::BlockHandle *block,
        lyric_compiler::CompilerScanDriver *driver)
        : BaseGrouping(block, driver),
          m_fragment(fragment)
    {}

    tempo_utils::Status updateResultStack(int numOperands, const lyric_common::TypeDef &result = {})
    {
        auto *driver = getDriver();
        for (; 0 < numOperands; numOperands--) {
            TU_RETURN_IF_NOT_OK (driver->popResult());
        }
        if (result.isValid()) {
            TU_RETURN_IF_NOT_OK (driver->pushResult(result));
        }
        return {};
    }

    tempo_utils::Status before(
        const lyric_parser::ArchetypeState *state,
        const lyric_parser::ArchetypeNode *node,
        lyric_compiler::BeforeContext &ctx) override
    {
        lyric_object::Opcode opcode;
        TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_assembler::kLyricAssemblerOpcodeEnum, opcode));
        auto *name = lyric_object::opcode_to_name(opcode);

        bool hasStackOffset = node->hasAttr(lyric_assembler::kLyricAssemblerStackOffset);
        tu_uint16 stackOffset;
        if (hasStackOffset) {
            TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_assembler::kLyricAssemblerStackOffset, stackOffset));
        }

        auto *driver = getDriver();
        auto *fundamentalCache = driver->getFundamentalCache();
        auto BoolType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Bool);
        auto IntType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Int);
        auto FloatType = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::Float);

        switch (opcode) {
            case lyric_object::Opcode::OP_NOOP:
                return m_fragment->noOperation();

            // stack manipulation
            case lyric_object::Opcode::OP_POP:
                TU_RETURN_IF_NOT_OK (m_fragment->popValue());
                return driver->popResult();
            case lyric_object::Opcode::OP_DUP: {
                TU_RETURN_IF_NOT_OK (m_fragment->dupValue());
                auto peek = driver->peekResult();
                return driver->pushResult(peek);
            }
            case lyric_object::Opcode::OP_PICK:
            case lyric_object::Opcode::OP_DROP:
            case lyric_object::Opcode::OP_RPICK:
            case lyric_object::Opcode::OP_RDROP:

            // integer math
            case lyric_object::Opcode::OP_I64_ADD:
                TU_RETURN_IF_NOT_OK (m_fragment->intAdd());
                return updateResultStack(2, IntType);
            case lyric_object::Opcode::OP_I64_SUB:
                TU_RETURN_IF_NOT_OK (m_fragment->intSubtract());
                return updateResultStack(2, IntType);
            case lyric_object::Opcode::OP_I64_MUL:
                TU_RETURN_IF_NOT_OK (m_fragment->intMultiply());
                return updateResultStack(2, IntType);
            case lyric_object::Opcode::OP_I64_DIV:
                TU_RETURN_IF_NOT_OK (m_fragment->intDivide());
                return updateResultStack(2, IntType);
            case lyric_object::Opcode::OP_I64_NEG:
                TU_RETURN_IF_NOT_OK (m_fragment->intNegate());
                return updateResultStack(1, IntType);

            // rational math
            case lyric_object::Opcode::OP_DBL_ADD:
                TU_RETURN_IF_NOT_OK (m_fragment->floatAdd());
                return updateResultStack(2, FloatType);
            case lyric_object::Opcode::OP_DBL_SUB:
                TU_RETURN_IF_NOT_OK (m_fragment->floatSubtract());
                return updateResultStack(2, FloatType);
            case lyric_object::Opcode::OP_DBL_MUL:
                TU_RETURN_IF_NOT_OK (m_fragment->floatMultiply());
                return updateResultStack(2, FloatType);
            case lyric_object::Opcode::OP_DBL_DIV:
                TU_RETURN_IF_NOT_OK (m_fragment->floatDivide());
                return updateResultStack(2, FloatType);
            case lyric_object::Opcode::OP_DBL_NEG:
                TU_RETURN_IF_NOT_OK (m_fragment->floatNegate());
                return updateResultStack(1, FloatType);

            // comparisons
            case lyric_object::Opcode::OP_BOOL_CMP:
                TU_RETURN_IF_NOT_OK (m_fragment->boolCompare());
                return updateResultStack(2, BoolType);
            case lyric_object::Opcode::OP_I64_CMP:
                TU_RETURN_IF_NOT_OK (m_fragment->intCompare());
                return updateResultStack(2, BoolType);
            case lyric_object::Opcode::OP_DBL_CMP:
                TU_RETURN_IF_NOT_OK (m_fragment->floatCompare());
                return updateResultStack(2, BoolType);
            case lyric_object::Opcode::OP_CHR_CMP:
                TU_RETURN_IF_NOT_OK (m_fragment->charCompare());
                return updateResultStack(2, BoolType);
            case lyric_object::Opcode::OP_TYPE_CMP:
                TU_RETURN_IF_NOT_OK (m_fragment->typeCompare());
                return updateResultStack(2, BoolType);

            // logical operations
            case lyric_object::Opcode::OP_LOGICAL_AND:
                TU_RETURN_IF_NOT_OK (m_fragment->logicalAnd());
                return updateResultStack(2, BoolType);
            case lyric_object::Opcode::OP_LOGICAL_OR:
                TU_RETURN_IF_NOT_OK (m_fragment->logicalOr());
                return updateResultStack(2, BoolType);
            case lyric_object::Opcode::OP_LOGICAL_NOT:
                TU_RETURN_IF_NOT_OK (m_fragment->logicalNot());
                return updateResultStack(1, BoolType);

            // bitwise operations
            case lyric_object::Opcode::OP_BITWISE_AND:
                TU_RETURN_IF_NOT_OK (m_fragment->bitwiseAnd());
                return updateResultStack(2, IntType);
            case lyric_object::Opcode::OP_BITWISE_OR:
                TU_RETURN_IF_NOT_OK (m_fragment->bitwiseOr());
                return updateResultStack(2, IntType);
            case lyric_object::Opcode::OP_BITWISE_XOR:
                TU_RETURN_IF_NOT_OK (m_fragment->bitwiseXor());
                return updateResultStack(2, IntType);
            case lyric_object::Opcode::OP_BITWISE_RIGHT_SHIFT:
                TU_RETURN_IF_NOT_OK (m_fragment->bitwiseRightShift());
                return updateResultStack(2, IntType);
            case lyric_object::Opcode::OP_BITWISE_LEFT_SHIFT:
                TU_RETURN_IF_NOT_OK (m_fragment->bitwiseLeftShift());
                return updateResultStack(2, IntType);

            default: {
                if (name == nullptr)
                    return lyric_assembler::AssemblerStatus::forCondition(
                        lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                        "invalid opcode {}", static_cast<tu_uint32>(opcode));
                return lyric_assembler::AssemblerStatus::forCondition(
                    lyric_assembler::AssemblerCondition::kAssemblerInvariant,
                    "unsupported opcode {}", name);
            }
        }
    }

    tempo_utils::Status after(
        const lyric_parser::ArchetypeState *state,
        const lyric_parser::ArchetypeNode *node,
        lyric_compiler::AfterContext &ctx) override
    {
        return {};
    }

private:
    lyric_assembler::CodeFragment *m_fragment;
};

class LoadDataHandler : public lyric_compiler::BaseGrouping {
public:
    LoadDataHandler(
        lyric_assembler::CodeFragment *fragment,
        lyric_assembler::BlockHandle *block,
        lyric_compiler::CompilerScanDriver *driver)
        : BaseGrouping(block, driver),
          m_fragment(fragment)
    {}

    tempo_utils::Status before(
        const lyric_parser::ArchetypeState *state,
        const lyric_parser::ArchetypeNode *node,
        lyric_compiler::BeforeContext &ctx) override
    {
        auto numChildren = node->numChildren();
        if (numChildren != 1)
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kCompilerInvariant,
                "expected 1 argument to LoadData but encountered {}", numChildren);
        auto *child = node->getChild(0);

        if (!child->isNamespace(lyric_schema::kLyricAstNs))
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kCompilerInvariant, "invalid form");

        auto *resource = lyric_schema::kLyricAstVocabulary.getResource(child->getIdValue());
        auto astId = resource->getId();

        TU_LOG_VV << "before LoadData@" << this << ": "
            << resource->getNsUrl() << "#" << resource->getName();

        auto *block = getBlock();
        auto *driver = getDriver();

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
            {
                auto handler = std::make_unique<lyric_compiler::FormChoice>(
                    lyric_compiler::FormType::Expression, m_fragment, block, driver);
                ctx.appendChoice(std::move(handler));
                return {};
            }
            case lyric_schema::LyricAstId::DataDeref: {
                auto handler = std::make_unique<lyric_compiler::DataDerefHandler>(
                    /* isSideEffect= */ false, m_fragment, block, driver);
                ctx.appendGrouping(std::move(handler));
                return {};
            }
            case lyric_schema::LyricAstId::SymbolDeref: {
                auto handler = std::make_unique<lyric_compiler::SymbolDerefHandler>(
                    /* isSideEffect= */ false, m_fragment, block, driver);
                ctx.appendGrouping(std::move(handler));
                return {};
            }
            default:
                return lyric_compiler::CompilerStatus::forCondition(
                    lyric_compiler::CompilerCondition::kCompilerInvariant, "invalid AST node");
        }
    }

    tempo_utils::Status after(
        const lyric_parser::ArchetypeState *state,
        const lyric_parser::ArchetypeNode *node,
        lyric_compiler::AfterContext &ctx) override
    {
        return {};
    }

private:
    lyric_assembler::CodeFragment *m_fragment;
};

struct StoreData {
    lyric_compiler::Target target;
    lyric_common::TypeDef rvalueType;
    std::unique_ptr<lyric_assembler::CodeFragment> loadTemporary;
    lyric_assembler::CodeFragment *fragment = nullptr;
};

class StoreDataTarget : public lyric_compiler::BaseChoice {
public:
    StoreDataTarget(
        StoreData *storeData,
        lyric_assembler::BlockHandle *block,
        lyric_compiler::CompilerScanDriver *driver)
        : BaseChoice(block, driver),
          m_storeData(storeData)
    {
        TU_ASSERT (m_storeData != nullptr);
    }

    tempo_utils::Status decide(
        const lyric_parser::ArchetypeState *state,
        const lyric_parser::ArchetypeNode *node,
        lyric_compiler::DecideContext &ctx) override
    {
        if (!node->isNamespace(lyric_schema::kLyricAstNs))
            return {};
        auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

        auto *block = getBlock();
        auto *driver = getDriver();

        // pop rvalue type from the top of the results stack
        auto rvalueType = driver->peekResult();
        m_storeData->rvalueType = rvalueType;
        TU_RETURN_IF_NOT_OK (driver->popResult());

        auto astId = resource->getId();
        switch (astId) {

            case lyric_schema::LyricAstId::Target: {
                auto *fragment = m_storeData->fragment;
                auto &loadTemporary = m_storeData->loadTemporary;

                // store top of stack in temp variable
                lyric_assembler::DataReference tmpVar;
                TU_ASSIGN_OR_RETURN (tmpVar, block->declareTemporary(rvalueType, /* isVariable= */ false));
                TU_RETURN_IF_NOT_OK (fragment->storeRef(tmpVar, /* initialStore= */ true));

                // add code to load temp variable
                loadTemporary = fragment->makeFragment();
                TU_RETURN_IF_NOT_OK (loadTemporary->loadRef(tmpVar));

                // process the Target node
                auto target = std::make_unique<lyric_compiler::TargetHandler>(
                    &m_storeData->target, fragment, block, driver);
                ctx.setGrouping(std::move(target));
                return {};
            }

            case lyric_schema::LyricAstId::Name: {
                return resolve_name(node, block, m_storeData->target.targetRef, driver);
            }

            default:
                return lyric_compiler::CompilerStatus::forCondition(
                    lyric_compiler::CompilerCondition::kCompilerInvariant, "invalid target node");
        }
    }

private:
    StoreData *m_storeData;
};

/**
 *
 */
class StoreDataHandler : public lyric_compiler::BaseGrouping {
public:
    StoreDataHandler(
        lyric_assembler::CodeFragment *fragment,
        lyric_assembler::BlockHandle *block,
        lyric_compiler::CompilerScanDriver *driver)
        : BaseGrouping(block, driver)
    {
        m_storeData.fragment = fragment;
    }

    tempo_utils::Status before(
        const lyric_parser::ArchetypeState *state,
        const lyric_parser::ArchetypeNode *node,
        lyric_compiler::BeforeContext &ctx) override
    {
        auto *block = getBlock();
        auto *driver = getDriver();

        auto target = std::make_unique<StoreDataTarget>(&m_storeData, block, driver);
        ctx.appendChoice(std::move(target));

        return {};
    }

    tempo_utils::Status after(
        const lyric_parser::ArchetypeState *state,
        const lyric_parser::ArchetypeNode *node,
        lyric_compiler::AfterContext &ctx) override
    {
        TU_LOG_VV << "after AssignmentHandler@" << this;

        auto *block = BaseGrouping::getBlock();
        auto *driver = getDriver();
        auto *symbolCache = driver->getSymbolCache();
        auto *typeSystem = driver->getTypeSystem();

        // load temporary back onto stack if necessary
        if (m_storeData.loadTemporary) {
            TU_RETURN_IF_NOT_OK (m_storeData.fragment->appendFragment(std::move(m_storeData.loadTemporary)));
        }

        bool isAssignable;

        // check that the rhs is assignable to the target type
        TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(
            m_storeData.target.targetRef.typeDef, m_storeData.rvalueType));
        if (!isAssignable)
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kIncompatibleType,
                "target does not match rvalue type {}", m_storeData.rvalueType.toString());

        // check if we are in a constructor
        auto definition = block->getDefinition();
        lyric_assembler::AbstractSymbol *symbol;
        TU_ASSIGN_OR_RETURN (symbol, symbolCache->getOrImportSymbol(definition));
        auto *definitionCall = cast_symbol_to_call(symbol);
        bool initialStore = definitionCall->isCtor();

        // store expression result in target
        TU_RETURN_IF_NOT_OK (m_storeData.fragment->storeRef(m_storeData.target.targetRef, initialStore));

        return {};
    }

private:
    StoreData m_storeData;
};

tempo_utils::Status
lyric_compiler::AssemblerChoice::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    if (!node->isNamespace(lyric_schema::kLyricAssemblerNs))
        return {};
    auto *resource = lyric_schema::kLyricAssemblerVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    TU_LOG_VV << "decide AssemblerChoice@" << this << ": "
                << resource->getNsUrl() << "#" << resource->getName();

    switch (astId) {

        case lyric_schema::LyricAssemblerId::Op: {
            auto handler = std::make_unique<OpHandler>(m_fragment, getBlock(), getDriver());
            ctx.setGrouping(std::move(handler));
            return {};
        }

        case lyric_schema::LyricAssemblerId::Trap: {
            auto *objectPlugin = getDriver()->getState()->objectPlugin();
            if (objectPlugin == nullptr)
                return CompilerStatus::forCondition(
                    CompilerCondition::kCompilerInvariant, "invalid object plugin");
            auto pluginLocation = objectPlugin->getLocation();
            std::string trapName;
            TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_assembler::kLyricAssemblerTrapName, trapName));
            TU_RETURN_IF_NOT_OK (m_fragment->trap(pluginLocation, trapName, /* flags= */ 0));
            return {};
        }

        case lyric_schema::LyricAssemblerId::LoadData: {
            auto handler = std::make_unique<LoadDataHandler>(m_fragment, getBlock(), getDriver());
            ctx.setGrouping(std::move(handler));
            return {};
        }

        case lyric_schema::LyricAssemblerId::StoreData: {
            auto handler = std::make_unique<StoreDataHandler>(m_fragment, getBlock(), getDriver());
            ctx.setGrouping(std::move(handler));
            return {};
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid assembler node");
    }
}