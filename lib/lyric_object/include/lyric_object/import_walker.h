#ifndef LYRIC_OBJECT_IMPORT_WALKER_H
#define LYRIC_OBJECT_IMPORT_WALKER_H

#include <lyric_common/assembly_location.h>

#include "object_types.h"

namespace lyric_object {

    class ImportWalker {

    public:
        ImportWalker();
        ImportWalker(const ImportWalker &other);

        bool isValid() const;

        lyric_common::AssemblyLocation getImportLocation() const;

        bool isSystemBootstrap() const;

        HashType getHashType() const;
        std::string getHash() const;

        tu_uint32 getDescriptorOffset() const;

    private:
        std::shared_ptr<const internal::ObjectReader> m_reader;
        tu_uint32 m_importOffset;

        ImportWalker(std::shared_ptr<const internal::ObjectReader> reader, tu_uint32 importOffset);

        friend class LinkWalker;
        friend class ObjectWalker;
    };
}

#endif // LYRIC_OBJECT_IMPORT_WALKER_H
