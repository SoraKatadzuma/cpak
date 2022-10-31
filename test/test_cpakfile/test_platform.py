import unittest
import yaml

from cpakfile       import Platform, NullableData
from .test_identity import test_version_obj, test_version_str


# Test data for cpakfile.Platform tests.
test_platform_type = "unix"
test_platform_dist = "ubuntu"
test_platform_arch = "amd64"
test_platform_body = f"""\
  type: {test_platform_type}
  dist: {test_platform_dist}
  arch: {test_platform_arch}
  semv: {test_version_str}\
"""
test_platform_prop = f"platform:\n{test_platform_body}"



class TestCPakfilePlatformDefinition(unittest.TestCase):
    test_yaml_Empty  = f"platform: !Platform"
    test_yaml_Normal = f"platform: !Platform\n{test_platform_body}"
    test_yaml_NoType = f"platform: !Platform\n  dist: {test_platform_dist}"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfilePlatformDefinition.validate(self, mapping["platform"])

    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoType)

    @staticmethod
    def validate(test: unittest.TestCase, plat: NullableData[Platform]):
        test.assertIsNotNone(plat)
        test.assertIsInstance(plat, Platform)
        test.assertEqual(plat.type, test_platform_type) # type: ignore
        test.assertEqual(plat.dist, test_platform_dist) # type: ignore
        test.assertEqual(plat.arch, test_platform_arch) # type: ignore
        test.assertEqual(plat.semv, test_version_obj)   # type: ignore


