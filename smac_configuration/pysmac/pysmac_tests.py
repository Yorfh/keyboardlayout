import unittest
from runner import Runner


class CommandLineTests(unittest.TestCase):
    def test_one_instance(self):
        runner = Runner(["instance1"])
        self.assertEqual(runner.instances, ["instance1"])

    def test_two_instances(self):
        runner = Runner(["instance1", "instance2"])
        self.assertEqual(runner.instances, ["instance1", "instance2"])

    def test_no_instances(self):
        with self.assertRaises(SystemExit):
            Runner([])


if __name__ == '__main__':
    unittest.main()
