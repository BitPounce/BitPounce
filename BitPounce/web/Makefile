ifndef verbose
  SILENT = @
endif

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
IMGUI_DIR  := $(VENDOR_DIR)/imgui
MSDF_ATLAS_DIR := $(VENDOR_DIR)/msdf-atlas-gen
MSDFGEN_DIR := $(VENDOR_DIR)/msdf-atlas-gen/msdfgen
ARTERY_DIR := $(MSDF_ATLAS_DIR)/artery-font-format
FT_SRC_DIR := $(VENDOR_DIR)/msdf-atlas-gen/msdfgen/freetype


## "../BitPounce/vendor/rttr/src",
##		"../BitPounce/vendor/rttr/src/rttr",

# Source files

FT_SRC := \
$(FT_SRC_DIR)/src/autofit/autofit.c \
$(FT_SRC_DIR)/src/base/ftbase.c \
$(FT_SRC_DIR)/src/base/ftbbox.c \
$(FT_SRC_DIR)/src/base/ftbdf.c \
$(FT_SRC_DIR)/src/base/ftbitmap.c \
$(FT_SRC_DIR)/src/base/ftcid.c \
$(FT_SRC_DIR)/src/base/ftdebug.c \
$(FT_SRC_DIR)/src/base/ftfstype.c \
$(FT_SRC_DIR)/src/base/ftgasp.c \
$(FT_SRC_DIR)/src/base/ftglyph.c \
$(FT_SRC_DIR)/src/base/ftgxval.c \
$(FT_SRC_DIR)/src/base/ftinit.c \
$(FT_SRC_DIR)/src/base/ftmm.c \
$(FT_SRC_DIR)/src/base/ftotval.c \
$(FT_SRC_DIR)/src/base/ftpatent.c \
$(FT_SRC_DIR)/src/base/ftpfr.c \
$(FT_SRC_DIR)/src/base/ftstroke.c \
$(FT_SRC_DIR)/src/base/ftsynth.c \
$(FT_SRC_DIR)/src/base/ftsystem.c \
$(FT_SRC_DIR)/src/base/fttype1.c \
$(FT_SRC_DIR)/src/base/ftwinfnt.c \
$(FT_SRC_DIR)/src/bdf/bdf.c \
$(FT_SRC_DIR)/src/bzip2/ftbzip2.c \
$(FT_SRC_DIR)/src/cache/ftcache.c \
$(FT_SRC_DIR)/src/cff/cff.c \
$(FT_SRC_DIR)/src/cid/type1cid.c \
$(FT_SRC_DIR)/src/gzip/ftgzip.c \
$(FT_SRC_DIR)/src/lzw/ftlzw.c \
$(FT_SRC_DIR)/src/pcf/pcf.c \
$(FT_SRC_DIR)/src/pfr/pfr.c \
$(FT_SRC_DIR)/src/psaux/psaux.c \
$(FT_SRC_DIR)/src/pshinter/pshinter.c \
$(FT_SRC_DIR)/src/psnames/psnames.c \
$(FT_SRC_DIR)/src/raster/raster.c \
$(FT_SRC_DIR)/src/sdf/sdf.c \
$(FT_SRC_DIR)/src/sfnt/sfnt.c \
$(FT_SRC_DIR)/src/smooth/smooth.c \
$(FT_SRC_DIR)/src/truetype/truetype.c \
$(FT_SRC_DIR)/src/type1/type1.c \
$(FT_SRC_DIR)/src/type42/type42.c \
$(FT_SRC_DIR)/src/winfonts/winfnt.c
MSDFGEN_SRC := \
$(shell find $(MSDFGEN_DIR)/core -name "*.cpp") \
$(shell find $(MSDFGEN_DIR)/ext -name "*.cpp") \
$(shell find $(MSDFGEN_DIR)/lib -name "*.cpp")

MSDF_ATLAS_SRC := \
$(shell find $(MSDF_ATLAS_DIR)/msdf-atlas-gen -name "*.cpp")
IMGUI_SRC := $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp ${VENDOR_DIR}/ImGuizmo/ImGuizmo.cpp $(IMGUI_DIR)/misc/cpp/imgui_stdlib.cpp
SRC_CPP	  := $(shell find $(ENGINE_DIR)/src -name "*.cpp") $(shell find $(ENGINE_DIR)/Platform/Web -name "*.cpp") $(shell find $(ENGINE_DIR)/Platform/OpenGL -name "*.cpp") $(IMGUI_SRC) $(shell find $(ENGINE_DIR)/vendor/angelscript/sdk/add_on -name "*.cpp") $(shell find $(ENGINE_DIR)/vendor/BitPouncePack/src -name "*.cpp") $(MSDFGEN_SRC)
SRC_CPP += $(MSDF_ATLAS_SRC)
SRC_C	  := $(shell find $(ENGINE_DIR)/src -name "*.c") $(ENGINE_DIR)/vendor/glad/src/gles2.c $(shell find $(VENDOR_DIR)/box2d/src -name "*.c")  $(FT_SRC)

# Object files
OBJ_CPP := $(SRC_CPP:.cpp=.o)
OBJ_CPP := $(patsubst ../%, $(BUILD_DIR)/%, $(OBJ_CPP))

OBJ_C := $(SRC_C:.c=.o)
OBJ_C := $(patsubst ../%, $(BUILD_DIR)/%, $(OBJ_C))
OBJ     := $(OBJ_CPP) $(OBJ_C)

# Include directories
INCLUDES := -I"$(ENGINE_DIR)/src" -I"$(ENGINE_DIR)/vendor/spdlog/include" -I"$(ENGINE_DIR)" -I"$(ENGINE_DIR)/vendor/GLFW/include" -I"$(ENGINE_DIR)" -I"$(ENGINE_DIR)/vendor/glad/include"
INCLUDES += -I"$(IMGUI_DIR)" -I"${VENDOR_DIR}/glm" -I"$(ENGINE_DIR)/vendor/debugbreak" -I"${VENDOR_DIR}/stb" -I"${VENDOR_DIR}/miniaudio" -I"${VENDOR_DIR}/nlohmannjson/single_include"
INCLUDES += -I"${VENDOR_DIR}/ImGuizmo" -I"${VENDOR_DIR}/entt/single_include" -I"${VENDOR_DIR}/box2d/include" -I"$(ENGINE_DIR)/vendor/angelscript/sdk/angelscript/include" -I"$(ENGINE_DIR)/vendor/BitPouncePack/zlib"
INCLUDES += \
-I$(MSDF_ATLAS_DIR)/msdf-atlas-gen \
-I$(MSDFGEN_DIR) \
-I$(MSDFGEN_DIR)/core \
-I$(MSDFGEN_DIR)/include \
-I$(ARTERY_DIR) \
-I$(ARTERY_DIR)/artery-font \
-I$(FT_SRC_DIR)/include
INCLUDES += -I"${ENGINE_DIR}/vendor/angelscript/sdk/add_on" -I"$(ENGINE_DIR)/vendor/BitPouncePack/src"

CXXFLAGS += -DFMT_HEADER_ONLY -DGLFW_INCLUDE_NONE -msimd128 -msse2 -DFT2_BUILD_LIBRARY -sASSERTIONS
# Compiler options (empty default)
OPTIONS := $(CXXFLAGS) -gsource-map -fmacro-prefix-map=$(ENGINE_DIR)/src/=  -msimd128 --use-port=emdawnwebgpu -O3

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
	$(SILENT) $(CXX) -std=c++20 $(OPTIONS) -MMD -MP -c $< -o $@ $(INCLUDES)

$(BUILD_DIR)/%.o: $(ENGINE_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling C:   $<"
	$(SILENT) $(CXX) $(OPTIONS) -MMD -MP -c $< -o $@ $(INCLUDES)

# Archive
$(DIST_DIR)/libBitPounce.a: $(OBJ)
	@mkdir -p $(DIST_DIR)
	@echo "Archiving libBitPounce.a"
	$(SILENT) emar rcs $@ $(OBJ)

# Clean
clean:
	@echo "Cleaning build directory..."
	@rm -rf "$(BUILD_DIR)" "$(DIST_DIR)/libBitPounce.a" || true

# Auto-deps
-include $(OBJ:.o=.d)
