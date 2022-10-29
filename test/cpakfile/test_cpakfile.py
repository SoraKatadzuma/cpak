import unittest
import unittest.mock
import yaml

from core.cpakfile    import CPakfile, _load_parent_cpakfile
from core.typedefs    import NullableData
from .test_identity   import test_version_obj, test_version_str
from .test_parent     import TestCPakfileParentDefinition, test_parent_prop
from .test_profile    import TestCPakfileProfileDefinition, test_profiles_prop
from .test_project    import TestCPakfileProjectDefinition, test_project_prop
from .test_build_info import TestCPakfileBuildInfoDefinition, test_build_info_prop
from .test_management import TestCPakfileManagementDefinition, test_management_prop
from .test_export_info import TestCPakfileExportInfoDefinition, test_export_info_prop

test_custom_props = f"""\
properties:
    custom_property: 1
"""


class TestCPakfileDefinition(unittest.TestCase):
    test_yaml_Empty  = f"!CPakfile"
    test_yaml_Normal = f"""\
!CPakfile
{test_project_prop}
{test_profiles_prop}
{test_management_prop}
{test_build_info_prop}
{test_export_info_prop}\
"""
    
    # TODO: add interpolated test.
    test_yaml_Interpolated = f"""\
!CPakfile
{test_project_prop}
{test_custom_props}
{test_profiles_prop}
{test_management_prop}
{test_build_info_prop}
{test_export_info_prop}\
"""
    
    # TODO: add parented test.
    test_yaml_Parented = f"""\
!CPakfile
{test_parent_prop}
{test_project_prop}
{test_custom_props}
{test_profiles_prop}
{test_management_prop}
{test_build_info_prop}
{test_export_info_prop}\
""" 
    
    test_yaml_NoProject = f"!CPakfile{test_build_info_prop}"
    test_yaml_NoBuild   = f"!CPakfile{test_project_prop}"

    @unittest.mock.patch('core.cpakfile._load_parent_cpakfile')
    def test_load_positive(self, mock_load_parent_cpakfile):
        mapping: CPakfile = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileDefinition.validate(self, mapping)
        TestCPakfileDefinition.validate_interpolated(self, yaml.safe_load(self.test_yaml_Interpolated))
        
        # Special case must mock function
        mock_load_parent_cpakfile.return_value = mapping
        TestCPakfileDefinition.validate_parented(self, yaml.safe_load(self.test_yaml_Parented))

    def test_load_negative(self):
        pass

    @staticmethod
    def validate(test: unittest.TestCase, cpf: NullableData[CPakfile]):
        test.assertIsNotNone(cpf)
        test.assertIsInstance(cpf, CPakfile)
        TestCPakfileProjectDefinition.validate(test, cpf.project)       # type: ignore
        TestCPakfileBuildInfoDefinition.validate(test, cpf.build)       # type: ignore
        TestCPakfileManagementDefinition.validate(test, cpf.management) # type: ignore
        TestCPakfileExportInfoDefinition.validate(test, cpf.export)     # type: ignore

        test.assertIsNotNone(cpf.profiles)                            # type: ignore
        test.assertEqual(len(cpf.profiles), 1)                        # type: ignore
        TestCPakfileProfileDefinition.validate(test, cpf.profiles[0]) # type: ignore 

    @staticmethod
    def validate_interpolated(test: unittest.TestCase, cpf: NullableData[CPakfile]):
        pass

    @staticmethod
    def validate_parented(test: unittest.TestCase, cpf: NullableData[CPakfile]):
        TestCPakfileDefinition.validate(test, cpf)

        parent = getattr(cpf, "parent")
        test.assertIsNotNone(parent)
        TestCPakfileDefinition.validate(test, getattr(cpf, "parent"))

