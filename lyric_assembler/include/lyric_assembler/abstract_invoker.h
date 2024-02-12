#ifndef LYRIC_ASSEMBLER_ABSTRACT_INVOKER_H
#define LYRIC_ASSEMBLER_ABSTRACT_INVOKER_H

#include <lyric_common/symbol_url.h>
#include <lyric_object/object_types.h>

namespace lyric_assembler {

    class AbstractInvoker {

    public:
        virtual ~AbstractInvoker() = default;

        virtual std::vector<lyric_object::Parameter>::const_iterator placementBegin() const = 0;
        virtual std::vector<lyric_object::Parameter>::const_iterator placementEnd() const = 0;
        virtual bool hasInitializer(const std::string &name) const = 0;
        virtual lyric_common::SymbolUrl getInitializer(const std::string &name) const = 0;
    };
}

#endif // LYRIC_ASSEMBLER_ABSTRACT_INVOKER_H