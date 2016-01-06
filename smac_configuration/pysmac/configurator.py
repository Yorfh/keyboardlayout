import argparse
import os
from datetime import datetime
import random


class Configurator:
    def __init__(self, args):
        parser = argparse.ArgumentParser("Configure an algorithm using smac for optimal running time. With unknown starting solution")
        parser.add_argument("instances", metavar="INST", type=str, nargs="+",
                            help="An instance string to be passed to the algorithm")
        args = parser.parse_args(args)
        self.instances = args.instances
        now = datetime.now()
        self.seed = random.randint(0, 4294967295)
        self.outputDir = os.path.join("output", "pysmac", now.strftime("%d.%m.%Y %H:%M:%S"))
        self.cmd = "call ../smac/smac.bat --seed %i --output-dir %s --scenario-file %s\\scenario.txt" % \
                   (self.seed, self.outputDir, self.outputDir)

    def run(self):
        pass

    def get_instance_file(self):
        return "\n".join(('"%s"' % (s) for s in self.instances))
