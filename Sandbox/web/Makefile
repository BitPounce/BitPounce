# Automatically enable parallel builds using all CPU cores
MAKEFLAGS += -j$(shell nproc)

# Compiler
CXX := emcc

# Directories
BASE_DIR   := ../..
ENGINE_DIR := $(BASE_DIR)/BitPounce
APP_DIR    := ../
DIST_DIR   := $(BASE_DIR)/bin/Debug-WebGL/Sandbox
BUILD_DIR  := $(BASE_DIR)/bin-int/Debug-WebGL/Sandbox
ENGINE_LIB := $(BASE_DIR)/bin/Debug-WebGL/BitPounce/libengine.a

# Source files
SRC_CPP := $(shell find $(APP_DIR)/src -name "*.cpp")
SRC_C   := $(shell find $(APP_DIR)/src -name "*.c")

# Object files (preserve directory structure)
OBJ_CPP := $(patsubst $(APP_DIR)/src/%.cpp, $(BUILD_DIR)/%.o, $(SRC_CPP))
OBJ_C   := $(patsubst $(APP_DIR)/src/%.c,   $(BUILD_DIR)/%.o, $(SRC_C))
OBJ     := $(OBJ_CPP) $(OBJ_C)

# Include directories
INCLUDES := -I"$(APP_DIR)/src" -I"$(ENGINE_DIR)/src" -I"$(ENGINE_DIR)/vendor/spdlog/include"

# Compiler options
OPTIONS := -std=c++20 -sUSE_GLFW=3 -sWASM=1 -sALLOW_MEMORY_GROWTH=1 \
           -sNO_DISABLE_EXCEPTION_CATCHING -sFULL_ES3=1 -O2 \
           -msimd128 -msse -gsource-map \
           --source-map-base http://localhost:8000/$(DIST_DIR)/ \
           -fmacro-prefix-map=$(ENGINE_DIR)/src/= \
           $(INCLUDES)

# Default target
all: buildEngine $(DIST_DIR)/index.html

# Compile rules
$(BUILD_DIR)/%.o: $(APP_DIR)/src/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling C++: $<"
	$(CXX) $(OPTIONS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: $(APP_DIR)/src/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling C:   $<"
	$(CXX) $(OPTIONS) -MMD -MP -c $< -o $@

# Build engine recursively
buildEngine:
	@echo "Building Engine..."
	$(MAKE) -C $(ENGINE_DIR)/web $(MAKEFLAGS)

# Ensure engine library exists before linking
$(ENGINE_LIB): buildEngine

# Link final WebAssembly app
$(DIST_DIR)/index.html: $(OBJ) $(ENGINE_LIB)
	@mkdir -p $(DIST_DIR)
	@echo "Linking index.html with WebAssembly..."
	$(CXX) $(OBJ) -o $@ \
	    -L"$(BASE_DIR)/bin/Debug-WebGL/BitPounce" \
	    -Wl,--whole-archive -lBitPounce -Wl,--no-whole-archive \
	    -sEXPORTED_FUNCTIONS='["_main", "_malloc", "_free"]' \
	    -sEXPORTED_RUNTIME_METHODS='["ccall","cwrap","requestFullscreen"]' \
	    $(OPTIONS) \
	    --preload-file "assets"

# Clean build directories
clean:
	@echo "Cleaning build directories..."
	@rm -rf "$(BUILD_DIR)" "$(DIST_DIR)"
	$(MAKE) -C $(ENGINE_DIR)/WebGL clean || true

# Auto-dependency includes
-include $(OBJ:.o=.d)
