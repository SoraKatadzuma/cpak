import argparse

parser = argparse.ArgumentParser(
    prog        = "cpak",
    description = """Configuration as code build tool for C/C++ designed to make it easier to build
your projects in every single way.""",
    add_help    = False
)

parser.add_argument(
    "-h",
    "--help",
    action = "help",
    help   = "Prints this help message and exits."
)

parser.add_argument(
    "-v",
    "--verbose",
    action = "store_true",
    dest   = "verbose",
    help   = "Provides more descriptive console output.",
)

command_parent = argparse.ArgumentParser(
    parents  = [parser],
    add_help = False
)

command_parent.add_argument(
    "cpakfile",
    action  = "store",
    default = None,
    help    = "Optional path to a CPakfile for the project.",
    metavar = "CPAKFILE",
    nargs   = "?",
    type    = str
)

command_parent.add_argument(
    "--local",
    action = "store_true",
    dest   = "local",
    help   = "Performs all operations locally within the project folder."
)

exclusive_parent = argparse.ArgumentParser(add_help = False)
only_group = exclusive_parent.add_mutually_exclusive_group()
only_group.add_argument(
    "--deps-only",
    action = "store_true",
    dest   = "deps_only",
    help   = "Operates only on the dependencies of the project."
)

only_group.add_argument(
    "--plugs-only",
    action = "store_true",
    dest   = "plugs_only",
    help   = "Operates only on the plugins of the project."
)

utility_parent = argparse.ArgumentParser(
    parents  = [parser],
    add_help = False
)

commands_subparser = parser.add_subparsers(
    title       = "CPak actions",
    description = "Valid commands for executing CPak actions.",
    dest        = "action",
    required    = True,
    metavar     = "<command>"
)


# Build specific parser and options.
build_parser = commands_subparser.add_parser(
    "build",
    description = """Handles build specific options and the execution of the build action within
CPak. Searches the current working directory for a CPakfile when one is not provided and begins the
process of building the project specified within the CPakfile. Will implicitly invoke the 'pull'
action to obtain plugins and dependencies that are missing.""",
    parents     = [command_parent, exclusive_parent],
    add_help    = False,
    help        = "Builds a CPak project."
)


# Clean specific parser and options.
clean_parser = commands_subparser.add_parser(
    "clean",
    description = """Handles clean specific options and the execution of the clean action within
CPak. Searches the current working directory for a CPakfile when one is not provided and begins the
process of cleaning the project. Will delete binaries for dependencies and the project build targets
so that they may be built again.""",
    parents     = [command_parent, exclusive_parent],
    add_help    = False,
    help        = "Cleans a CPak project."
)


# Configuration specific parser and options.
config_parser = commands_subparser.add_parser(
    "config",
    description = """Handles configuration specific options and the execution of the config
action within CPak. Requires a property to select. When the property is specified by itself without
a value or option, the property is displayed to the console. When specified with a value, the value
is set and written to the configuration.""",
    parents     = [utility_parent],
    add_help    = False,
    help        = "Configures a CPak project."
)

config_parser.add_argument(
    "property",
    action   = "store",
    help     = "The property to edit in the configuration.",
    metavar  = "PROPERTY",
    nargs    = 1,
    type     = str
)

config_parser.add_argument(
    "value",
    action  = "store",
    default = None,
    help    = "Optional value to set on the property in the configuration.",
    metavar = "VALUE",
    nargs   = "?",
    type    = str
)

config_parser.add_argument(
    "-g",
    "--global",
    action = "store_true",
    dest   = "global",
    help   = "Applies changes to the global configuration."
)

config_parser.add_argument(
    "-l",
    "--list",
    action = "store_true",
    dest   = "display",
    help   = "Displays the configuration to the console."
)

config_parser.add_argument(
    "-u",
    "--unset",
    action = "store_true",
    dest   = "unset",
    help   = "Unsets the provided configuration property."
)


# Init parser and options.
init_parser = commands_subparser.add_parser(
    "init",
    description = """Handles initialize specific options and the execution of the init action within
CPak. Creates CPakfile in the current working directory, or a provided directory.""",
    parents     = [utility_parent],
    add_help    = False,
    help        = "Initializes a CPak project"
)

init_parser.add_argument(
    "project",
    action  = "store",
    help    = """A string which defines the group ID, name, and version of the project. Should have
the form 'gpid:name:semv'.""",
    metavar = "PROJECT",
    nargs   = 1,
    type    = str
)

init_parser.add_argument(
    "--path",
    action   = "store",
    default  = None,
    dest     = "path",
    help     = "The path to create the project at.",
    metavar  = "PATH",
    nargs    = 1,
    type     = str
)


# Install parser and options.
install_parser = commands_subparser.add_parser(
    "install",
    description = """Handles install specific options and the execution of the install action
within CPak. Searches the current working directory for a CPakfile when one is not provided and
begins the process of installing the project specified within the CPakfile. Will create a local CPak
repo and copy the project into that repo.""",
    parents     = [command_parent],
    add_help    = False,
    help        = "Installs a CPak project."
)

install_parser.add_argument(
    "--export-only",
    action  = "store_true",
    dest    = "export_only",
    help    = "Only installs the exportable parts of the project and its CPakfile."
)


# Pull parser and options.
pull_parser = commands_subparser.add_parser(
    "pull",
    description = """Handles pulling specific options and the execution of the pull action within
CPak. Pulls plugins and dependencies from their respective remotes if applicable.""",
    parents     = [command_parent, exclusive_parent],
    add_help    = False,
    help        = "Pulls plugins and dependencies for a CPak project."
)

