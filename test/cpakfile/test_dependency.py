import unittest
import yaml

from core.cpakfile  import Dependency
from core.typedefs  import NullableData
from .test_remote   import TestCPakfileRemoteDefinition, test_remote_prop
from .test_identity import (
    test_identity_body,
    test_identity_gpid,
    test_identity_name,
    test_version_obj,
    test_version_str
)


# Test data for cpakfile.Dependency tests.
test_dependency_body = f"""\
{test_identity_body}
{test_remote_prop}
"""
test_dependency_item = f"-{test_dependency_body[1:]}"



class TestCPakfileDependencyDefinition(unittest.TestCase):
    test_yaml_Empty  = f"dependency: !Dependency"
    test_yaml_Normal = f"dependency: !Dependency\n{test_dependency_body}"
    test_yaml_NoName = f"dependency: !Dependency\n  gpid: {test_identity_gpid}\n  semv: {test_version_str}"
    test_yaml_NoGPID = f"dependency: !Dependency\n  name: {test_identity_name}\n  semv: {test_version_str}"
    test_yaml_NoSemV = f"dependency: !Dependency\n  name: {test_identity_name}\n  gpid: {test_identity_gpid}"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileDependencyDefinition.validate(self, mapping["dependency"])

    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoName)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoGPID)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoSemV)

    @staticmethod
    def validate(test: unittest.TestCase, depy: NullableData[Dependency]):
        test.assertIsNotNone(depy)
        test.assertIsInstance(depy, Dependency)
        test.assertEqual(depy.name, test_identity_name)          # type: ignore
        test.assertEqual(depy.gpid, test_identity_gpid)          # type: ignore
        test.assertEqual(depy.semv, test_version_obj)            # type: ignore
        TestCPakfileRemoteDefinition.validate(test, depy.remote) # type: ignore


