from pysmac.runner import Runner
import sys


def main():
    runner = Runner(sys.argv)
    runner.run()

if __name__ == "__main__":
    main()