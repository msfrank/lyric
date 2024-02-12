#ifndef LYRIC_IMPORTER_IMPORTER_TYPES_H
#define LYRIC_IMPORTER_IMPORTER_TYPES_H

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <lyric_common/symbol_path.h>
#include <lyric_common/symbol_url.h>
#include <lyric_object/lyric_object.h>
#include <tempo_utils/compressed_bitmap.h>
#include <tempo_utils/status.h>

#include "type_import.h"

namespace lyric_importer {

    /**
     * Describes a parameter.
     */
    struct Parameter {
        std::string name;                       /**< name of the parameter */
        int index;                              /**< placement index */
        std::string label;                      /**< optional parameter label */
        TypeImport *type;                       /**< type of the parameter */
        lyric_object::PlacementType placement;  /**< parameter placement */
        bool isVariable;                        /**< true if the parameter has variable binding, otherwise false */
    };

    /**
     * Describes a template parameter.
     */
    struct TemplateParameter {
        std::string name;                       /**< */
        int index;                              /**< */
        TypeImport *type;                       /**< */
        lyric_object::VarianceType variance;    /**< */
        lyric_object::BoundType bound;          /**< */
    };

    /**
     * Describes an impl extension.
     */
    struct Extension {
        lyric_common::SymbolUrl actionUrl;
        lyric_common::SymbolUrl callUrl;
    };

    /**
     * Describes an impl.
     */
    struct Impl {
        TypeImport *type;
        absl::flat_hash_map<std::string, Extension> extensions;
    };
}

#endif // LYRIC_IMPORTER_IMPORTER_TYPES_H