#ifndef LYRIC_PACKAGING_ENTRY_WALKER_H
#define LYRIC_PACKAGING_ENTRY_WALKER_H

#include <filesystem>

#include <tempo_utils/attr.h>
#include <tempo_utils/integer_types.h>

#include "manifest_attr_parser.h"
#include "package_result.h"
#include "package_types.h"

namespace lyric_packaging {

    class EntryWalker {

    public:
        EntryWalker();
        EntryWalker(const EntryWalker &other);

        bool isValid() const;

        EntryType getEntryType() const;
        std::filesystem::path getPath() const;
        tu_uint64 getFileOffset() const;
        tu_uint64 getFileSize() const;
        EntryWalker getLink() const;
        EntryWalker resolveLink() const;

        bool hasAttr(const tempo_utils::AttrKey &key) const;
        bool hasAttr(const tempo_utils::AttrValidator &validator) const;
        int numAttrs() const;

        EntryWalker getChild(tu_uint32 index) const;
        EntryWalker getChild(std::string_view name) const;
        int numChildren() const;

    private:
        std::shared_ptr<const internal::ManifestReader> m_reader;
        tu_uint32 m_index;

        EntryWalker(std::shared_ptr<const internal::ManifestReader> reader, tu_uint32 index);
        friend class ManifestWalker;

        tu_uint32 findIndexForAttr(const tempo_utils::AttrKey &key) const;

    public:
        /**
         *
         * @tparam AttrType
         * @tparam SerdeType
         * @param attr
         * @param value
         * @return
         */
        template<class AttrType,
            typename SerdeType = typename AttrType::SerdeType>
        tempo_utils::Status
        parseAttr(const AttrType &attr, SerdeType &value) const {
            auto index = findIndexForAttr(attr.getKey());
            if (index == kInvalidOffsetU32)
                return PackageStatus::forCondition(
                    PackageCondition::kPackageInvariant, "missing attr in entry");
            ManifestAttrParser parser(m_reader);
            return attr.parseAttr(index, &parser, value);
        }
    };
}

#endif // LYRIC_PACKAGING_ENTRY_WALKER_H