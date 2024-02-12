#ifndef LYRIC_OBJECT_TEMPLATE_WALKER_H
#define LYRIC_OBJECT_TEMPLATE_WALKER_H

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class TemplateParameterWalker;

    class TemplateWalker {

    public:
        TemplateWalker();
        TemplateWalker(const TemplateWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;

        TemplateParameterWalker getTemplateParameter(tu_uint8 index) const;
        tu_uint8 numTemplateParameters() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_templateOffset;

        TemplateWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 templateOffset);

        friend class ActionWalker;
        friend class CallWalker;
        friend class ClassWalker;
        friend class ConceptWalker;
        friend class ExistentialWalker;
        friend class ObjectWalker;
        friend class PlaceholderTypeWalker;
    };
}

#endif // LYRIC_OBJECT_TEMPLATE_WALKER_H
