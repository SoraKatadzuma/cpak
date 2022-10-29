import textwrap
import unittest
import yaml

from core.cpakfile    import Management
from core.typedefs    import NullableData
from .test_dependency import TestCPakfileDependencyDefinition, test_dependency_item
from .test_plugin     import TestCPakfilePluginDefinition, test_plugin_item


# Test data for cpakfile.Management tests.
test_management_body = f"""\
  plugins:\n{textwrap.indent(test_plugin_item, ' ' * 2)}\n
  depends:\n{textwrap.indent(test_dependency_item, ' ' * 2)}\
"""
test_management_prop = f"management:\n{test_management_body}"



class TestCPakfileManagementDefinition(unittest.TestCase):
    test_yaml_Empty  = f"management: !Management"
    test_yaml_Normal = f"management: !Management\n{test_management_body}"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileManagementDefinition.validate(self, mapping["management"])

    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)

    @staticmethod
    def validate(test: unittest.TestCase, mgmt: NullableData[Management]):
        test.assertIsNotNone(mgmt)
        test.assertIsInstance(mgmt, Management)

        test.assertIsNotNone(mgmt.plugins)                           # type: ignore
        test.assertEqual(len(mgmt.plugins), 1)                       # type: ignore
        TestCPakfilePluginDefinition.validate(test, mgmt.plugins[0]) # type: ignore

        test.assertIsNotNone(mgmt.depends)                                # type: ignore
        test.assertEqual(len(mgmt.depends), 1)                            # type: ignore
        TestCPakfileDependencyDefinition.validate(test, mgmt.depends[0]) # type: ignore


