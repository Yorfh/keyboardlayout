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
parser.add_argument("--copy-last", action="store_true")
args = parser.parse_known_args(sys.argv)[0]

scenario = args.scenario_file
scenario_dir, scenario = os.path.split(scenario)
scenario = os.path.splitext(scenario)[0]
outputdir = args.output_dir

trajectoryfile = ""
filename = "validationRunResultLineMatrix-%s-walltime.csv"
if outputdir is not None and outputdir.find("validate") == -1:
    filename = filename % ("traj-run-%s" % args.seed)
    filename = os.path.join(outputdir, scenario, filename)
    trajectoryfile = os.path.join(outputdir, scenario, "traj-run-%s.txt" % args.seed)
else:
    with open(args.scenario_file) as f:
        for line in f.readlines():
            if line.find("trajectoryFile") != -1:
                trajectoryfile = line.split("=")[1].strip()
                t = os.path.split(trajectoryfile)[1]
                t = os.path.splitext(t)[0]
                filename = filename % (t)
    if outputdir:
        filename = os.path.join(outputdir, filename)

filename = os.path.abspath(filename)
filename = os.path.normpath(filename)

if args.copy_last:
    last_dir = os.path.join(scenario_dir, "optimized_parameters")
    last_file = os.path.join(last_dir, "last.txt")
    with open(trajectoryfile) as f:
        lines = f.readlines()
        with open(last_file, "w") as f2:
            f2.writelines(lines[0] + lines[-1])

with open(trajectoryfile) as f:
    reader = csv.DictReader(f, restkey="rest")
    last_row = list(reader)[-1]
    configuration = last_row["rest"]
    configuration.append(last_row["Configuration..."])
    configuration = (c.strip().replace("=", " ").replace("'", "") for c in configuration)
    configuration = sorted(configuration)
    print "Base parameters"
    for c in configuration:
        print c
    configuration="--" + " --".join(configuration)
    print "**********************************************************"
    print configuration
    print "**********************************************************"

with open(args.scenario_file) as f:
    for line in f.readlines():
        if line.find("test-instance-file") != -1:
            print "Instance parameters"
            print "**********************************************************"
            test_instance = line.split("=")[1].strip()
            with open(os.path.join(scenario_dir, test_instance)) as f2:
                for l in f2.readlines():
                    print "--test "  + l.strip().replace("\"", "")
            print "**********************************************************"

if os.path.exists(filename):
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
