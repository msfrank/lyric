#ifndef LYRIC_IMPORTER_IMPORTER_TYPES_H
#define LYRIC_IMPORTER_IMPORTER_TYPES_H

#include <optional>

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
        std::string name;                       /**< The name of the parameter. */
        int index;                              /**< The placement index. */
        std::weak_ptr<TypeImport> type;         /**< The type of the parameter. */
        lyric_object::PlacementType placement;  /**< The parameter placement type. */
        bool isVariable;                        /**< true if the parameter has variable binding, otherwise false. */
    };

    /**
     * Describes a template parameter.
     */
    struct TemplateParameter {
        std::string name;                       /**< The name of the template parameter. */
        int index;                              /**< The index of the parameter in the template. */
        lyric_object::VarianceType variance;    /**< The variance of the parameter. */
        struct Constraint {
            lyric_object::BoundType bound;      /**< The type bound. */
            std::weak_ptr<TypeImport> type;     /**< The constraint type. */
        };
        std::optional<Constraint> constraint;   /**< Contains the constraint on the template parameter, or empty if there is no constraint. */
    };

    /**
     * Describes an impl extension.
     */
    struct Extension {
        lyric_common::SymbolUrl actionUrl;      /**< The symbol url of the action declaration. */
        lyric_common::SymbolUrl callUrl;        /**< The symbol url of the call definition. */
    };

    /**
     * Describes an impl.
     */
    struct Impl {
        std::weak_ptr<TypeImport> type;         /**< The impl type. */
        absl::flat_hash_map<
            std::string,
            Extension> extensions;              /**< Map containing the extensions provided by the impl. */
    };
}

#endif // LYRIC_IMPORTER_IMPORTER_TYPES_H