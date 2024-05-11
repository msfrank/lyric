#ifndef LYRIC_TYPING_TYPE_SYSTEM_H
#define LYRIC_TYPING_TYPE_SYSTEM_H

#include <lyric_assembler/assembly_state.h>
#include <lyric_assembler/block_handle.h>
#include <lyric_parser/node_walker.h>

namespace lyric_typing {

    class TypeSystem {

    public:
        explicit TypeSystem(lyric_assembler::AssemblyState *state);

        lyric_assembler::AssemblyState *getState() const;

        tempo_utils::Result<lyric_parser::Assignable>
        parseAssignable(
            lyric_assembler::BlockHandle *block,
            const lyric_parser::NodeWalker &walker);

        tempo_utils::Result<lyric_assembler::PackSpec>
        parsePack(
            lyric_assembler::BlockHandle *block,
            const lyric_parser::NodeWalker &walker);

        tempo_utils::Result<lyric_runtime::TypeComparison> compareAssignable(
            const lyric_common::TypeDef &toRef,
            const lyric_common::TypeDef &fromRef);

        tempo_utils::Result<bool> isAssignable(
            const lyric_common::TypeDef &toRef,
            const lyric_common::TypeDef &fromRef);

        tempo_utils::Result<lyric_common::TypeDef>
        resolveAssignable(
            lyric_assembler::BlockHandle *block,
            const lyric_parser::NodeWalker &walker);

        tempo_utils::Result<std::pair<lyric_object::BoundType,lyric_common::TypeDef>>
        resolveBound(const lyric_common::TypeDef &placeholderType);

        tempo_utils::Result<lyric_assembler::TemplateSpec>
        resolveTemplate(
            lyric_assembler::BlockHandle *block,
            const lyric_parser::NodeWalker &walker);

        tempo_utils::Result<lyric_common::TypeDef> unifyAssignable(
            const lyric_common::TypeDef &toRef,
            const lyric_common::TypeDef &fromRef);

        tempo_utils::Result<bool> isImplementable(
            const lyric_common::TypeDef &toConcept,
            const lyric_common::TypeDef &fromRef);

    private:
        lyric_assembler::AssemblyState *m_state;
    };
}

#endif // LYRIC_TYPING_TYPE_SYSTEM_H