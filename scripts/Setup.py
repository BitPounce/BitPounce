import os
os.system("git submodule update --init --recursive")
os.system("git submodule foreach --recursive git fetch")
os.system("git submodule update --init --recursive")

os.system("py Gen-Proj.py")