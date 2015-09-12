import sys
import os
import csv
import argparse
from collections import defaultdict
from itertools import chain

if not len(sys.argv) == 2:
    print "Usage: analyze_validation_results runid"

filename = "validationRunResultLineMatrix-traj-run-%s-walltime.csv" % (sys.argv[1])
filename = os.path.join("output\\QAP", filename)
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
