import platform
import sys
import os

os.chdir("../BitPounce/vendor/bgfx")
os.system("make projgen -j" + os.cpu_count().__str__())
os.system("make wasm -j" + os.cpu_count().__str__())

build_os = platform.system()

if build_os.lower() == "windows":
	os.system("make vs2022")
	
	exit(0)
if build_os.lower() == "linux":
	os.system("make linux-gcc -j" + os.cpu_count().__str__())
	exit()