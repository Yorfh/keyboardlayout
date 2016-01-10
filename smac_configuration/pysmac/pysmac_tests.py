import unittest
from configurator import Configurator
from runner import Runner
import os
import pysmac


class CommandLineTests(unittest.TestCase):
    def test_one_instance(self):
        conf = Configurator(["instance1", "--parameters", "parameters.txt"])
        self.assertEqual(conf.instances, ["instance1"])

    def test_two_instances(self):
        conf = Configurator(["instance1", "instance2", "--parameters", "parameters.txt"])
        self.assertEqual(conf.instances, ["instance1", "instance2"])
        self.assertRegexpMatches(conf.outputDir, r"output\\pysmac\\\d\d_\d\d_\d\d\d\d_\d\d_\d\d_\d\d")
        outdir_regexp = conf.outputDir.replace("\\", "\\\\")
        self.assertRegexpMatches(conf.cmd,
                         r"..\\smac\\smac.bat --seed \d+ --output-dir %s --scenario-file %s\\scenario.txt" %
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
        conf = Configurator(["instance1", "instance2", "--parameters", "inasubdir\\hello.txt"])
        p = os.path.join(pysmac.__path__[0], "scenario.txt")
        with open(p, "r") as f:
            c = f.read()
            c = c % {"parameters": "parameters.txt"}
            self.assertEqual(conf.get_scenario_file(), c)


class RunnerTests(unittest.TestCase):
    def test_commandline_generation(self):
        runner = Runner("keyboardlayout.exe", ["run_instance.py", "test_instance --param1 4", "instance_info", "cutoff_time", "cutoff_length", "seed", "--first_extra_arg", "--second_extra_arg"])
        self.assertEqual(runner.cmd, "keyboardlayout.exe --smac --test test_instance --param1 4 --instance_info instance_info --cutoff_time cutoff_time --cutoff_length cutoff_length --seed seed --first_extra_arg --second_extra_arg")

    def test_a_timeout_is_transformed_to_sat(self):
        result = "Result of this algorithm run: TIMEOUT, 1, 0.359375, 1818146, 9310539, 0"
        self.assertEqual(Runner.transform_timeout_to_sat(result), "Result of this algorithm run: SAT, 0.359375, 0.359375, 1818146, 9310539, 0")

    def test_a_sat_is_not_transformed(self):
        result = "Result of this algorithm run: SAT, 0.359375, 0.359375, 1818146, 9310539, 0"
        self.assertEqual(Runner.transform_timeout_to_sat(result), "Result of this algorithm run: SAT, 0.359375, 0.359375, 1818146, 9310539, 0")


if __name__ == '__main__':
    unittest.main()
