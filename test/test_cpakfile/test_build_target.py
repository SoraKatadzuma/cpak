import pathlib
import unittest
import yaml

from cpakfile import BuildTarget, BuildType, BuildToolChain, NullableData

# Test data for paths in the yaml.
test_path_str = "/path/to/file"
test_path_obj = pathlib.Path(test_path_str)

# Test data for flags in the yaml.
test_build_flags = "-g -O0"

# Test data for cpakfile.BuildTarget tests.
test_build_target_name      = "libcpak"
test_build_target_type      = BuildType.LIBRARY.name.lower()
test_build_target_toolchain = BuildToolChain.GCC.name.lower()
test_build_target_body = f"""\
  name:      {test_build_target_name}
  type:      {test_build_target_type}
  flags:     {test_build_flags}
  toolchain: {test_build_target_toolchain}
  sources:
  - {test_path_str}
  includes:
  - {test_path_str}
  libraries:
  - {test_path_str}
"""
test_build_target_item = f"-{test_build_target_body[1:]}"


class TestCPakfileBuildTargetDefinition(unittest.TestCase):
    test_yaml_Empty       = f"target: !BuildTarget"
    test_yaml_Normal      = f"target: !BuildTarget\n{test_build_target_body}"
    test_yaml_NoName      = f"target: !BuildTarget\n  type: {test_build_target_type}\n  toolchain: {test_build_target_toolchain}\n  sources: [{test_path_str}]"
    test_yaml_NoType      = f"target: !BuildTarget\n  name: {test_build_target_name}\n  toolchain: {test_build_target_toolchain}\n  sources: [{test_path_str}]"
    test_yaml_NoToolchain = f"target: !BuildTarget\n  name: {test_build_target_name}\n  type: {test_build_target_type}\n  sources: [{test_path_str}]"
    test_yaml_NoSources   = f"target: !BuildTarget\n  name: {test_build_target_name}\n  type: {test_build_target_type}\n  toolchain: {test_build_target_toolchain}\n"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileBuildTargetDefinition.validate(self, mapping["target"])


    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoName)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoType)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoToolchain)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoSources)


    @staticmethod
    def validate(test: unittest.TestCase, bldt: NullableData[BuildTarget]):
        test.assertIsNotNone(bldt)
        test.assertIsInstance(bldt, BuildTarget)
        test.assertEqual(bldt.name, test_build_target_name)     # type: ignore
        test.assertEqual(bldt.type, BuildType.LIBRARY) # type: ignore
        test.assertEqual(bldt.flags, test_build_flags)          # type: ignore

        test.assertIsNotNone(bldt.sources)               # type: ignore
        test.assertEqual(len(bldt.sources), 1)           # type: ignore
        test.assertEqual(bldt.sources[0], test_path_obj) # type: ignore

        test.assertIsNotNone(bldt.includes)               # type: ignore
        test.assertEqual(len(bldt.includes), 1)           # type: ignore
        test.assertEqual(bldt.includes[0], test_path_obj) # type: ignore

        test.assertIsNotNone(bldt.libraries)               # type: ignore
        test.assertEqual(len(bldt.libraries), 1)           # type: ignore
        test.assertEqual(bldt.libraries[0], test_path_obj) # type: ignore


