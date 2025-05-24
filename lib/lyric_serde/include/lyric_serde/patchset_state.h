#ifndef LYRIC_SERDE_PATCHSET_STATE_H
#define LYRIC_SERDE_PATCHSET_STATE_H

#include <tempo_schema/attr.h>
#include <tempo_schema/schema_resource.h>
#include <tempo_utils/url.h>

#include "lyric_patchset.h"
#include "operation_path.h"
#include "serde_result.h"
#include "serde_types.h"

namespace lyric_serde {

    // forward declarations
    class PatchsetNamespace;
    class PatchsetChange;
    class PatchsetValue;

    class PatchsetState {

    public:
        PatchsetState();

        bool hasNamespace(const tempo_utils::Url &nsUrl) const;
        PatchsetNamespace *getNamespace(int index) const;
        PatchsetNamespace *getNamespace(const tempo_utils::Url &nsUrl) const;
        tempo_utils::Result<PatchsetNamespace *> putNamespace(const tempo_utils::Url &nsUrl);
        std::vector<PatchsetNamespace *>::const_iterator namespacesBegin() const;
        std::vector<PatchsetNamespace *>::const_iterator namespacesEnd() const;
        int numNamespaces() const;

        tempo_utils::Result<PatchsetValue *> appendValue(const tempo_schema::AttrValue &value);
        tempo_utils::Result<PatchsetValue *> appendValue(
            const char *nsUrl,
            tu_uint32 idValue,
            ValueAddress value);
        tempo_utils::Result<PatchsetValue *> appendValue(
            const char *nsUrl,
            tu_uint32 idValue,
            const std::vector<ValueAddress> &children);
        PatchsetValue *getValue(int index) const;
        std::vector<PatchsetValue *>::const_iterator valuesBegin() const;
        std::vector<PatchsetValue *>::const_iterator valuesEnd() const;
        int numValues() const;

        bool hasChange(std::string_view id) const;
        PatchsetChange *getChange(int index) const;
        PatchsetChange *getChange(std::string_view id) const;
        tempo_utils::Result<PatchsetChange *> appendChange(std::string_view id);
        std::vector<PatchsetChange *>::const_iterator changesBegin() const;
        std::vector<PatchsetChange *>::const_iterator changesEnd() const;
        int numChanges() const;

        tempo_utils::Result<LyricPatchset> toPatchset() const;

    private:
        std::vector<PatchsetNamespace *> m_patchsetNamespaces;
        std::vector<PatchsetValue *> m_patchsetValues;
        std::vector<PatchsetChange *> m_patchsetChanges;
        absl::flat_hash_map<tempo_utils::Url,tu_int16> m_namespaceIndex;
        absl::flat_hash_map<std::string,tu_uint32> m_idIndex;

    public:
        /**
         *
         * @tparam NsType
         * @tparam IdType
         * @param attrProperty
         * @param value
         * @return
         */
        template<class NsType, class IdType>
        tempo_utils::Result<PatchsetValue *> appendValue(
            const tempo_schema::SchemaProperty<NsType, IdType> &attrProperty,
            ValueAddress value)
        {
            return appendValue(attrProperty.getNsUrl(), attrProperty.getIdValue(), value);
        }

        /**
         *
         * @tparam NsType
         * @tparam IdType
         * @param elementClass
         * @param children
         * @return
         */
        template<class NsType, class IdType>
        tempo_utils::Result<PatchsetValue *> appendValue(
            const tempo_schema::SchemaClass<NsType, IdType> &elementClass,
            const std::vector<ValueAddress> &children)
        {
            return appendValue(elementClass.getNsUrl(), elementClass.getIdValue(), children);
        }
    };
}

#endif // LYRIC_SERDE_PATCHSET_STATE_H
