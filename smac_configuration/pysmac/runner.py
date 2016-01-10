import subprocess

class Runner:
    def __init__(self, exe, argv):
        rest = " ".join(argv[6:])
        self.cmd = r"%s --smac --test %s --instance_info %s --cutoff_time %s --cutoff_length %s --seed %s %s" % \
            (exe, argv[1], argv[2], argv[3], argv[4], argv[5], rest)

    def run(self):
        p = subprocess.Popen(self.cmd, stdout=subprocess.PIPE)
        stdoutdata, _ = p.communicate()
        # TODO, if we didn't specify a target, then assume success, and change the result
        # TODO Make keyboardlayout.exe output the time when the best solution was found so it can be used
        # TODO(optional), if the result was better than target, then abort everything, remove all results for that instance
        # and restart
        return stdoutdata

