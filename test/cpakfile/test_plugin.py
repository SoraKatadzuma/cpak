import unittest
import yaml

from core.cpakfile  import Plugin
from core.typedefs  import NullableData
from .test_identity import (
    test_identity_body,
    test_identity_gpid,
    test_identity_name,
    test_version_obj,
    test_version_str
)

# Test data for properties in the yaml.
test_property_str    = "custom: 1"
test_property_obj    = { "custom": 1 }
test_properties_prop = f"""\
properties:
  {test_property_str}
"""

# Test data for cpakfile.Plugin tests.
test_plugin_conf = f"""\
  config:
    {test_property_str}\
"""
test_plugin_body = f"{test_identity_body}\n{test_plugin_conf}"
test_plugin_item = f"-{test_plugin_body[1:]}"



class TestCPakfilePluginDefinition(unittest.TestCase):
    test_yaml_Empty  = f"plugin: !Plugin"
    test_yaml_Normal = f"plugin: !Plugin\n{test_plugin_body}"
    test_yaml_NoName = f"plugin: !Plugin\n  gpid: {test_identity_gpid}\n  semv: {test_version_str}"
    test_yaml_NoGPID = f"plugin: !Plugin\n  name: {test_identity_name}\n  semv: {test_version_str}"
    test_yaml_NoSemV = f"plugin: !Plugin\n  name: {test_identity_name}\n  gpid: {test_identity_gpid}"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfilePluginDefinition.validate(self, mapping["plugin"])

    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoName)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoGPID)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoSemV)

    @staticmethod
    def validate(test: unittest.TestCase, plgn: NullableData[Plugin]):
        test.assertIsNotNone(plgn)
        test.assertIsInstance(plgn, Plugin)
        test.assertIsNotNone(plgn.config) # type: ignore

        test.assertEqual(plgn.name, test_identity_name)  # type: ignore
        test.assertEqual(plgn.gpid, test_identity_gpid)  # type: ignore
        test.assertEqual(plgn.semv, test_version_obj)    # type: ignore
        test.assertEqual(plgn.config, test_property_obj) # type: ignore


