import pathlib
import unittest
import yaml

from cpakfile         import NullableData
from core.application import DEFAULT_REPO_PATH, Configuration

test_repoloc_str = DEFAULT_REPO_PATH
test_repoloc_obj = pathlib.Path(test_repoloc_str)


class TestApplicationConfigurationDefinition(unittest.TestCase):
    test_yaml_Empty  = "!CPakConfiguration"
    test_yaml_Normal = f"""\
!CPakConfiguration

repoloc: {test_repoloc_str}\
"""

    def test_load_positive(self):
        TestApplicationConfigurationDefinition.validate(self, yaml.safe_load(self.test_yaml_Normal))


    def test_load_negative(self):
        pass


    def test_load_from_path(self):
        path   = pathlib.Path.cwd() / "test/test_application/test_config.yaml"
        result = Configuration.load_config(path)

        self.assertTrue(path.exists())
        self.assertIsNotNone(result)
        TestApplicationConfigurationDefinition.validate(self, result)


    def test_build_to_path(self):
        path   = pathlib.Path.cwd() / "temp/config.yaml"
        result = Configuration.build_config(path)

        self.assertTrue(path.exists())
        self.assertIsNotNone(result)
        TestApplicationConfigurationDefinition.validate(self, result)
        pathlib.Path.unlink(path)
        pathlib.Path.rmdir(path.parent)


    @staticmethod
    def validate(test: unittest.TestCase, conf: NullableData[Configuration]):
        test.assertIsNotNone(conf)
        test.assertEqual(conf.repoloc, test_repoloc_obj) # type: ignore
