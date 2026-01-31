import os
os.system("git submodule update --init --recursive")
os.system("git submodule foreach --recursive git fetch")
os.system("git submodule update --init --recursive")

os.system("python3 Buildbgfx.py")
os.system("python3 Gen-Proj.py")