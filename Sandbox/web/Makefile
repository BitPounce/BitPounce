PORT = 8000

ifndef verbose
	SILENT = @
endif

# Automatically enable parallel builds using all CPU cores
MAKEFLAGS += -j$(shell getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu)

# Compiler
CXX := emcc

# Directories
BASE_DIR   := ../..
ENGINE_DIR := $(BASE_DIR)/BitPounce
APP_DIR    := ../

DIST_DIR   := $(BASE_DIR)/bin/Debug-WebGL/Sandbox
BUILD_DIR  := $(BASE_DIR)/bin-int/Debug-WebGL/Sandbox

ENGINE_LIB := $(BASE_DIR)/bin/Debug-WebGL/BitPounce/libengine.a

VENDOR_DIR := $(ENGINE_DIR)/vendor

# Source files
SRC_CPP := $(shell find $(APP_DIR)/src -name "*.cpp")
SRC_C   := $(shell find $(APP_DIR)/src -name "*.c")

SRC_GAMELIB_CPP := $(shell find $(BASE_DIR)/GameLib/src -name "*.cpp")

# Object files
OBJ_CPP := $(patsubst $(APP_DIR)/src/%.cpp,$(BUILD_DIR)/%.o,$(SRC_CPP))
OBJ_C   := $(patsubst $(APP_DIR)/src/%.c,$(BUILD_DIR)/%.o,$(SRC_C))

OBJ_GAMELIB_CPP := $(patsubst $(BASE_DIR)/GameLib/src/%.cpp,$(BUILD_DIR)/gamelib/%.o,$(SRC_GAMELIB_CPP))

OBJ := $(OBJ_CPP) $(OBJ_C) $(OBJ_GAMELIB_CPP)

# Include directories
INCLUDES := \
	-I"$(APP_DIR)/src" \
	-I"$(ENGINE_DIR)/src" \
	-I"$(ENGINE_DIR)/vendor/spdlog/include" \
	-I"$(ENGINE_DIR)/vendor/glad/include" \
	-I"$(ENGINE_DIR)/vendor/glm" \
	-I"$(ENGINE_DIR)/vendor/imgui" \
	-I"$(ENGINE_DIR)" \
	-I"$(ENGINE_DIR)/vendor/entt/single_include" \
	-I"$(ENGINE_DIR)/vendor/nlohmannjson/single_include" \
	-I"$(ENGINE_DIR)/vendor/box2d/include" \
	-I"$(VENDOR_DIR)/ImGuizmo" \
	-I"$(ENGINE_DIR)/vendor/angelscript/sdk/angelscript/include" \
	-I"$(ENGINE_DIR)/vendor/BitPouncePack/src" \
	-I"$(BASE_DIR)/GameLib/src"

# Compiler flags
CXXFLAGS += -DFMT_HEADER_ONLY

CPPFLAGS += \
	-DIMGUI_IMPL_OPENGL_ES3 \
	-s USE_WEBGL2=1 \
	-s USE_ZLIB=1

# Link directories
LINKDIRS := \
	-L"$(ENGINE_DIR)/vendor/bgfx/.build/wasm/bin"

# Static libraries
LINKS := \
	"$(ENGINE_DIR)/vendor/bgfx/.build/wasm/bin/bimgDebug.a" \
	"$(ENGINE_DIR)/vendor/bgfx/.build/wasm/bin/bimg_decodeDebug.a" \
	"$(ENGINE_DIR)/vendor/bgfx/.build/wasm/bin/bgfxDebug.a" \
	"$(ENGINE_DIR)/vendor/angelscript/sdk/angelscript/projects/cmake/emcc/libangelscript.a"

# Common options
OPTIONS := \
	-std=c++20 \
	-sUSE_GLFW=3 \
	-sWASM=1 \
	-sALLOW_MEMORY_GROWTH=1 \
	-sNO_DISABLE_EXCEPTION_CATCHING \
	-sFULL_ES3=1 \
	-O2 \
	-msimd128 \
	-msse \
	-gsource-map \
	--source-map-base http://localhost:$(PORT)/$(DIST_DIR)/ \
	-fmacro-prefix-map=$(ENGINE_DIR)/src/= \
	$(INCLUDES) \
	$(CXXFLAGS) \
	$(CPPFLAGS) \
	$(LINKDIRS) \
	$(LINKS) \
	-s LLD_REPORT_UNDEFINED \
	-sASSERTIONS

# Default target
all: buildEngine prepare-assets $(DIST_DIR)/index.html

print-info:
	$(SILENT) $(MAKE) -C $(ENGINE_DIR)/web print-info

# ------------------------------------------------------------
# Compile rules
# ------------------------------------------------------------

$(BUILD_DIR)/%.o: $(APP_DIR)/src/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling C++: $<"
	$(SILENT) $(CXX) $(OPTIONS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/%.o: $(APP_DIR)/src/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling C: $<"
	$(SILENT) $(CXX) $(OPTIONS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/gamelib/%.o: $(BASE_DIR)/GameLib/src/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling GameLib C++: $<"
	$(SILENT) $(CXX) $(OPTIONS) -MMD -MP -c $< -o $@

# ------------------------------------------------------------
# Engine
# ------------------------------------------------------------

buildEngine:
	@echo "Building Engine..."
	$(SILENT) $(MAKE) -C $(ENGINE_DIR)/web

# ------------------------------------------------------------
# Assets
# ------------------------------------------------------------

prepare-assets:
	@echo "Preparing assets..."
	@rm -rf ./assets
	@cp -r $(APP_DIR)/assets ./assets

# Ensure engine library exists
$(ENGINE_LIB): buildEngine

# ------------------------------------------------------------
# Link final WebAssembly app
# ------------------------------------------------------------

$(DIST_DIR)/index.html: $(OBJ) $(ENGINE_LIB)
	@mkdir -p $(DIST_DIR)

	@echo "Linking WebAssembly application..."

	$(SILENT) $(CXX) \
		$(OBJ) \
		-o $@ \
		-L"$(BASE_DIR)/bin/Debug-WebGL/BitPounce" \
		-Wl,--whole-archive -lBitPounce -Wl,--no-whole-archive \
		-sEXPORTED_FUNCTIONS='["_main","_malloc","_free"]' \
		-sEXPORTED_RUNTIME_METHODS='["ccall","cwrap","requestFullscreen"]' \
		$(OPTIONS) \
		--preload-file assets

	@rm -rf ./assets

# ------------------------------------------------------------
# Run local web server
# ------------------------------------------------------------

serve:
	@echo "Serving at http://localhost:$(PORT)"
	@cd $(BASE_DIR) && python3 -m http.server $(PORT)

# ------------------------------------------------------------
# Clean
# ------------------------------------------------------------

clean:
	@echo "Cleaning build directories..."
	@rm -rf "$(BUILD_DIR)"
	@rm -rf "$(DIST_DIR)"
	@rm -rf ./assets

	$(SILENT) $(MAKE) -C $(ENGINE_DIR)/web clean

# ------------------------------------------------------------
# Auto dependency includes
# ------------------------------------------------------------

-include $(OBJ:.o=.d)