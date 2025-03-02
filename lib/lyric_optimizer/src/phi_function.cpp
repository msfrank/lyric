
#include <absl/strings/str_join.h>
#include <lyric_optimizer/optimizer_result.h>
#include <lyric_optimizer/phi_function.h>

lyric_optimizer::PhiFunction::PhiFunction(const std::vector<Instance> &arguments)
    : m_arguments(arguments)
{
    TU_ASSERT (!m_arguments.empty());
}

lyric_optimizer::DirectiveType
lyric_optimizer::PhiFunction::getType() const
{
    return DirectiveType::PhiFunction;
}

bool
lyric_optimizer::PhiFunction::isExpression() const
{
    return false;
}

tempo_utils::Status
lyric_optimizer::PhiFunction::applyOperands(ActivationState &state, OperandStack &stack)
{
    return {};
}

tempo_utils::Status
lyric_optimizer::PhiFunction::buildCode(
    lyric_assembler::CodeFragment *codeFragment,
    lyric_assembler::ProcHandle *procHandle)
{
    return OptimizerStatus::forCondition(
        OptimizerCondition::kOptimizerInvariant, "unimplemented");
}

std::string
lyric_optimizer::PhiFunction::toString() const
{
    std::vector<std::string> arguments;
    for (const auto &argument : m_arguments) {
        arguments.push_back(argument.getName());
    }
    auto arglist = absl::StrJoin(arguments.begin(), arguments.end(), ",");
    return absl::StrCat("PhiFunction(", arglist, ")");
}

std::vector<lyric_optimizer::Instance>::const_iterator
lyric_optimizer::PhiFunction::argumentsBegin() const
{
    return m_arguments.cbegin();
}

std::vector<lyric_optimizer::Instance>::const_iterator
lyric_optimizer::PhiFunction::argumentsEnd() const
{
    return m_arguments.cend();
}

int
lyric_optimizer::PhiFunction::numArguments() const
{
    return m_arguments.size();
}