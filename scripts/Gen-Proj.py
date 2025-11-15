import platform
import sys
import os

build_os = platform.system()

print(sys.argv)
if len(sys.argv) > 1 and sys.argv[1] == "web" :
    build_os = "web"

print("Gening for: " + build_os)

if build_os.lower() == "windows":
    os.system("Gen-Proj.bat")
    exit(0)

print(build_os + " build not supported yet")