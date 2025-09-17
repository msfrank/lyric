
#include <lyric_assembler/assembler_attrs.h>
#include <lyric_assembler/fundamental_cache.h>
#include <lyric_assembler/object_plugin.h>
#include <lyric_compiler/assembler_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/data_deref_handler.h>
#include <lyric_compiler/symbol_deref_handler.h>
#include <lyric_parser/ast_attrs.h>
#include <lyric_schema/assembler_schema.h>

lyric_compiler::AssemblerChoice::AssemblerChoice(
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseChoice(block, driver),
      m_fragment(fragment)
{
    TU_ASSERT (m_fragment != nullptr);
}

class Op : public lyric_compiler::BaseGrouping {
public:
    Op(
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

class LoadData : public lyric_compiler::BaseGrouping {
public:
    LoadData(
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

/**
 *
 */
class StoreData : public lyric_compiler::BaseGrouping {
public:
    StoreData(
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
        ctx.setSkipChildren(true);

        auto numChildren = node->numChildren();
        if (numChildren != 1)
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kCompilerInvariant,
                "expected 1 argument to StoreData but encountered {}", numChildren);
        auto *child = node->getChild(0);

        if (!child->isNamespace(lyric_schema::kLyricAstNs))
            return lyric_compiler::CompilerStatus::forCondition(
                lyric_compiler::CompilerCondition::kCompilerInvariant, "invalid form");

        auto *resource = lyric_schema::kLyricAstVocabulary.getResource(child->getIdValue());
        auto astId = resource->getId();

        TU_LOG_VV << "before StoreData@" << this << ": "
            << resource->getNsUrl() << "#" << resource->getName();

        auto *block = getBlock();
        auto *driver = getDriver();

        switch (astId) {
            case lyric_schema::LyricAstId::Name:
            {
                std::string identifier;
                TU_RETURN_IF_NOT_OK (child->parseAttr(lyric_parser::kLyricAstIdentifier, identifier));
                lyric_assembler::DataReference ref;
                TU_ASSIGN_OR_RETURN (ref, block->resolveReference(identifier));
                auto resultType = driver->peekResult();

                // check that the result type is assignable to the ref type
                auto *typeSystem = driver->getTypeSystem();
                bool isAssignable;
                TU_ASSIGN_OR_RETURN (isAssignable, typeSystem->isAssignable(ref.typeDef, resultType));
                if (!isAssignable)
                    return lyric_compiler::CompilerStatus::forCondition(
                        lyric_compiler::CompilerCondition::kIncompatibleType,
                        "StoreData target {} is incompatible with result type {}",
                        identifier, resultType.toString());

                // store result in ref
                TU_RETURN_IF_NOT_OK (m_fragment->storeRef(ref, /* initialStore= */ true));

                return {};
            }
            default:
                return lyric_compiler::CompilerStatus::forCondition(
                    lyric_compiler::CompilerCondition::kCompilerInvariant, "invalid AST node");
        }
    }
private:
    lyric_assembler::CodeFragment *m_fragment;
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
            auto handler = std::make_unique<LoadData>(m_fragment, getBlock(), getDriver());
            ctx.setGrouping(std::move(handler));
            return {};
        }

        case lyric_schema::LyricAssemblerId::StoreData: {
            auto handler = std::make_unique<StoreData>(m_fragment, getBlock(), getDriver());
            ctx.setGrouping(std::move(handler));
            return {};
        }

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "invalid assembler node");
    }
}