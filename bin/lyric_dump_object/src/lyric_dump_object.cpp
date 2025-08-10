#include <filesystem>
#include <iostream>

#include <absl/container/flat_hash_map.h>

#include <lyric_object/internal/object_reader.h>
#include <lyric_object/lyric_object.h>
#include <tempo_command/command_help.h>
#include <tempo_config/base_conversions.h>
#include <tempo_utils/file_reader.h>
#include <tempo_utils/unicode.h>

tempo_utils::Status
dump_lyric_object(int argc, const char *argv[])
{
    tempo_config::PathParser objectPathParser;
    tempo_config::IntegerParser verboseParser(0);
    tempo_config::IntegerParser quietParser(0);
    tempo_config::BooleanParser silentParser(false);

    std::vector<tempo_command::Default> cmdDefaults = {
        {"verbose", verboseParser.getDefault(),
            "Display verbose output (specify twice for even more verbose output)"},
        {"quiet", quietParser.getDefault(),
            "Display warnings and errors only (specify twice for errors only)"},
        {"silent", silentParser.getDefault(),
            "Suppress all output"},
        {"objectPath", {}, "path to the object to dump", "PATH"},
    };

    const std::vector<tempo_command::Grouping> cmdGroupings = {
        {"verbose", {"-v"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"quiet", {"-q"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"silent", {"-s", "--silent"}, tempo_command::GroupingType::NO_ARGUMENT},
        {"help", {"-h", "--help"}, tempo_command::GroupingType::HELP_FLAG},
        {"version", {"--version"}, tempo_command::GroupingType::VERSION_FLAG},
    };

    const std::vector<tempo_command::Mapping> optMappings = {
        {tempo_command::MappingType::COUNT_INSTANCES, "verbose"},
        {tempo_command::MappingType::COUNT_INSTANCES, "quiet"},
        {tempo_command::MappingType::TRUE_IF_INSTANCE, "silent"},
    };

    std::vector<tempo_command::Mapping> argMappings = {
        {tempo_command::MappingType::ONE_INSTANCE, "objectPath"},
    };

    // parse argv array into a vector of tokens
    auto tokenizeResult = tempo_command::tokenize_argv(argc - 1, &argv[1]);
    if (tokenizeResult.isStatus())
        tempo_command::display_status_and_exit(tokenizeResult.getStatus());
    auto tokens = tokenizeResult.getResult();

    std::string subcommand;
    tempo_command::OptionsHash cmdOptions;
    tempo_command::ArgumentVector cmdArguments;

    // parse global options and get the subcommand
    auto status = tempo_command::parse_completely(tokens, cmdGroupings, cmdOptions, cmdArguments);
    if (status.notOk()) {
        tempo_command::CommandStatus commandStatus;
        if (!status.convertTo(commandStatus))
            return status;
        switch (commandStatus.getCondition()) {
            case tempo_command::CommandCondition::kHelpRequested:
                tempo_command::display_help_and_exit({"dump-lyric-object"},
                    "Dump a lyric object into a human-readable format",
                    {}, cmdGroupings, optMappings, argMappings, cmdDefaults);
            default:
                return status;
        }
    }

    // initialize the global config from defaults
    auto cmdConfig = command_config_from_defaults(cmdDefaults);

    // convert options to config
    TU_RETURN_IF_NOT_OK (convert_options(cmdOptions, optMappings, cmdConfig));

    // convert arguments to config
    TU_RETURN_IF_NOT_OK (convert_arguments(cmdArguments, argMappings, cmdConfig));

    TU_LOG_INFO << "cmd config:\n" << tempo_command::command_config_to_string(cmdConfig);

    // configure logging
    tempo_utils::LoggingConfiguration logging = {
        tempo_utils::SeverityFilter::kDefault,
        false,
    };

    bool silent;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(silent, silentParser,
        cmdConfig, "silent"));
    if (silent) {
        logging.severityFilter = tempo_utils::SeverityFilter::kSilent;
    } else {
        int verbose, quiet;
        TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(verbose, verboseParser,
            cmdConfig, "verbose"));
        TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(quiet, quietParser,
            cmdConfig, "quiet"));
        if (verbose && quiet)
            return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
                "cannot specify both -v and -q");
        if (verbose == 1) {
            logging.severityFilter = tempo_utils::SeverityFilter::kVerbose;
        } else if (verbose > 1) {
            logging.severityFilter = tempo_utils::SeverityFilter::kVeryVerbose;
        }
        if (quiet == 1) {
            logging.severityFilter = tempo_utils::SeverityFilter::kWarningsAndErrors;
        } else if (quiet > 1) {
            logging.severityFilter = tempo_utils::SeverityFilter::kErrorsOnly;
        }
    }

    // initialize logging
    tempo_utils::init_logging(logging);

    // determine the object path
    std::filesystem::path objectPath;
    TU_RETURN_IF_NOT_OK(tempo_command::parse_command_config(objectPath, objectPathParser,
        cmdConfig, "objectPath"));

    // read the lyric object file into memory
    tempo_utils::FileReader objectReader(objectPath);
    if (!objectReader.isValid())
        return objectReader.getStatus();
    auto bytes = objectReader.getBytes();

    // verify the object
    if (!lyric_object::LyricObject::verify(std::span<const tu_uint8>(bytes->getData(), bytes->getSize())))
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "failed to verify object");
    lyric_object::LyricObject object(bytes);
    if (!object.isValid())
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "failed to load object");
    auto reader = object.getReader();
    if (reader == nullptr)
        return tempo_command::CommandStatus::forCondition(tempo_command::CommandCondition::kCommandError,
            "failed to load object");

    // dump the object in json format
    std::string jsonData = reader->dumpJson();
    TU_CONSOLE_OUT << jsonData;

    return {};
}

int
main(int argc, const char *argv[])
{
    if (argc == 0 || argv == nullptr)
        return -1;

    auto status = dump_lyric_object(argc, argv);
    if (!status.isOk())
        tempo_command::display_status_and_exit(status);
    return 0;
}