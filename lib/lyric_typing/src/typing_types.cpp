
#include <lyric_typing/typing_types.h>

lyric_typing::TemplateSpec::TemplateSpec()
{
}

lyric_typing::TemplateSpec::TemplateSpec(
    const lyric_parser::NodeWalker &node,
    const std::vector<lyric_object::TemplateParameter> &templateParameters)
    : node(node),
      templateParameters(templateParameters)
{
    TU_ASSERT (node.isValid());
}

lyric_typing::ParameterSpec::ParameterSpec()
    : name(),
      label(),
      type(),
      binding(lyric_parser::BindingType::VALUE),
      init()
{
}

lyric_typing::ParameterSpec::ParameterSpec(
    const lyric_parser::NodeWalker &node,
    const std::string &name,
    const std::string &label,
    const lyric_parser::TypeSpec &type,
    lyric_parser::BindingType binding,
    const Option<lyric_parser::NodeWalker> &init)
    : node(node),
      name(name),
      label(label),
      type(type),
      binding(binding),
      init(init)
{
    TU_ASSERT (node.isValid());
}

lyric_typing::PackSpec::PackSpec()
{
}

lyric_typing::PackSpec::PackSpec(
    const lyric_parser::NodeWalker &node,
    const std::vector<ParameterSpec> &listParameterSpec,
    const std::vector<ParameterSpec> &namedParameterSpec,
    const std::vector<ParameterSpec> &ctxParameterSpec,
    const Option<ParameterSpec> &restParameterSpec,
    const absl::flat_hash_map<std::string,lyric_parser::NodeWalker> &initializers)
    : node(node),
      listParameterSpec(listParameterSpec),
      namedParameterSpec(namedParameterSpec),
      ctxParameterSpec(ctxParameterSpec),
      restParameterSpec(restParameterSpec),
      initializers(initializers)
{
    TU_ASSERT (node.isValid());
}
