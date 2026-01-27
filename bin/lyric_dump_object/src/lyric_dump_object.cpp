#include <filesystem>
#include <iostream>

#include <absl/container/flat_hash_map.h>

#include <lyric_object/internal/object_reader.h>
#include <lyric_object/lyric_object.h>
#include <tempo_command/command.h>
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

    tempo_command::Command command("lyric-dump-object");

    command.addArgument("objectPath", "PATH", tempo_command::MappingType::ONE_INSTANCE,
        "Path to the object to dump");
    command.addFlag("verbose", {"-v"}, tempo_command::MappingType::COUNT_INSTANCES,
        "Display verbose output (specify twice for even more verbose output)");
    command.addFlag("quiet", {"-q"}, tempo_command::MappingType::COUNT_INSTANCES,
        "Display warnings and errors only (specify twice for errors only)");
    command.addFlag("silent", {"-s", "--silent"}, tempo_command::MappingType::TRUE_IF_INSTANCE,
        "Suppress all output");
    command.addHelpOption("help", {"-h", "--help"},
        "Dump a lyric object into a human-readable format");

    TU_RETURN_IF_NOT_OK (command.parse(argc - 1, &argv[1]));

    // configure logging
    tempo_utils::LoggingConfiguration logging = {
        tempo_utils::SeverityFilter::kDefault,
        false,
    };

    bool silent;
    TU_RETURN_IF_NOT_OK(command.convert(silent, silentParser, "silent"));
    if (silent) {
        logging.severityFilter = tempo_utils::SeverityFilter::kSilent;
    } else {
        int verbose, quiet;
        TU_RETURN_IF_NOT_OK(command.convert(verbose, verboseParser, "verbose"));
        TU_RETURN_IF_NOT_OK(command.convert(quiet, quietParser, "quiet"));
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
    TU_RETURN_IF_NOT_OK(command.convert(objectPath, objectPathParser, "objectPath"));

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