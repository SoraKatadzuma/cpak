import textwrap
import unittest
import yaml

from cpakfile           import BuildInfo, NullableData
from .test_build_target import (
    TestCPakfileBuildTargetDefinition,
    test_build_flags,
    test_build_target_item,
    test_path_obj,
    test_path_str
)

# Test data for cpakfile.BuildInfo tests.
test_build_info_body = f"""\
  threads:   5
  flags:     {test_build_flags}
  includes:  [{test_path_str}]
  libraries: [{test_path_str}]
  targets:\n{textwrap.indent(test_build_target_item, ' ' * 2)}\
"""
test_build_info_prop = f"build:\n{test_build_info_body}"



class TestCPakfileBuildInfoDefinition(unittest.TestCase):
    test_yaml_Empty     = f"build: !BuildInfo"
    test_yaml_Normal    = f"build: !BuildInfo\n{test_build_info_body}"
    test_yaml_NoTargets = f"build: !BuildInfo\n  threads: 1"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileBuildInfoDefinition.validate(self, mapping["build"])

    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoTargets)

    @staticmethod
    def validate(test: unittest.TestCase, bldi: NullableData[BuildInfo]):
        test.assertIsNotNone(bldi)
        test.assertIsInstance(bldi, BuildInfo)
        test.assertEqual(bldi.threads, 5)              # type: ignore
        test.assertEqual(bldi.flags, test_build_flags) # type: ignore

        test.assertIsNotNone(bldi.includes)               # type: ignore
        test.assertEqual(len(bldi.includes), 1)           # type: ignore
        test.assertEqual(bldi.includes[0], test_path_obj) # type: ignore

        test.assertIsNotNone(bldi.libraries)               # type: ignore
        test.assertEqual(len(bldi.libraries), 1)           # type: ignore
        test.assertEqual(bldi.libraries[0], test_path_obj) # type: ignore

        test.assertIsNotNone(bldi.targets)                                # type: ignore
        test.assertEqual(len(bldi.targets), 1)                            # type: ignore
        TestCPakfileBuildTargetDefinition.validate(test, bldi.targets[0]) # type: ignore


