import textwrap
import unittest
import yaml

from core.cpakfile import Remote
from core.typedefs import NullableData


# Test data for cpakfile.SourceControl tests.
test_remote_address  = "https://github.com/SoraKatadzuma/CPak.git"
test_remote_username = "xXMyUsernameXx"
test_remote_password = "3xt7em31yS@fePa55wd"
test_remote_vcstool  = "git"
test_remote_branch   = "master"
test_remote_body     = f"""\
  address:  {test_remote_address}
  username: {test_remote_username}
  password: {test_remote_password}
  vcstool:  {test_remote_vcstool}
  branch:   {test_remote_branch}
"""
test_remote_prop = f"""\
  remote:\n{textwrap.indent(test_remote_body, ' ' * 2)}\
"""



class TestCPakfileRemoteDefinition(unittest.TestCase):
    test_yaml_Empty      = f"remote: !Remote"
    test_yaml_Normal     = f"remote: !Remote\n{test_remote_body}"
    test_yaml_NoAddress  = f"remote: !Remote\n  username: {test_remote_username}\n  password: {test_remote_password}"
    test_yaml_NoUsername = f"remote: !Remote\n  address: {test_remote_address}\n  password: {test_remote_password}"
    test_yaml_NoPassword = f"remote: !Remote\n  address: {test_remote_address}\n  username: {test_remote_username}"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileRemoteDefinition.validate(self, mapping["remote"])

    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoAddress)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoUsername)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoPassword)

    @staticmethod
    def validate(test: unittest.TestCase, vcsd: NullableData[Remote]):
        test.assertIsNotNone(vcsd)
        test.assertIsInstance(vcsd, Remote)
        test.assertEqual(vcsd.address, test_remote_address)   # type: ignore
        test.assertEqual(vcsd.username, test_remote_username) # type: ignore
        test.assertEqual(vcsd.password, test_remote_password) # type: ignore
        test.assertEqual(vcsd.vcstool, test_remote_vcstool)   # type: ignore
        test.assertEqual(vcsd.branch, test_remote_branch)     # type: ignore
        test.assertTrue(vcsd.shallow)                         # type: ignore


