
#include <absl/strings/ascii.h>

#include <lyric_analyzer/internal/analyzer_utils.h>
#include <lyric_analyzer/analyzer_result.h>
#include <lyric_assembler/concept_symbol.h>

lyric_object::DeriveType
lyric_analyzer::internal::convert_derive_type(lyric_parser::DeriveType derive)
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

tempo_utils::Result<lyric_common::TypeDef>
lyric_analyzer::internal::resolve_companion(
    const lyric_common::SymbolPath &symbolPath,
    const std::string &literalValue,
    lyric_assembler::BlockHandle *block)
{
    lyric_common::TypeDef conceptType;
    TU_ASSIGN_OR_RETURN (conceptType, lyric_common::TypeDef::forConcrete(lyric_common::SymbolUrl(symbolPath)));
    lyric_assembler::ConceptSymbol *conceptSymbol;
    TU_ASSIGN_OR_RETURN (conceptSymbol, block->resolveConcept(conceptType));

    auto *templateHandle = conceptSymbol->conceptTemplate();
    if (templateHandle == nullptr)
        return AnalyzerStatus::forCondition(
            AnalyzerCondition::kAnalyzerInvariant,
            "invalid companion symbol '{}'", conceptType.toString());

    if (literalValue.empty())
        return AnalyzerStatus::forCondition(
            AnalyzerCondition::kAnalyzerInvariant,
            "invalid literal value '{}' for companion alias", literalValue);

    lyric_common::TypeDef companionType;
    if (absl::ascii_isalpha(literalValue.front())) {
        if (templateHandle->hasTemplateParameter(literalValue)) {
            companionType = templateHandle->getPlaceholder(literalValue);
        }
    } else {
        int index;
        if (!absl::SimpleAtoi(literalValue, &index))
            return AnalyzerStatus::forCondition(
                AnalyzerCondition::kAnalyzerInvariant,
                "invalid template parameter index {}", literalValue);
        companionType = templateHandle->getPlaceholder(index);
    }

    if (!companionType.isValid())
        return AnalyzerStatus::forCondition(
            AnalyzerCondition::kAnalyzerInvariant,
            "missing template parameter {} on symbol {}", literalValue, conceptType.toString());

    return companionType;
}
