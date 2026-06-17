
#include <boost/multiprecision/integer.hpp>

#include <lyric_assembler/fundamental_cache.h>
#include <lyric_common/parse_numeric.h>
#include <lyric_compiler/cast_handler.h>
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/form_handler.h>
#include <lyric_parser/ast_attrs.h>

lyric_compiler::CastHandler::CastHandler(
    bool isSideEffect,
    lyric_assembler::CodeFragment *fragment,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
    : BaseGrouping(block, driver),
      m_isSideEffect(isSideEffect)
{
    m_cast.fragment = fragment;
    TU_ASSERT (m_cast.fragment != nullptr);
}

tempo_utils::Status
lyric_compiler::CastHandler::before(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    BeforeContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    lyric_parser::ArchetypeNode *typeNode;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstTypeOffset, typeNode));
    lyric_typing::TypeSpec castSpec;
    TU_ASSIGN_OR_RETURN (castSpec, typeSystem->parseAssignable(block, typeNode->getArchetypeNode()));
    TU_ASSIGN_OR_RETURN (m_cast.castType, typeSystem->resolveAssignable(block, castSpec));

    auto target = std::make_unique<CastTarget>(&m_cast, block, driver);
    ctx.appendChoice(std::move(target));
    return {};
}

tempo_utils::Status
lyric_compiler::CastHandler::after(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    AfterContext &ctx)
{
    auto *driver = getDriver();
    auto *typeSystem = driver->getTypeSystem();

    // get the target type of the cast
    auto targetType = driver->peekResult();

    lyric_runtime::TypeComparison cmp;
    TU_ASSIGN_OR_RETURN (cmp, typeSystem->compareAssignable(m_cast.castType, targetType));
    switch (cmp) {
        case lyric_runtime::TypeComparison::EQUAL:
        case lyric_runtime::TypeComparison::EXTENDS:
            TU_RETURN_IF_NOT_OK (driver->popResult());
            TU_RETURN_IF_NOT_OK (driver->pushResult(m_cast.castType));
            break;
        case lyric_runtime::TypeComparison::DISJOINT:
        case lyric_runtime::TypeComparison::SUPER:
            return CompilerStatus::forCondition(CompilerCondition::kIncompatibleType,
                "cannot cast {} to {}", targetType.toString(), m_cast.castType.toString());
    }

    // if expression is a side effect then pop the result off of the stack
    if (m_isSideEffect) {
        TU_RETURN_IF_NOT_OK (m_cast.fragment->popValue());
        TU_RETURN_IF_NOT_OK (driver->popResult());
    }

    return {};
}

lyric_compiler::CastTarget::CastTarget(
    Cast *cast,
    lyric_assembler::BlockHandle *block,
    CompilerScanDriver *driver)
        : BaseChoice(block, driver),
          m_cast(cast)
{
    TU_NOTNULL (m_cast);
}

tempo_utils::Result<bool>
cast_to_integer(
    const lyric_assembler::FundamentalCache *fundamentalCache,
    const lyric_parser::ArchetypeNode *node,
    const lyric_common::TypeDef &castType,
    lyric_assembler::CodeFragment *fragment)
{
    std::string literalValue;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));
    lyric_common::NumericBase base;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstBaseType, base));

    auto I64Type = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::I64);
    if (castType == I64Type) {
        tu_int64 i64;
        TU_ASSIGN_OR_RETURN (i64, lyric_common::parse_I64(literalValue, base));
        TU_RETURN_IF_NOT_OK (fragment->immediateI64(i64));
        return true;
    }

    auto I32Type = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::I32);
    if (castType == I32Type) {
        tu_int32 i32;
        TU_ASSIGN_OR_RETURN (i32, lyric_common::parse_I32(literalValue, base));
        TU_RETURN_IF_NOT_OK (fragment->immediateI32(i32));
        return true;
    }

    auto I16Type = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::I16);
    if (castType == I16Type) {
        tu_int16 i16;
        TU_ASSIGN_OR_RETURN (i16, lyric_common::parse_I16(literalValue, base));
        TU_RETURN_IF_NOT_OK (fragment->immediateI16(i16));
        return true;
    }

    auto I8Type = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::I8);
    if (castType == I8Type) {
        tu_int8 i8;
        TU_ASSIGN_OR_RETURN (i8, lyric_common::parse_I8(literalValue, base));
        TU_RETURN_IF_NOT_OK (fragment->immediateI8(i8));
        return true;
    }

    auto U64Type = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::U64);
    if (castType == U64Type) {
        tu_uint64 u64;
        TU_ASSIGN_OR_RETURN (u64, lyric_common::parse_U64(literalValue, base));
        TU_RETURN_IF_NOT_OK (fragment->immediateU64(u64));
        return true;
    }

    auto U32Type = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::U32);
    if (castType == U32Type) {
        tu_uint32 u32;
        TU_ASSIGN_OR_RETURN (u32, lyric_common::parse_U32(literalValue, base));
        TU_RETURN_IF_NOT_OK (fragment->immediateU32(u32));
        return true;
    }

    auto U16Type = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::U16);
    if (castType == U16Type) {
        tu_uint16 u16;
        TU_ASSIGN_OR_RETURN (u16, lyric_common::parse_U16(literalValue, base));
        TU_RETURN_IF_NOT_OK (fragment->immediateU16(u16));
        return true;
    }

    auto U8Type = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::U8);
    if (castType == U8Type) {
        tu_uint8 u8;
        TU_ASSIGN_OR_RETURN (u8, lyric_common::parse_U64(literalValue, base));
        TU_RETURN_IF_NOT_OK (fragment->immediateU8(u8));
        return true;
    }

    auto F64Type = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::F64);
    if (castType == F64Type) {
        double f64;
        TU_ASSIGN_OR_RETURN (f64, lyric_common::parse_F64(literalValue, base, /* scientific= */ false));
        TU_RETURN_IF_NOT_OK (fragment->immediateF64(f64));
        return true;
    }

    auto F32Type = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::F32);
    if (castType == F32Type) {
        float f32;
        TU_ASSIGN_OR_RETURN (f32, lyric_common::parse_F32(literalValue, base, /* scientific= */ false));
        TU_RETURN_IF_NOT_OK (fragment->immediateF32(f32));
        return true;
    }

    return false;
}

tempo_utils::Result<bool>
cast_to_float(
    const lyric_assembler::FundamentalCache *fundamentalCache,
    const lyric_parser::ArchetypeNode *node,
    const lyric_common::TypeDef &castType,
    lyric_assembler::CodeFragment *fragment)
{
    std::string literalValue;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstLiteralValue, literalValue));
    lyric_common::NumericBase base;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstBaseType, base));
    bool scientific;
    TU_RETURN_IF_NOT_OK (node->parseAttr(lyric_parser::kLyricAstIsScientific, scientific));

    auto F64Type = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::F64);
    if (castType == F64Type) {
        double f64;
        TU_ASSIGN_OR_RETURN (f64, lyric_common::parse_F64(literalValue, base, scientific));
        TU_RETURN_IF_NOT_OK (fragment->immediateF64(f64));
        return true;
    }

    auto F32Type = fundamentalCache->getFundamentalType(lyric_assembler::FundamentalSymbol::F32);
    if (castType == F32Type) {
        float f32;
        TU_ASSIGN_OR_RETURN (f32, lyric_common::parse_F32(literalValue, base, scientific));
        TU_RETURN_IF_NOT_OK (fragment->immediateF32(f32));
        return true;
    }

    return false;
}

tempo_utils::Status
lyric_compiler::CastTarget::decide(
    const lyric_parser::ArchetypeState *state,
    const lyric_parser::ArchetypeNode *node,
    DecideContext &ctx)
{
    auto *block = getBlock();
    auto *driver = getDriver();
    auto *fundamentalCache = driver->getFundamentalCache();


    if (!node->isNamespace(lyric_schema::kLyricAstNs))
        return {};
    auto *resource = lyric_schema::kLyricAstVocabulary.getResource(node->getIdValue());

    auto astId = resource->getId();
    switch (astId) {
        case lyric_schema::LyricAstId::Integer: {
            TU_ASSIGN_OR_RETURN (m_cast->isImmediateConversion, cast_to_integer(
                fundamentalCache, node, m_cast->castType, m_cast->fragment));
            break;
        }
        case lyric_schema::LyricAstId::Float: {
            TU_ASSIGN_OR_RETURN (m_cast->isImmediateConversion, cast_to_float(
                fundamentalCache, node, m_cast->castType, m_cast->fragment));
            break;
        }
        default:
            break;
    }

    // if we were able to rewrite the literal then push the type and return
    if (m_cast->isImmediateConversion) {
        return driver->pushResult(m_cast->castType);
    }

    // otherwise we process the target form
    auto expr = std::make_unique<FormChoice>(FormType::Expression, m_cast->fragment, block, driver);
    ctx.setChoice(std::move(expr));
    return {};
}