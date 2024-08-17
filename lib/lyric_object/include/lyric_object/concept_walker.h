#ifndef LYRIC_OBJECT_CONCEPT_WALKER_H
#define LYRIC_OBJECT_CONCEPT_WALKER_H

#include "object_types.h"

namespace lyric_object {

    // forward declarations
    class ActionWalker;
    class ImplWalker;
    class LinkWalker;
    class ParameterWalker;
    class TemplateWalker;
    class TypeWalker;

    /**
     *
     */
    class ConceptAction {
    public:
        ConceptAction();
        ConceptAction(const ConceptAction &other);

        bool isValid() const;
        AddressType actionAddressType() const;
        ActionWalker getNearAction() const;
        LinkWalker getFarAction() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        void *m_conceptDescriptor;
        tu_uint8 m_actionOffset;

        ConceptAction(
            std::shared_ptr<const internal::ObjectReader> reader,
            void *conceptDescriptor,
            tu_uint8 actionOffset);

        friend class ConceptWalker;
    };

    /**
     *
     */
    class ConceptWalker {
    public:
        ConceptWalker();
        ConceptWalker(const ConceptWalker &other);

        bool isValid() const;

        lyric_common::SymbolPath getSymbolPath() const;

        bool isDeclOnly() const;
        DeriveType getDeriveType() const;
        AccessType getAccess() const;

        bool hasSuperConcept() const;
        AddressType superConceptAddressType() const;
        ConceptWalker getNearSuperConcept() const;
        LinkWalker getFarSuperConcept() const;

        bool hasTemplate() const;
        TemplateWalker getTemplate() const;

        tu_uint8 numActions() const;
        ConceptAction getAction(tu_uint8 index) const;

        tu_uint8 numImpls() const;
        ImplWalker getImpl(tu_uint8 index) const;

        tu_uint8 numSealedSubConcepts() const;
        TypeWalker getSealedSubConcept(tu_uint8 index) const;

        TypeWalker getConceptType() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_conceptOffset;

        ConceptWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 conceptOffset);

        friend class ActionWalker;
        friend class FieldWalker;
        friend class ImplWalker;
        friend class ObjectWalker;
        friend class ParameterWalker;
        friend class StaticWalker;
    };
}

#endif // LYRIC_OBJECT_CONCEPT_WALKER_H
