import textwrap
import unittest
import unittest.mock
import yaml

from core.cpakfile      import CPakfile
from core.typedefs      import NullableData
from .test_build_info   import TestCPakfileBuildInfoDefinition, test_build_info_prop
from .test_build_target import test_build_flags, test_build_target_name, test_build_target_type, test_path_str
from .test_export_info  import TestCPakfileExportInfoDefinition, test_export_info_prop
from .test_identity     import test_identity_name, test_identity_gpid, test_version_str
from .test_management   import TestCPakfileManagementDefinition, test_management_prop
from .test_parent       import TestCPakfileParentDefinition, test_parent_prop
from .test_platform     import test_platform_type, test_platform_dist, test_platform_arch
from .test_plugin       import test_plugin_conf, test_properties_prop
from .test_profile      import TestCPakfileProfileDefinition, test_profile_name, test_profiles_prop
from .test_project      import TestCPakfileProjectDefinition, test_project_prop
from .test_remote       import test_remote_address, test_remote_username, test_remote_password, test_remote_vcstool, test_remote_branch

test_interpolation_properties = f"""\
properties:
  custom_identity:
    name: {test_identity_name}
    gpid: {test_identity_gpid}
    semv: {test_version_str}
  
  dedicated_platform:
    type: {test_platform_type}
    dist: {test_platform_dist}
    arch: {test_platform_arch}
    semv: {test_version_str}
  
  custom_github:
    repository: {test_remote_address}
    username:   {test_remote_username}
    password:   {test_remote_password}
    vsctool:    {test_remote_vcstool}
    branch:     {test_remote_branch}

  cpak_target:
    name:    {test_build_target_name}
    type:    {test_build_target_type}
    flags:   {test_build_flags}
    threads: 5
"""

# Custom interpolated identity.
test_interpolated_identity_body = """\
  name: "{custom_identity[name]}"
  gpid: "{custom_identity[gpid]}"
  semv: "{custom_identity[semv]}"
"""

# Custom interpolated remote.
test_interpolated_remote_body = """\
  address:  "{custom_github[repository]}"
  username: "{custom_github[username]}"
  password: "{custom_github[password]}"
  vcstool:  "{custom_github[vsctool]}"
  branch:   "{custom_github[branch]}"
"""
test_interpolated_remote_prop = "remote:\n%s" % test_interpolated_remote_body

# Custom interpolated dependency.
test_interpolated_dependency_body = "%s\n%s" % \
        (test_interpolated_identity_body,
         textwrap.indent(test_interpolated_remote_prop, ' ' * 2))
test_interpolated_dependency_item = "-%s" % test_interpolated_dependency_body[1:]

# Custom interpolated plugin.
test_interpolated_plugin_body = "%s\n%s" % \
        (test_interpolated_identity_body[1:], test_plugin_conf)
test_interpolated_plguin_item = "-%s\n" % test_interpolated_plugin_body

# Custom interpolated management.
test_interpolated_management_body = "plugins:\n%s\ndepends:\n%s" % \
        (test_interpolated_plguin_item, test_interpolated_dependency_item)
test_interpolated_management_prop = "management:\n%s" % \
        textwrap.indent(test_interpolated_management_body, ' ' * 2)

# Custom interpolated platform.
test_interpolated_platform_body = """\
  type: "{dedicated_platform[type]}"
  dist: "{dedicated_platform[dist]}"
  arch: "{dedicated_platform[arch]}"
  semv: "{dedicated_platform[semv]}"
"""
test_interpolated_platform_prop = "platform:\n%s" % \
        textwrap.indent(test_interpolated_platform_body, ' ' * 2)

# Custom interpolated build target.
test_interpolated_build_target_body = """\
  name:  "{cpak_target[name]}"
  type:  "{cpak_target[type]}"
  flags: "{cpak_target[flags]}"
  sources:
  - %s
  includes:
  - %s
  libraries:
  - %s\
""" % (test_path_str, test_path_str, test_path_str)
test_interpolated_build_target_item = "-%s" % test_interpolated_build_target_body[1:]

# Custom interpolated build info.
test_interpolated_build_info_body = """\
  threads:   "{cpak_target[threads]}"
  flags:     "{cpak_target[flags]}"
  includes:  [%s]
  libraries: [%s]
  targets:\n%s
""" % (test_path_str, test_path_str, textwrap.indent(test_interpolated_build_target_item, ' ' * 2))
test_interpolated_build_info_prop = "build:\n%s" % test_interpolated_build_info_body

# Custom interpolated activation.
test_interpolated_activation_body = "  mode: debug\n  %s" % test_interpolated_platform_prop
test_interpolated_activation_prop = "activation:\n%s" % test_interpolated_activation_body

# Custom interpolated profile.
test_interpolated_profile_body = "name: %s\n%s\n%s\n%s\n%s" % \
        (test_profile_name,
         textwrap.indent(test_interpolated_activation_prop, ' ' * 2),
         textwrap.indent(test_properties_prop, ' ' * 2),
         textwrap.indent(test_interpolated_management_prop, ' ' * 2),
         textwrap.indent(test_interpolated_build_info_prop, ' ' * 2))

test_interpolated_profile_item = "- %s" % test_interpolated_profile_body
test_interpolated_profiles_prop = "profiles:\n%s" % \
        textwrap.indent(test_interpolated_profile_item, ' ' * 2)

# Custom interpolated export info.
test_interpolated_export_info_body = """\
  includes:  [%s]
  libraries: [%s]
""" % (test_path_str, test_path_str)
test_interpolated_export_info_prop = "export:\n%s" % test_interpolated_export_info_body



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
{test_interpolation_properties}
{test_interpolated_profiles_prop}
{test_interpolated_management_prop}
{test_interpolated_build_info_prop}
{test_interpolated_export_info_prop}\
"""
    
    # TODO: add parented test.
    test_yaml_Parented = f"""\
!CPakfile
{test_parent_prop}
{test_project_prop}
{test_profiles_prop}
{test_management_prop}
{test_build_info_prop}
{test_export_info_prop}\
""" 
    
    test_yaml_NoProject = f"!CPakfile\n{test_build_info_prop}"
    test_yaml_NoBuild   = f"!CPakfile\n{test_project_prop}"

    @unittest.mock.patch('core.cpakfile.load_parent_cpakfile')
    def test_load_positive(self, mock_load_parent_cpakfile):
        TestCPakfileDefinition.validate(self, yaml.safe_load(self.test_yaml_Normal))
        TestCPakfileDefinition.validate(self, yaml.safe_load(self.test_yaml_Interpolated))
        
        # Special case must mock function
        mock_load_parent_cpakfile.return_value = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileDefinition.validate_parented(self, yaml.safe_load(self.test_yaml_Parented))

    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoProject)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoBuild)

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
    def validate_parented(test: unittest.TestCase, cpf: NullableData[CPakfile]):
        TestCPakfileDefinition.validate(test, cpf)

        test.assertIsNotNone(cpf.parentref)                        # type: ignore
        TestCPakfileParentDefinition.validate(test, cpf.parentref) # type: ignore

        test.assertIsNotNone(cpf.parent)                  # type: ignore
        TestCPakfileDefinition.validate(test, cpf.parent) # type: ignore

        
