# Automatically enable parallel builds using all CPU cores
MAKEFLAGS += -j$(shell nproc)

# Compiler
CXX := emcc

# Directories
BASE_DIR   := ../..
ENGINE_DIR := ..
DIST_DIR   := $(BASE_DIR)/bin/Debug-WebGL/BitPounce
BUILD_DIR  := $(BASE_DIR)/bin-int/Debug-WebGL/BitPounce
VENDOR_DIR := $(ENGINE_DIR)/vendor

# Source files
SRC_CPP := $(shell find $(ENGINE_DIR)/src -name "*.cpp")
SRC_C   := $(shell find $(ENGINE_DIR)/src -name "*.c")

# Object files
OBJ_CPP := $(patsubst $(ENGINE_DIR)/%, $(BUILD_DIR)/%, $(SRC_CPP:.cpp=.o))
OBJ_C   := $(patsubst $(ENGINE_DIR)/%, $(BUILD_DIR)/%, $(SRC_C:.c=.o))
OBJ     := $(OBJ_CPP) $(OBJ_C)

# Include directories
INCLUDES := -I"$(ENGINE_DIR)/src" -I"$(ENGINE_DIR)/vendor/spdlog/include" -I"$(ENGINE_DIR)"
CXXFLAGS += -DFMT_HEADER_ONLY
# Compiler options (empty default)
OPTIONS := $(CXXFLAGS) -gsource-map -fmacro-prefix-map=$(ENGINE_DIR)/src/= 

all: print-info $(DIST_DIR)/libBitPounce.a

# Debug info
print-info:
	@echo "ENGINE_DIR: $(ENGINE_DIR)"
	@echo "SRC_CPP:";  printf "  %s\n" $(SRC_CPP)
	@echo "SRC_C:";    printf "  %s\n" $(SRC_C)
	@echo "OBJ_CPP:";  printf "  %s\n" $(OBJ_CPP)
	@echo "OBJ_C:";    printf "  %s\n" $(OBJ_C)
	@echo "INCLUDES: $(INCLUDES)"

# Compile rules
$(BUILD_DIR)/%.o: $(ENGINE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling C++: $<"
	$(CXX) -std=c++20 $(OPTIONS) -MMD -MP -c $< -o $@ $(INCLUDES)

$(BUILD_DIR)/%.o: $(ENGINE_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling C:   $<"
	$(CXX) $(OPTIONS) -MMD -MP -c $< -o $@ $(INCLUDES)

# Archive
$(DIST_DIR)/libBitPounce.a: $(OBJ)
	@mkdir -p $(DIST_DIR)
	@echo "Archiving libBitPounce.a"
	emar rcs $@ $(OBJ)

# Clean
clean:
	@echo "Cleaning build directory..."
	@rm -rf "$(BUILD_DIR)" "$(DIST_DIR)/libBitPounce.a" || true

# Auto-deps
-include $(OBJ:.o=.d)
