import os

os.chdir("../BitPounce/vendor/bgfx")
os.system("make projgen -j" + os.cpu_count().__str__())
os.system("make wasm -j" + os.cpu_count().__str__())
os.system("make linux-gcc -j" + os.cpu_count().__str__())