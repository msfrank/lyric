#ifndef LYRIC_OBJECT_TEMPLATE_PARAMETER_WALKER_H
#define LYRIC_OBJECT_TEMPLATE_PARAMETER_WALKER_H

#include "type_walker.h"

namespace lyric_object {

    class TemplateParameterWalker {

    public:
        TemplateParameterWalker();
        TemplateParameterWalker(const TemplateParameterWalker &other);

        bool isValid() const;

        TemplateParameter getTemplateParameter() const;

        std::string getPlaceholderName() const;
        VarianceType getPlaceholderVariance() const;

        bool hasConstraint() const;
        TypeWalker getConstraintType() const;
        BoundType getConstraintBound() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_templateDescriptor;
        tu_uint8 m_placeholderOffset;

        TemplateParameterWalker(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *templateDescriptor,
            tu_uint8 placeholderOffset);

        friend class TemplateWalker;
    };
}

#endif // LYRIC_OBJECT_TEMPLATE_PARAMETER_WALKER_H
