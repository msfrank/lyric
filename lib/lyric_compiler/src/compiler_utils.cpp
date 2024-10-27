
#include <lyric_compiler/compiler_result.h>
#include <lyric_compiler/compiler_utils.h>

lyric_object::AccessType
lyric_compiler::convert_access_type(lyric_parser::AccessType access)
{
    switch (access) {
        case lyric_parser::AccessType::Public:
            return lyric_object::AccessType::Public;
        case lyric_parser::AccessType::Protected:
            return lyric_object::AccessType::Protected;
        case lyric_parser::AccessType::Private:
            return lyric_object::AccessType::Private;
        default:
            return lyric_object::AccessType::Invalid;
    }
}

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
            return {"add"};
        case lyric_schema::LyricAstId::Sub:
            return {"subtract"};
        case lyric_schema::LyricAstId::Mul:
            return {"multiply"};
        case lyric_schema::LyricAstId::Div:
            return {"divide"};
        case lyric_schema::LyricAstId::Neg:
            return {"negate"};
        case lyric_schema::LyricAstId::And:
            return {"conjunct"};
        case lyric_schema::LyricAstId::Or:
            return {"disjunct"};
        case lyric_schema::LyricAstId::Not:
            return {"complement"};
        case lyric_schema::LyricAstId::IsEq:
            return {"equals"};
        case lyric_schema::LyricAstId::IsLt:
            return {"lessthan"};
        case lyric_schema::LyricAstId::IsLe:
            return {"lessequals"};
        case lyric_schema::LyricAstId::IsGt:
            return {"greaterthan"};
        case lyric_schema::LyricAstId::IsGe:
            return {"greaterequals"};
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
