import unittest

loader = unittest.TestLoader()
runner = unittest.TextTestRunner()

# Discover other suites
cpakfile_suite = loader.discover("cpakfile")

# Run all suites
runner.run(cpakfile_suite)
