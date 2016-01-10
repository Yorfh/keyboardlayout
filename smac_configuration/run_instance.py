import sys
import subprocess
from pysmac.runner import Runner


def main():
    runner = Runner(r"..\x64\release\keyboardlayout.exe", sys.argv)
    print runner.run()

if __name__ == "__main__":
    main()
