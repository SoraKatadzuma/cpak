import unittest
import yaml

from cpakfile           import ExportInfo, NullableData
from .test_build_target import test_path_obj, test_path_str


# Test data for cpakfile.ExportInfo tests.
test_export_info_body = f"""\
  includes:  [{test_path_str}]
  libraries: [{test_path_str}]\
"""
test_export_info_prop = f"export:\n{test_export_info_body}"



class TestCPakfileExportInfoDefinition(unittest.TestCase):
    test_yaml_Empty  = f"export: !ExportInfo"
    test_yaml_Normal = f"export: !ExportInfo\n{test_export_info_body}"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileExportInfoDefinition.validate(self, mapping["export"])

    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)

    @staticmethod
    def validate(test: unittest.TestCase, expt: NullableData[ExportInfo]):
        test.assertIsNotNone(expt)
        test.assertIsInstance(expt, ExportInfo)

        test.assertIsNotNone(expt.includes)               # type: ignore
        test.assertEqual(len(expt.includes), 1)           # type: ignore
        test.assertEqual(expt.includes[0], test_path_obj) # type: ignore

        test.assertIsNotNone(expt.libraries)               # type: ignore
        test.assertEqual(len(expt.libraries), 1)           # type: ignore
        test.assertEqual(expt.libraries[0], test_path_obj) # type: ignore


