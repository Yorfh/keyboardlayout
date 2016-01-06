import unittest
from configurator import Configurator
import os
import pysmac


class CommandLineTests(unittest.TestCase):
    def test_one_instance(self):
        conf = Configurator(["instance1", "--parameters", "parameters.txt"])
        self.assertEqual(conf.instances, ["instance1"])

    def test_two_instances(self):
        conf = Configurator(["instance1", "instance2", "--parameters", "parameters.txt"])
        self.assertEqual(conf.instances, ["instance1", "instance2"])
        self.assertRegexpMatches(conf.outputDir, r"output\\pysmac\\\d\d.\d\d.\d\d\d\d \d\d:\d\d:\d\d")
        outdir_regexp = conf.outputDir.replace("\\", "\\\\")
        self.assertRegexpMatches(conf.cmd,
                         r"call ../smac/smac.bat --seed \d+ --output-dir %s --scenario-file %s\\scenario.txt" %
                         (outdir_regexp, outdir_regexp))

    def test_empty_arguments(self):
        with self.assertRaises(SystemExit):
            Configurator([])

    def test_no_instances(self):
        with self.assertRaises(SystemExit):
            Configurator(["--parameters", "parameters.txt"])

    def test_no_parameters(self):
        with self.assertRaises(SystemExit):
            Configurator(["instance"])


class InstanceFileGenerationTests(unittest.TestCase):
    def test_two_instances(self):
        conf = Configurator(["instance1", "instance2", "--parameters", "parameters.txt"])
        expected_result = (
            '"instance1"\n'
            '"instance2"'
        )

        self.assertEqual(conf.get_instance_file(), expected_result)


class ScenarioFileGenerationTests(unittest.TestCase):
    def test_two_instances(self):
        conf = Configurator(["instance1", "instance2", "--parameters", "hello.txt"])
        p = os.path.join(pysmac.__path__[0], "scenario.txt")
        with open(p, "r") as f:
            c = f.read()
            c = c % {"parameters": "hello.txt"}
            self.assertEqual(conf.get_scenario_file(), c)


if __name__ == '__main__':
    unittest.main()
