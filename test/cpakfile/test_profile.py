import textwrap
import unittest
import yaml

from core.cpakfile    import Profile
from core.typedefs    import NullableData
from .test_activation import TestCPakfileActivationDefinition, test_activation_mode, test_activation_prop
from .test_build_info import TestCPakfileBuildInfoDefinition, test_build_info_prop
from .test_management import TestCPakfileManagementDefinition, test_management_prop
from .test_plugin     import test_properties_prop, test_property_obj

# Test data for cpakfile.Profile tests.
test_profile_name = "linux64-debug"
test_profile_body = f"""\
  name: {test_profile_name}
{textwrap.indent(test_activation_prop, ' ' * 2)}
{textwrap.indent(test_properties_prop, ' ' * 2)}
{textwrap.indent(test_management_prop, ' ' * 2)}
{textwrap.indent(test_build_info_prop, ' ' * 2)}\
"""
test_profile_item  = f"-{test_profile_body[1:]}"
test_profiles_prop = f"profiles:\n{textwrap.indent(test_profile_item, ' ' * 2)}"



class TestCPakfileProfileDefinition(unittest.TestCase):
    test_yaml_Empty        = f"profile: !Profile"
    test_yaml_Normal       = f"profile: !Profile\n{test_profile_body}"
    test_yaml_NoName       = f"profile: !Profile\n  activation:\n    mode: {test_activation_mode}"
    test_yaml_NoActivation = f"profile: !Profile\n  name: {test_profile_name}"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileProfileDefinition.validate(self, mapping["profile"])

    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoName)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoActivation)

    @staticmethod
    def validate(test: unittest.TestCase, prfl: NullableData[Profile]):
        test.assertIsNotNone(prfl)
        test.assertIsInstance(prfl, Profile)
        test.assertEqual(prfl.name, test_profile_name)                   # type: ignore
        TestCPakfileActivationDefinition.validate(test, prfl.activation) # type: ignore

        test.assertIsNotNone(prfl.properties)                            # type: ignore
        test.assertEqual(prfl.properties, test_property_obj)             # type: ignore
        TestCPakfileBuildInfoDefinition.validate(test, prfl.build)       # type: ignore
        TestCPakfileManagementDefinition.validate(test, prfl.management) # type: ignore


