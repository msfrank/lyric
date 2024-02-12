#ifndef LYRIC_OBJECT_ACTION_WALKER_H
#define LYRIC_OBJECT_ACTION_WALKER_H

#include <span>

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class ActionParameterWalker;
    class ConceptWalker;
    class LinkWalker;
    class RestParameterWalker;
    class SymbolWalker;
    class TemplateWalker;
    class TypeWalker;

    class ActionWalker {

    public:
        ActionWalker();
        ActionWalker(const ActionWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;

        SymbolWalker getReceiver() const;

        bool hasTemplate() const;
        TemplateWalker getTemplate() const;

        tu_uint8 numParameters() const;
        ActionParameterWalker getParameter(tu_uint8 index) const;

        bool hasRest() const;
        RestParameterWalker getRest() const;

        TypeWalker getResultType() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_actionOffset;

        ActionWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 actionOffset);

        friend class ConceptAction;
        friend class ConceptWalker;
        friend class ExtensionWalker;
        friend class ObjectWalker;
        friend class ParameterWalker;
    };
}

#endif // LYRIC_OBJECT_ACTION_WALKER_H
