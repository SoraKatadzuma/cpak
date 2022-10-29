import textwrap
import unittest
import yaml

from core.cpakfile    import ParentReference
from core.typedefs    import NullableData
from .test_build_info import test_path_obj, test_path_str
from .test_identity   import (
    TestCPakfileIdentityDefinition,
    test_identity_body,
    test_identity_gpid,
    test_identity_name,
    test_version_str
)

# Test data for cpakfile.Project tests.
test_parent_body = f"""\
{test_identity_body}
  path: {test_path_str}
"""
test_parent_prop = f"parentref:\n{textwrap.indent(test_parent_body, ' ' * 2)}\n"



class TestCPakfileParentDefinition(unittest.TestCase):
    test_yaml_Empty  = f"parent: !ParentReference"
    test_yaml_Normal = f"parent: !ParentReference\n{test_parent_body}"
    test_yaml_NoName = f"parent: !ParentReference\n  gpid: {test_identity_gpid}\n  semv: {test_version_str}"
    test_yaml_NoGPID = f"parent: !ParentReference\n  name: {test_identity_name}\n  semv: {test_version_str}"
    test_yaml_NoSemV = f"parent: !ParentReference\n  name: {test_identity_name}\n  gpid: {test_identity_gpid}"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileParentDefinition.validate(self, mapping["parent"])

    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoName)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoGPID)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoSemV)

    @staticmethod
    def validate(test: unittest.TestCase, prnt: NullableData[ParentReference]):
        test.assertIsInstance(prnt, ParentReference)
        TestCPakfileIdentityDefinition.validate(test, prnt)
        
        test.assertIsNotNone(prnt.path)            # type: ignore
        test.assertEqual(prnt.path, test_path_obj) # type: ignore


