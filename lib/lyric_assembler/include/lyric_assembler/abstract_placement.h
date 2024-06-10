#ifndef LYRIC_ASSEMBLER_ABSTRACT_PLACEMENT_H
#define LYRIC_ASSEMBLER_ABSTRACT_PLACEMENT_H

#include <lyric_common/symbol_url.h>
#include <lyric_object/object_types.h>

#include "abstract_callsite_reifier.h"
#include "assembler_types.h"

namespace lyric_assembler {

    // forward declarations
    class TemplateHandle;

    class AbstractPlacement {
    public:
        virtual ~AbstractPlacement() = default;

        virtual TemplateHandle *getTemplate() const = 0;
        virtual std::vector<Parameter>::const_iterator listPlacementBegin() const = 0;
        virtual std::vector<Parameter>::const_iterator listPlacementEnd() const = 0;
        virtual std::vector<Parameter>::const_iterator namedPlacementBegin() const = 0;
        virtual std::vector<Parameter>::const_iterator namedPlacementEnd() const = 0;
        virtual const Parameter *restPlacement() const = 0;
        virtual bool hasInitializer(const std::string &name) const = 0;
        virtual lyric_common::SymbolUrl getInitializer(const std::string &name) const = 0;
    };
}

#endif // LYRIC_ASSEMBLER_ABSTRACT_PLACEMENT_H
