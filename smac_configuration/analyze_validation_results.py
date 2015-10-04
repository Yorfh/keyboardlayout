import sys
import os
import csv
import argparse
from collections import defaultdict
from itertools import chain

parser = argparse.ArgumentParser()
parser.add_argument("--seed", type=int)
parser.add_argument("--scenario-file", type=str)
parser.add_argument("--output-dir", type=str)
args = parser.parse_known_args(sys.argv)[0]

scenario = args.scenario_file
scenario = os.path.split(scenario)[1]
scenario = os.path.splitext(scenario)[0]
outputdir = args.output_dir

filename = "validationRunResultLineMatrix-%s-walltime.csv"
if outputdir.find("validate") == -1:
    filename = filename % ("traj-run-%s" % args.seed)
    filename = os.path.join(outputdir, scenario, filename)
else:
    with open(args.scenario_file) as f:
        for line in f.readlines():
            if line.find("trajectoryFile") != -1:
                trajectoryfile = line.split("=")[1]
                trajectoryfile = os.path.split(trajectoryfile)[1]
                trajectoryfile = os.path.splitext(trajectoryfile)[0]
                filename = filename % (trajectoryfile)
    filename = os.path.join(outputdir, filename)

filename = os.path.abspath(filename)
filename = os.path.normpath(filename)
with open(filename) as f:
    instances = defaultdict(lambda: defaultdict(list))
    reader = csv.DictReader(f)
    for row in reader:
        result = row["Run result line of validation config #1"]
        instance = row["Problem Instance"]
        parser = argparse.ArgumentParser()
        parser.add_argument("instance")
        parser.add_argument("--evaluations", type=int)
        args = parser.parse_args(instance.split(" "))
        status, runtime, runlength, quality, seed = result.split(", ")
        instances[args.instance][args.evaluations].append(float(quality))

    for instance in sorted(instances.iteritems()):
        best = min(chain(*(e for e in instance[1].itervalues())))
        print "Instance %s - best %f" % (instance[0], best)
        for e in sorted(instance[1].iteritems()):
            print "Evaluations %i - %i/%i" % (e[0], e[1].count(best), len(e[1]))
