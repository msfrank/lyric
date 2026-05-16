
#include "compile_unwrap.h"

// CoreConcept *build_core_Unwrap(BuilderState &state, const CoreConcept *IdeaConcept)
// {
//     lyric_common::SymbolPath conceptPath({"Unwrap"});
//
//     auto *UnwrapTemplate = state.addTemplate(
//         conceptPath,
//         {
//             {"W", lyo1::PlaceholderVariance::Invariant},
//             {"T", lyo1::PlaceholderVariance::Invariant},
//         });
//
//     auto *WType = UnwrapTemplate->types["W"];
//     auto *TType = UnwrapTemplate->types["T"];
//
//     auto *UnwrapConcept = state.addGenericConcept(conceptPath, UnwrapTemplate,
//         lyo1::ConceptFlags::NONE, IdeaConcept);
//
//     state.addConceptAction("Unwrap", UnwrapConcept,
//         {
//             make_list_param("wrapped", WType),
//         },
//         TType);
//
//     return UnwrapConcept;
// }

CoreConcept *build_core_UnwrapN(
    BuilderState &state,
    int arity,
    const CoreConcept *IdeaConcept,
    const CoreType *TupleNType)
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
        lyo1::ConceptFlags::NONE, IdeaConcept);

    std::vector<const CoreType *> elementTypes;
    for (int i = 1; i < UnwrapNTemplate->placeholders.size(); i++) {
        const auto *elementType = UnwrapNTemplate->types.at(absl::StrCat("T", i - 1));
        elementTypes.push_back(elementType);
    }

    auto *ResultType = state.addConcreteType(
        TupleNType, TupleNType->concreteSection, TupleNType->concreteDescriptor, elementTypes);

    state.addConceptAction("Unwrap", UnwrapConcept,
        {
            make_list_param("wrapped", WrappedType),
        },
        ResultType);

    return UnwrapConcept;
}
