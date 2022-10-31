import textwrap
import unittest
import yaml

from cpakfile       import Project, ProjectType, NullableData
from .test_author   import TestCPakfileAuthorDefinition, test_author_item
from .test_identity import (
    TestCPakfileIdentityDefinition,
    test_identity_body,
    test_identity_gpid,
    test_identity_name,
    test_version_str
)

# Test data for cpakfile.Project tests.
test_project_type = ProjectType.DEFAULT.name.lower()
test_project_desc = """\
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore
et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut
aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse
cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in
culpa qui officia deserunt mollit anim id est laborum.
"""
test_project_home = "https://github.com/SoraKatadzuma/cpak.git"
test_project_license = "MIT"
test_project_body = f"""\
{test_identity_body}
  type: {test_project_type}
  desc: |\n{textwrap.indent(test_project_desc, ' ' * 4)}
  home: {test_project_home}
  license: {test_project_license}
  authors:\n{textwrap.indent(test_author_item, ' ' * 2)}\
"""
test_project_prop = f"project:\n{test_project_body}\n"



class TestCPakfileProjectDefinition(unittest.TestCase):
    test_yaml_Empty  = f"project: !Project"
    test_yaml_Normal = f"project: !Project\n{test_project_body}"
    test_yaml_NoName = f"project: !Project\n  gpid: {test_identity_gpid}\n  semv: {test_version_str}"
    test_yaml_NoGPID = f"project: !Project\n  name: {test_identity_name}\n  semv: {test_version_str}"
    test_yaml_NoSemV = f"project: !Project\n  name: {test_identity_name}\n  gpid: {test_identity_gpid}"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileProjectDefinition.validate(self, mapping["project"])

    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoName)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoGPID)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoSemV)

    @staticmethod
    def validate(test: unittest.TestCase, proj: NullableData[Project]):
        TestCPakfileIdentityDefinition.validate(test, proj)

        test.assertEqual(proj.type, ProjectType.DEFAULT)             # type: ignore
        test.assertEqual(proj.desc, test_project_desc)               # type: ignore
        test.assertEqual(proj.home, test_project_home)               # type: ignore
        test.assertEqual(proj.license, test_project_license)         # type: ignore
        TestCPakfileAuthorDefinition.validate(test, proj.authors[0]) # type: ignore


