import textwrap
import unittest
import yaml

from core.cpakfile  import Activation, ActivationMode
from core.typedefs  import NullableData
from .test_platform import (
    TestCPakfilePlatformDefinition,
    test_platform_prop,
    test_platform_type
)

# Test data for cpakfile.Activation tests.
test_activation_mode = ActivationMode.DEBUG.name.lower()
test_activation_body = f"""\
  mode: {test_activation_mode}
{textwrap.indent(test_platform_prop, ' ' * 2)}
"""
test_activation_prop = f"activation:\n{test_activation_body}"



class TestCPakfileActivationDefinition(unittest.TestCase):
    test_yaml_Empty  = f"activation: !Activation"
    test_yaml_Normal = f"activation: !Activation\n{test_activation_body}"
    test_yaml_NoMode = f"activation: !Activation\n  platform:\n    type: {test_platform_type}"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileActivationDefinition.validate(self, mapping["activation"])

    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoMode)

    @staticmethod
    def validate(test: unittest.TestCase, actn: NullableData[Activation]):
        test.assertIsNotNone(actn)
        test.assertIsInstance(actn, Activation)
        test.assertEqual(actn.mode, ActivationMode.DEBUG)            # type: ignore
        TestCPakfilePlatformDefinition.validate(test, actn.platform) # type: ignore


