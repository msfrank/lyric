#ifndef LYRIC_IMPORTER_TYPE_IMPORT_H
#define LYRIC_IMPORTER_TYPE_IMPORT_H

#include "module_import.h"

namespace lyric_importer {

    class TypeImport {
    public:
        TypeImport(std::shared_ptr<ModuleImport> moduleImport, tu_uint32 typeOffset);

        lyric_common::TypeDef getTypeDef();
        TypeImport *getSuperType();

        std::vector<TypeImport *>::const_iterator argumentsBegin();
        std::vector<TypeImport *>::const_iterator argumentsEnd();
        int numArguments();

    private:
        std::shared_ptr<ModuleImport> m_moduleImport;
        tu_uint32 m_typeOffset;
        absl::Mutex m_lock;

        struct Priv;
        std::unique_ptr<Priv> m_priv ABSL_GUARDED_BY(m_lock);

        void load();
    };
}

#endif // LYRIC_IMPORTER_TYPE_IMPORT_H
