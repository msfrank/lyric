#ifndef LYRIC_TYPING_TYPING_TYPES_H
#define LYRIC_TYPING_TYPING_TYPES_H

#include <lyric_object/object_types.h>
#include <lyric_parser/assignable.h>
#include <lyric_parser/node_walker.h>

namespace lyric_typing {

    struct TemplateSpec {
        lyric_parser::NodeWalker node;              // the node of the template in the archetype
        std::vector<lyric_object::TemplateParameter> templateParameters;
        TemplateSpec();
        TemplateSpec(
            const lyric_parser::NodeWalker &node,
            const std::vector<lyric_object::TemplateParameter> &templateParameters);
    };

    struct ParameterSpec {
        lyric_parser::NodeWalker node;          // the node of the parameter in the archetype
        std::string name;                       // name of the parameter
        std::string label;                      // optional parameter label
        lyric_parser::Assignable type;          // type of the parameter
        lyric_parser::BindingType binding;      // fixed or variable binding
        Option<lyric_parser::NodeWalker> init;  // node containing the default initializer, can be empty
        ParameterSpec();
        ParameterSpec(
            const lyric_parser::NodeWalker &node,
            const std::string &name,
            const std::string &label,
            const lyric_parser::Assignable &type,
            lyric_parser::BindingType binding,
            const Option<lyric_parser::NodeWalker> &init = {});
    };

    struct PackSpec {
        lyric_parser::NodeWalker node;                  // the node of the pack in the archetype
        std::vector<ParameterSpec> listParameterSpec;   // list parameters
        std::vector<ParameterSpec> namedParameterSpec;  // named parameters
        std::vector<ParameterSpec> ctxParameterSpec;    // ctx parameters
        Option<ParameterSpec> restParameterSpec;        // rest parameter, or empty if there is no rest parameter
        absl::flat_hash_map<
            std::string,
            lyric_parser::NodeWalker> initializers;     //
        PackSpec();
        PackSpec(
            const lyric_parser::NodeWalker &node,
            const std::vector<ParameterSpec> &listParameterSpec,
            const std::vector<ParameterSpec> &namedParameterSpec,
            const std::vector<ParameterSpec> &ctxParameterSpec,
            const Option<ParameterSpec> &restParameterSpec,
            const absl::flat_hash_map<
                std::string,
                lyric_parser::NodeWalker> &initializers);
    };
}

#endif // LYRIC_TYPING_TYPING_TYPES_H
