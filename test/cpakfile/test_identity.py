import unittest
import yaml

from core.cpakfile    import Identity
from core.typedefs    import NullableData
from semantic_version import Version

# Test data for versions in the yaml.
test_version_str = "1.0.0-alpha"
test_version_obj = Version(test_version_str)

# Test data for cpakfile.Identity tests.
test_identity_name = "CPak"
test_identity_gpid = "Simular::Tech"
test_identity_body = f"""\
  name: {test_identity_name}
  gpid: {test_identity_gpid}
  semv: {test_version_str}\
"""


class TestCPakfileIdentityDefinition(unittest.TestCase):
    test_yaml_Empty  = f"identity: !Identity"
    test_yaml_Normal = f"identity: !Identity\n{test_identity_body}"
    test_yaml_NoName = f"identity: !Identity\n  gpid: {test_identity_gpid}\n  semv: {test_version_str}"
    test_yaml_NoGPID = f"identity: !Identity\n  name: {test_identity_name}\n  semv: {test_version_str}"
    test_yaml_NoSemV = f"identity: !Identity\n  name: {test_identity_name}\n  gpid: {test_identity_gpid}"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileIdentityDefinition.validate(self, mapping["identity"])

    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoName)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoGPID)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoSemV)

    @staticmethod
    def validate(test: unittest.TestCase, idtt: NullableData[Identity]):
        test.assertIsNotNone(idtt)
        test.assertIsInstance(idtt, Identity)
        test.assertEqual(idtt.name, test_identity_name) # type: ignore
        test.assertEqual(idtt.gpid, test_identity_gpid) # type: ignore
        test.assertEqual(idtt.semv, test_version_obj)   # type: ignore


