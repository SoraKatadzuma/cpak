import argparse
import logging
import platform
import os
import sys
import typing
import yaml

from pathlib import Path

from .cpakfile import CPakfile


MAJOR = 1
MINOR = 0
PATCH = 0
LOGO  = """
 ::::::::  :::::::::      :::     :::    :::
:+:    :+: :+:    :+:   :+: :+:   :+:   :+:
+:+        +:+    +:+  +:+   +:+  +:+  +:+
+#+        +#++:++#+  +#++:++#++: +#++:++
+#+        +#+        +#+     +#+ +#+  +#+
#+#    #+# #+#        #+#     #+# #+#   #+#
 ########  ###        ###     ### ###    ###"""

VERSION     = f"v{MAJOR}.{MINOR}.{PATCH}".rjust(45)
DESCRIPTION = """
Configuration as code build tool for C/C++ designed to
make it easier to build your projects in every single way."""

LOG_FORMAT  = "[%(asctime)s] %(name)s | %(levelname)s: %(message)s"
TIME_FORMAT = "%H:%M:%S"



@typing.final
class Application:
    def __init__(self) -> None:
        self.__verbose = False
        self.__prepare_logger()
        self.__prepare_options()
        self.__load_config()
        print('\n'.join([LOGO,VERSION]) + "\n")

    @property
    def log(self) -> logging.Logger:
        return self.__logger

    @property
    def config(self) -> CPakfile:
        return self.__config

    def read_arguments(self) -> None:
        arguments = self.__base.parse_args()
        if hasattr(arguments, "verbose"):
            self.__verbose = True
            self.__logger.setLevel(logging.DEBUG)

    def __load_config(self) -> None:
        cfgdir = Path.home() / ".cpak/config.yaml"
        if not cfgdir.exists():
            self.__build_config(cfgdir)
            self.__logger.warn("No user configuration found, created default.")

        with cfgdir.open() as source:
            self.__config = yaml.safe_load(source)

    def __build_config(self, cfgdir: Path) -> None:
        ...

    def __prepare_logger(self) -> None:
        self.__logger    = logging.getLogger("cpak")
        self.__handler   = logging.StreamHandler(sys.stdout)
        self.__formatter = logging.Formatter(LOG_FORMAT, TIME_FORMAT)
        self.__handler.setFormatter(self.__formatter)
        self.__logger.addHandler(self.__handler)
        self.__logger.setLevel(logging.INFO)

    def __prepare_options(self) -> None:
        self.__base = argparse.ArgumentParser(
            description     = DESCRIPTION,
            formatter_class = argparse.RawTextHelpFormatter,
            add_help        = False
        )

        self.__options = self.__base.add_argument_group(
            title       = "Universal Options",
            description = "Options that can be used by all commands."
        )

        self.__options.add_argument(
            "-h",
            "--help",
            action = "help",
            help   = "Prints this help message and exits."
        )

        self.__options.add_argument(
            "-v",
            "--verbose",
            action = "store_true",
            help   = "Provides more descriptive console output."
        )

        self.__parser = argparse.ArgumentParser(
            parents  = [self.__base],
            add_help = False
        )

        self.__commands = self.__base.add_subparsers(
            title       = "CPak actions",
            description = "Valid commands for executing CPak actions.",
            dest        = "action",
            required    = True,
            metavar     = "<command>"
        )


cpakapp = Application()
