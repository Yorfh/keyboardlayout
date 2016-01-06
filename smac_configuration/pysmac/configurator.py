import argparse
import os
from datetime import datetime
import random
import pysmac
import shutil
import subprocess


class Configurator:
    def __init__(self, args):
        parser = argparse.ArgumentParser("Configure an algorithm using smac for optimal running time. With unknown starting solution")
        parser.add_argument("instances", metavar="INST", type=str, nargs="+",
                            help="An instance string to be passed to the algorithm")
        parser.add_argument("--parameters", type=str, required=True, help="The parameter file for smac")
        args = parser.parse_args(args)
        self.instances = args.instances
        self.parameters = args.parameters
        now = datetime.now()
        self.seed = random.randint(0, 2147483647)
        self.outputDir = os.path.join("output", "pysmac", now.strftime("%d_%m_%Y_%H_%M_%S"))
        self.cmd = "..\\smac\\smac.bat --seed %i --output-dir %s --scenario-file %s\\scenario.txt" % \
                   (self.seed, self.outputDir, self.outputDir)

    def run(self):
        os.makedirs(self.outputDir)
        with open(os.path.join(self.outputDir, "instances.txt"), "w") as f:
            f.write(self.get_instance_file())
        with open(os.path.join(self.outputDir, "scenario.txt"), "w") as f:
            f.write(self.get_scenario_file())
        shutil.copyfile(self.parameters, os.path.join(self.outputDir, "parameters.txt"))
        p = subprocess.Popen(self.cmd, creationflags=subprocess.CREATE_NEW_CONSOLE)
        #p = subprocess.Popen(self.cmd)
        p.communicate()


    def get_instance_file(self):
        return "\n".join(('"%s"' % (s.replace("\\", "\\\\")) for s in self.instances))

    def get_scenario_file(self):
        p = os.path.join(pysmac.__path__[0], "scenario.txt")
        with open(p, "r") as f:
            c = f.read()
            return c

