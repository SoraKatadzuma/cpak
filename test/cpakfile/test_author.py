import unittest
import yaml

from core.cpakfile import Author
from core.typedefs import NullableData

# Test data for cpakfile.Author tests.
test_author_name  = "John Doe"
test_author_email = "jdoe@example.com"
test_author_body  = f"""\
  name: {test_author_name}
  email: {test_author_email}\
"""
test_author_item = f"-{test_author_body[1:]}"



class TestCPakfileAuthorDefinition(unittest.TestCase):
    test_yaml_Empty   = f"author: !Author"
    test_yaml_Normal  = f"author: !Author\n{test_author_body}"
    test_yaml_NoName  = f"author: !Author\n  email: {test_author_email}"
    test_yaml_NoEmail = f"author: !Author\n  name: {test_author_name}"

    def test_load_positive(self):
        mapping: dict = yaml.safe_load(self.test_yaml_Normal)
        TestCPakfileAuthorDefinition.validate(self, mapping["author"])
        
    def test_load_negative(self):
        self.assertRaises(yaml.YAMLError, yaml.safe_load, self.test_yaml_Empty)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoName)
        self.assertRaises(TypeError, yaml.safe_load, self.test_yaml_NoEmail)

    @staticmethod
    def validate(test: unittest.TestCase, auth: NullableData[Author]):
        test.assertIsNotNone(auth)
        test.assertIsInstance(auth, Author)
        test.assertEqual(auth.name, test_author_name)   # type: ignore
        test.assertEqual(auth.email, test_author_email) # type: ignore


