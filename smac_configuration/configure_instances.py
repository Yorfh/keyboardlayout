from pysmac.configurator import Configurator
import sys


def main():
    configurator = Configurator(sys.argv[1:])
    configurator.run()

if __name__ == "__main__":
    main()