import unittest
from configurator import Configurator


class CommandLineTests(unittest.TestCase):
    def test_one_instance(self):
        runner = Configurator(["instance1"])
        self.assertEqual(runner.instances, ["instance1"])

    def test_two_instances(self):
        runner = Configurator(["instance1", "instance2"])
        self.assertEqual(runner.instances, ["instance1", "instance2"])

    def test_no_instances(self):
        with self.assertRaises(SystemExit):
            Configurator([])


class InstanceFileGenerationTests(unittest.TestCase):
    def test_two_instances(self):
        runner = Configurator(["instance1", "instance2"])
        expected_result = (
            '"instance1"\n'
            '"instance2"'
        )

        self.assertEqual(runner.get_instance_file(), expected_result)

if __name__ == '__main__':
    unittest.main()
