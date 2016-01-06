import sys
import subprocess


def main():
    rest = " ".join(sys.argv[6:])
    cmd = r"..\x64\release\keyboardlayout.exe --smac --test %s --instance_info %s --cutoff_time %s --cutoff_length %s --seed %s %s" % \
          (sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], rest)
    p = subprocess.Popen(cmd)
    p.communicate()

if __name__ == "__main__":
    main()
