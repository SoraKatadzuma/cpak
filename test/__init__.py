import unittest

loader = unittest.TestLoader()
runner = unittest.TextTestRunner()

# Discover other suites
application_suite = loader.discover("test_application")
cpakfile_suite    = loader.discover("test_cpakfile")

# Run all suites
runner.run(application_suite)
runner.run(cpakfile_suite)
