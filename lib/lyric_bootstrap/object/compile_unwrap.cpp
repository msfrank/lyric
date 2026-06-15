
#include "compile_unwrap.h"
#include "prelude_symbols.h"

CoreConcept *build_core_UnwrapN(BuilderState &state, int arity, const PreludeSymbols &preludeSymbols)
{
    lyric_common::SymbolPath conceptPath({absl::StrCat("Unwrap", arity)});

    std::vector<CorePlaceholder> placeholders;
    placeholders.push_back({"WrappedType", lyo1::PlaceholderVariance::Invariant});
    for (int i = 0; i < arity; i++) {
        auto name = absl::StrCat("T", i);
        CorePlaceholder p;
        placeholders.push_back({name, lyo1::PlaceholderVariance::Invariant, /* isAlias= */ true});
    }

    auto *UnwrapNTemplate = state.addTemplate(conceptPath, placeholders);

    auto *WrappedType = UnwrapNTemplate->types["WrappedType"];

    auto *UnwrapConcept = state.addGenericConcept(conceptPath, UnwrapNTemplate,
        lyo1::ConceptFlags::NONE, preludeSymbols.IdeaConcept);

    std::vector<const CoreType *> elementTypes;
    for (int i = 1; i < UnwrapNTemplate->placeholders.size(); i++) {
        const auto *elementType = UnwrapNTemplate->types.at(absl::StrCat("T", i - 1));
        elementTypes.push_back(elementType);
    }

    auto *TupleNClass = preludeSymbols.tupleClasses[arity - 1];
    auto *TupleNType = TupleNClass->classType;

    auto *ResultType = state.addConcreteType(
        TupleNType, TupleNType->concreteSection, TupleNType->concreteDescriptor, elementTypes);

    state.addConceptAction("Unwrap", UnwrapConcept,
        {
            make_list_param("wrapped", WrappedType),
        },
        ResultType);

    return UnwrapConcept;
}
