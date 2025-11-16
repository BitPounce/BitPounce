# Compiler
CXX := emcc

# Directories
BASE_DIR := ../..
ENGINE_DIR := $(BASE_DIR)/BitPounce
APP_DIR := ../
DIST_DIR := $(BASE_DIR)/bin/Debug-WebGL/Sandbox
BUILD_DIR := $(BASE_DIR)/bin-int/Debug-WebGL/Sandbox
ENGINE_LIB := $(BASE_DIR)/bin/Debug-WebGL/BitPounce/libengine.a

# Source files
SRC_CPP := $(shell find $(APP_DIR)/src -name "*.cpp")
SRC_C   := $(shell find $(APP_DIR)/src -name "*.c")

# Object files
OBJ_CPP := $(patsubst $(APP_DIR)/src/%, $(BUILD_DIR)/%, $(SRC_CPP:.cpp=.o))
OBJ_C   := $(patsubst $(APP_DIR)/src/%, $(BUILD_DIR)/%, $(SRC_C:.c=.o))
OBJ     := $(OBJ_CPP) $(OBJ_C)

# Includes and options
INCLUDES := -I"$(APP_DIR)/src" -I"$(ENGINE_DIR)/src" -I"$(ENGINE_DIR)/vendor/spdlog/include"
OPTIONS  := -sUSE_GLFW=3 -sWASM=1 -sALLOW_MEMORY_GROWTH=1 -sNO_DISABLE_EXCEPTION_CATCHING $(INCLUDES) -s FULL_ES3=1 -O2 -msimd128 -msse

# Default target
all: buildEngine $(DIST_DIR)/index.html

# Compile rules with Auto-deps
$(BUILD_DIR)/%.o: $(APP_DIR)/src/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling C++: $<"
	$(CXX) -std=c++20 $(OPTIONS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: $(APP_DIR)/src/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling C: $<"
	$(CXX) $(OPTIONS) -MMD -MP -c $< -o $@

# Build engine
buildEngine:
	@echo "Building Engine..."
	$(MAKE) -C $(ENGINE_DIR)/web $(MAKEFLAGS)

# Ensure the engine library exists before linking
$(ENGINE_LIB): buildEngine

# Link final app
$(DIST_DIR)/index.html: $(OBJ) $(ENGINE_LIB)
	@mkdir -p $(DIST_DIR)
	@echo "Linking index.html with WebAssembly..."
	$(CXX) $(OBJ) -std=c++20 -o $@ \
	    -L"$(BASE_DIR)/bin/Debug-WebGL/BitPounce" \
	    -Wl,--whole-archive -lBitPounce -Wl,--no-whole-archive \
	    -sEXPORTED_FUNCTIONS='["_main", "_malloc", "_free"]' \
	    -sEXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "requestFullscreen"]' \
	    $(OPTIONS)	\
		--preload-file "assets" 

# Clean
clean:
	@if [ -d "$(BUILD_DIR)" ]; then rm -rf "$(BUILD_DIR)"; fi
	@if [ -d "$(DIST_DIR)" ]; then rm -rf "$(DIST_DIR)"; fi
	$(MAKE) -C $(ENGINE_DIR)/WebGL clean

# Auto-deps
-include $(OBJ:.o=.d)
