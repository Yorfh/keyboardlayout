import subprocess
import re

class Runner:
    def __init__(self, exe, argv):
        rest = " ".join(argv[6:])
        self.cmd = r"%s --smac --test %s --instance_info %s --cutoff_time %s --cutoff_length %s --seed %s %s" % \
            (exe, argv[1], argv[2], argv[3], argv[4], argv[5], rest)

    def run(self):
        p = subprocess.Popen(self.cmd, stdout=subprocess.PIPE)
        stdoutdata, _ = p.communicate()
        # TODO(optional), if the result was better than target, then abort everything, remove all results for that instance
        # and restart
        return Runner.transform_timeout_to_sat(stdoutdata)

    @classmethod
    def transform_timeout_to_sat(cls, res):
        if res.find("TIMEOUT"):
            res = res.replace("Result of this algorithm run: ", "")
            output = res.split(", ")
            output[0] = "SAT"
            output[1] = output[2]
            res = "Result of this algorithm run: " + ", ".join(output)

        return res

