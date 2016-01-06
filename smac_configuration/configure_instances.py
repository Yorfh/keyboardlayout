from pysmac.configurator import Configurator
import sys


def main():
    runner = Configurator(sys.argv)
    runner.run()

if __name__ == "__main__":
    main()