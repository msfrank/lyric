
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>

#include "lyric_assembler/symbol_cache.h"

lyric_object::DeriveType
lyric_compiler::convert_derive_type(lyric_parser::DeriveType derive)
{
    switch (derive) {
        case lyric_parser::DeriveType::Any:
            return lyric_object::DeriveType::Any;
        case lyric_parser::DeriveType::Sealed:
            return lyric_object::DeriveType::Sealed;
        case lyric_parser::DeriveType::Final:
            return lyric_object::DeriveType::Final;
        default:
            return lyric_object::DeriveType::Invalid;
    }
}

tempo_utils::Result<std::string>
lyric_compiler::resolve_operator_action_name(lyric_schema::LyricAstId operatorClass)
{
    switch (operatorClass) {
        case lyric_schema::LyricAstId::Add:
            return {"Add"};
        case lyric_schema::LyricAstId::Sub:
            return {"Subtract"};
        case lyric_schema::LyricAstId::Mul:
            return {"Multiply"};
        case lyric_schema::LyricAstId::Div:
            return {"Divide"};
        case lyric_schema::LyricAstId::Neg:
            return {"Negate"};
        case lyric_schema::LyricAstId::And:
            return {"Conjunct"};
        case lyric_schema::LyricAstId::Or:
            return {"Disjunct"};
        case lyric_schema::LyricAstId::Not:
            return {"Complement"};
        case lyric_schema::LyricAstId::IsEq:
            return {"Equals"};
        case lyric_schema::LyricAstId::IsLt:
            return {"LessThan"};
        case lyric_schema::LyricAstId::IsLe:
            return {"LessEquals"};
        case lyric_schema::LyricAstId::IsGt:
            return {"GreaterThan"};
        case lyric_schema::LyricAstId::IsGe:
            return {"GreaterEquals"};
        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "failed to resolve operator action; invalid operator class");
    }
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::resolve_unary_operator_concept_type(
    const lyric_assembler::FundamentalCache *fundamentalCache,
    lyric_schema::LyricAstId operatorClass,
    const lyric_common::TypeDef &operand1)
{
    lyric_common::SymbolUrl conceptUrl;

    switch (operatorClass) {
        case lyric_schema::LyricAstId::Neg:
            return lyric_common::TypeDef::forConcrete(
                fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Arithmetic),
                {operand1, operand1});

        case lyric_schema::LyricAstId::Not:
            return lyric_common::TypeDef::forConcrete(
                fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Proposition),
                {operand1, operand1});

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "failed to resolve operator concept; invalid operator class");
    }
}

tempo_utils::Result<lyric_common::TypeDef>
lyric_compiler::resolve_binary_operator_concept_type(
    const lyric_assembler::FundamentalCache *fundamentalCache,
    lyric_schema::LyricAstId operatorClass,
    const lyric_common::TypeDef &operand1,
    const lyric_common::TypeDef &operand2)
{
    lyric_common::SymbolUrl conceptUrl;

    switch (operatorClass) {
        case lyric_schema::LyricAstId::Add:
        case lyric_schema::LyricAstId::Sub:
        case lyric_schema::LyricAstId::Mul:
        case lyric_schema::LyricAstId::Div:
            return lyric_common::TypeDef::forConcrete(
                fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Arithmetic),
                {operand1, operand2});

        case lyric_schema::LyricAstId::And:
        case lyric_schema::LyricAstId::Or:
            return lyric_common::TypeDef::forConcrete(
                fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Proposition),
                {operand1, operand2});

        case lyric_schema::LyricAstId::IsEq:
            return lyric_common::TypeDef::forConcrete(
                fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Equality),
                {operand1, operand2});

        case lyric_schema::LyricAstId::IsLt:
        case lyric_schema::LyricAstId::IsLe:
        case lyric_schema::LyricAstId::IsGt:
        case lyric_schema::LyricAstId::IsGe:
            return lyric_common::TypeDef::forConcrete(
                fundamentalCache->getFundamentalUrl(lyric_assembler::FundamentalSymbol::Comparison),
                {operand1, operand2});

        default:
            return CompilerStatus::forCondition(
                CompilerCondition::kCompilerInvariant, "failed to resolve operator concept; invalid operator class");
    }
}

static tempo_utils::Result<lyric_assembler::ActionSymbol *>
resolve_action(
    lyric_assembler::FundamentalSymbol symbol,
    std::string_view actionName,
    lyric_assembler::FundamentalCache *fundamentalCache,
    lyric_assembler::SymbolCache *symbolCache)
{
    auto conceptUrl = fundamentalCache->getFundamentalUrl(symbol);
    auto conceptPath = conceptUrl.getSymbolPath();
    lyric_common::SymbolPath actionPath(conceptPath.getPath(), actionName);
    lyric_common::SymbolUrl actionUrl(conceptUrl.getModuleLocation(), actionPath);
    return symbolCache->getOrImportAction(actionUrl);
}

tempo_utils::Result<lyric_assembler::ActionSymbol *>
lyric_compiler::resolve_operator_action(
    lyric_schema::LyricAstId operatorClass,
    lyric_assembler::FundamentalCache *fundamentalCache,
    lyric_assembler::SymbolCache *symbolCache)
{
    switch (operatorClass) {
        case lyric_schema::LyricAstId::Add:
            return resolve_action(lyric_assembler::FundamentalSymbol::Arithmetic, "Add", fundamentalCache, symbolCache);
        case lyric_schema::LyricAstId::Sub:
            return resolve_action(lyric_assembler::FundamentalSymbol::Arithmetic, "Subtract", fundamentalCache, symbolCache);
        case lyric_schema::LyricAstId::Mul:
            return resolve_action(lyric_assembler::FundamentalSymbol::Arithmetic, "Multiply", fundamentalCache, symbolCache);
        case lyric_schema::LyricAstId::Div:
            return resolve_action(lyric_assembler::FundamentalSymbol::Arithmetic, "Divide", fundamentalCache, symbolCache);
        case lyric_schema::LyricAstId::Neg:
            return resolve_action(lyric_assembler::FundamentalSymbol::Arithmetic, "Negate", fundamentalCache, symbolCache);
        case lyric_schema::LyricAstId::And:
            return resolve_action(lyric_assembler::FundamentalSymbol::Proposition, "Conjunct", fundamentalCache, symbolCache);
        case lyric_schema::LyricAstId::Or:
            return resolve_action(lyric_assembler::FundamentalSymbol::Proposition, "Disjunct", fundamentalCache, symbolCache);
        case lyric_schema::LyricAstId::Not:
            return resolve_action(lyric_assembler::FundamentalSymbol::Proposition, "Complement", fundamentalCache, symbolCache);
        case lyric_schema::LyricAstId::IsEq:
            return resolve_action(lyric_assembler::FundamentalSymbol::Equality, "Equals", fundamentalCache, symbolCache);
        case lyric_schema::LyricAstId::IsLt:
            return resolve_action(lyric_assembler::FundamentalSymbol::Comparison, "LessThan", fundamentalCache, symbolCache);
        case lyric_schema::LyricAstId::IsLe:
            return resolve_action(lyric_assembler::FundamentalSymbol::Comparison, "LessEquals", fundamentalCache, symbolCache);
        case lyric_schema::LyricAstId::IsGt:
            return resolve_action(lyric_assembler::FundamentalSymbol::Comparison, "GreaterThan", fundamentalCache, symbolCache);
        case lyric_schema::LyricAstId::IsGe:
            return resolve_action(lyric_assembler::FundamentalSymbol::Comparison, "GreaterEquals", fundamentalCache, symbolCache);

        default:
            return CompilerStatus::forCondition(CompilerCondition::kCompilerInvariant,
                "failed to resolve operator concept; invalid operator class");
    }
}