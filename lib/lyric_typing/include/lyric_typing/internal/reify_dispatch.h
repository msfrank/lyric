#ifndef LYRIC_TYPING_INTERNAL_REIFY_DISPATCH_H
#define LYRIC_TYPING_INTERNAL_REIFY_DISPATCH_H

#include <lyric_assembler/object_state.h>

namespace lyric_typing::internal {

    struct DispatchState {
        lyric_assembler::ObjectState *objectState = nullptr;
        lyric_assembler::AbstractSymbol *invokerSymbol = nullptr;
        lyric_assembler::TemplateHandle *templateHandle = nullptr;
        std::vector<lyric_common::TypeDef> reifiedPlaceholders;
    };

    tempo_utils::Result<lyric_common::TypeDef> reify_singular_parameter(
        const lyric_common::TypeDef &paramType,
        const lyric_common::TypeDef &argType,
        DispatchState *invokerState);

    tempo_utils::Result<lyric_common::TypeDef> reify_union_parameter(
        const lyric_common::TypeDef &paramType,
        const lyric_common::TypeDef &argType,
        DispatchState *invokerState);

    tempo_utils::Result<lyric_common::TypeDef> reify_result_type(
        const lyric_common::TypeDef &returnType,
        DispatchState *invokerState);
}

#endif // LYRIC_TYPING_INTERNAL_REIFY_DISPATCH_H
