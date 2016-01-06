import argparse


class Configurator:
    def __init__(self, args):
        parser = argparse.ArgumentParser("Configure an algorithm using smac for optimal running time. With unknown starting solution")
        parser.add_argument("instances", metavar="INST", type=str, nargs="+",
                            help="An instance string to be passed to the algorithm")
        args = parser.parse_args(args)
        self.instances = args.instances

    def run(self):
        pass

    def get_instance_file(self):
        return "\n".join(('"%s"' % (s) for s in self.instances))
