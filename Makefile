space:= $(empty) $(empty)
OUTC = -o$(space)
OUTF = -o$(space)
##########################

TARGET = einherjarEngine

CXX = clang
#make sure to add a custom buildfolder to the .gitignore
BUILDFOLDER = out/mf
TEXTENSION = .exe

ARGO = -c -Wall
ARGOP = -std=c++17 -c -Wall
ARGF = -std=c++17 -Wall
NODEPS = clean cleanDBG

LIB = -Lfolder -lglad -lglfw3 -lC:/VulkanSDK/1.3.231.1/Lib/vulkan-1
INC = -Ilib/imgui -Ilib/imgui/backends -Ilib/glfw3 -Ilib

SRC = $(wildcard src/*.cpp) $(wildcard src/scenes/*.cpp) lib/glad/glad.c
HEAD = $(wildcard src/*.hpp) $(wildcard src/scenes/*.hpp)

##########################

IMGUI = $(wildcard lib/imgui/*.cpp) lib/imgui/backends/imgui_impl_opengl3.cpp lib/imgui/backends/imgui_impl_glfw.cpp
IMGUIO2 = $(patsubst %, $(BUILDFOLDER)/%, $(IMGUI))
IMGUIO = $(IMGUIO2:.cpp=.o)

##########################

OBJ3 = $(patsubst %, $(BUILDFOLDER)/%, $(SRC))
OBJ2 = $(OBJ3:.cpp=.o)
OBJ = $(OBJ2:.c=.o)

##########################

VULKAN_INC = -IC:/VulkanSDK/1.3.231.1/Include

#TARGET_VULK = einherjarEngineVulk
#TODO vulkan main and include files?
#VULKAN

#TODO include vulkan headers
#$(TARGET_VULK): $(OBJ) $(IMGUIO)
#	$(CXX) $(ARGF) $(OBJ) $(IMGUIO) $(OUTF)$(TARGET)$(TEXTENSION) $(LIB) $(INC) $(VULKAN_INC)
##########################

all: $(TARGET)

$(BUILDFOLDER)/%.o: %.c
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGO) $< $(OUTC)$@ $(INC) $(VULKAN_INC)

$(BUILDFOLDER)/src/%.o: src/%.cpp $(HEAD)
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGOP) $< $(OUTC)$@ $(INC) $(VULKAN_INC)

$(TARGET): $(OBJ) $(IMGUIO)
	$(CXX) $(ARGF) $(OBJ) $(IMGUIO) $(OUTF)$(TARGET)$(TEXTENSION) $(LIB) $(INC) $(VULKAN_INC)

.PHONY: clean
clean:
	rmdir /s /q "$(BUILDFOLDER)"

.PHONY: cleanDBG
cleanDBG:
	rmdir /s /q "$(BUILDFOLDER)dbg"

$(BUILDFOLDER)/lib/%.o: lib/%.cpp
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGOP) $< $(OUTC)$@ $(INC)

##########################

.PHONY: vulkan
vulkan: $(TARGET_VULK)
