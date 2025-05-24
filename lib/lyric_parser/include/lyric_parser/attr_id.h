#ifndef LYRIC_PARSER_ATTR_ID_H
#define LYRIC_PARSER_ATTR_ID_H

#include <tempo_schema/schema_namespace.h>

#include "parser_types.h"

namespace lyric_parser {

    // forward declarations
    class ArchetypeNamespace;

    class AttrId {

    public:
        AttrId();
        AttrId(ArchetypeNamespace *attrNamespace, tu_uint32 type);
        AttrId(const AttrId &other);

        bool isValid() const;
        ArchetypeNamespace *getNamespace() const;
        std::string_view namespaceView() const;
        bool isNamespace(const tempo_schema::SchemaNs &schemaNs) const;
        tu_uint32 getType() const;

        bool operator==(const AttrId &other) const;

        template <typename H>
        friend H AbslHashValue(H h, const AttrId &id) {
            if (id.m_namespace != nullptr)
                return H::combine(std::move(h), id.getIdOffset(), id.m_type);
            return H::combine(std::move(h), 0);
        }

    private:
        ArchetypeNamespace *m_namespace;
        tu_uint32 m_type;
        tu_uint32 getIdOffset() const;
    };
}

#endif // LYRIC_PARSER_ATTR_ID_H
