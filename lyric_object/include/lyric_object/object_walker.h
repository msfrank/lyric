#ifndef LYRIC_OBJECT_OBJECT_WALKER_H
#define LYRIC_OBJECT_OBJECT_WALKER_H

#include <tempo_utils/integer_types.h>

#include "action_parameter_walker.h"
#include "action_walker.h"
#include "call_parameter_walker.h"
#include "call_walker.h"
#include "class_walker.h"
#include "concept_walker.h"
#include "concrete_type_walker.h"
#include "enum_walker.h"
#include "existential_walker.h"
#include "extension_walker.h"
#include "field_walker.h"
#include "import_walker.h"
#include "impl_walker.h"
#include "instance_walker.h"
#include "intersection_type_walker.h"
#include "link_walker.h"
#include "literal_walker.h"
#include "namespace_walker.h"
#include "object_types.h"
#include "placeholder_type_walker.h"
#include "rest_parameter_walker.h"
#include "static_walker.h"
#include "struct_walker.h"
#include "symbol_walker.h"
#include "template_walker.h"
#include "template_parameter_walker.h"
#include "type_walker.h"
#include "union_type_walker.h"

namespace lyric_object {

    class ObjectWalker {

    public:
        ObjectWalker();
        ObjectWalker(const ObjectWalker &other);

        bool isValid() const;

        uint32_t getVersionMajor() const;
        uint32_t getVersionMinor() const;
        uint32_t getVersionPatch() const;
        std::string getVersionString() const;

        ActionWalker getAction(tu_uint32 index) const;
        int numActions() const;

        CallWalker getCall(tu_uint32 index) const;
        int numCalls() const;

        ClassWalker getClass(tu_uint32 index) const;
        int numClasses() const;

        ConceptWalker getConcept(tu_uint32 index) const;
        int numConcepts() const;

        EnumWalker getEnum(tu_uint32 index) const;
        int numEnums() const;

        ExistentialWalker getExistential(tu_uint32 index) const;
        int numExistentials() const;

        FieldWalker getField(tu_uint32 index) const;
        int numFields() const;

        ImplWalker getImpl(tu_uint32 index) const;
        int numImpls() const;

        ImportWalker getImport(tu_uint32 index) const;
        int numImports() const;

        InstanceWalker getInstance(tu_uint32 index) const;
        int numInstances() const;

        LinkWalker getLink(tu_uint32 index) const;
        int numLinks() const;

        LiteralWalker getLiteral(tu_uint32 index) const;
        int numLiterals() const;

        NamespaceWalker getNamespace(tu_uint32 index) const;
        int numNamespaces() const;

        StaticWalker getStatic(tu_uint32 index) const;
        int numStatics() const;

        StructWalker getStruct(tu_uint32 index) const;
        int numStructs() const;

        TemplateWalker getTemplate(tu_uint32 index) const;
        int numTemplates() const;

        TypeWalker getType(tu_uint32 index) const;
        int numTypes() const;

        SymbolWalker getSymbol(tu_uint32 index) const;
        SymbolWalker findSymbol(const lyric_common::SymbolPath &symbolPath) const;
        int numSymbols() const;

        lyric_common::SymbolPath getSymbolPath(LinkageSection section, tu_uint32 index) const;
        tu_uint32 getSymbolIndex(const lyric_common::SymbolPath &symbolPath) const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;

        ObjectWalker(std::shared_ptr<const internal::ObjectReader> reader);
        friend class LyricObject;
    };

}

#endif // LYRIC_OBJECT_OBJECT_WALKER_H
