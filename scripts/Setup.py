import os
import sys
os.system("git submodule update --init --recursive");
os.system("git submodule foreach --recursive git fetch");
os.system("git submodule foreach --recursive git submodule update --init --recursive");
os.system("git submodule foreach --recursive git submodule update --init --recursive");
os.system("git submodule foreach --recursive git submodule foreach --recursive git fetch");
os.system("git submodule foreach --recursive git submodule foreach --recursive git submodule update --init --recursive");
os.system("git submodule update --init --recursive")

os.system(sys.executable + " Buildbgfx.py");
os.system(sys.executable + " Gen-Proj.py");