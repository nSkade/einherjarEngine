########################## useful Variables
space:= $(empty) $(empty)
OUTC = -o$(space)
OUTF = -o$(space)
TRUE = t
FALSE = f

########################## general settings

CXX = clang
# requires VOLK to be installed with VULKAN SDK
VULKAN_ENABLED = $(TRUE)
#VULKAN_ENABLED = $(FALSE)
VULKAN_PATH = C:/VulkanSDK/1.3.231.1

#make sure to add a custom buildfolder to the .gitignore
BUILDFOLDER = out/mf

ARGO = -c -Wall
ARGOP = -std=c++17 -c -Wall
ARGF = -std=c++17 -Wall

TARGET = einherjarEngine
TEXTENSION = .exe

########################## source
LIB = -Lfolder -lglad -lglfw3
INC = -Ilib/imgui -Ilib/imgui/backends -Ilib/glfw3 -Ilib

SRC = $(wildcard src/*.cpp) $(wildcard src/scenes/*.cpp) lib/glad/glad.c
HEAD = $(wildcard src/*.hpp)
HSCENE = $(wildcard src/scenes/*.hpp)

########################## vulkan
ifeq ($(VULKAN_ENABLED),$(TRUE))
VULKAN_INC = -I$(VULKAN_PATH)/Include
VULKAN_LIB = -l$(VULKAN_PATH)/Lib/vulkan-1

IMGUI_VULK = lib/imgui/backends/imgui_impl_vulkan.cpp
else
VULKAN_INC = $(space)
VULKAN_LIB = $(space)

IMGUI_VULK = $(space)
endif

########################## imgui
IMGUI = $(wildcard lib/imgui/*.cpp) lib/imgui/backends/imgui_impl_opengl3.cpp lib/imgui/backends/imgui_impl_glfw.cpp $(IMGUI_VULK)
IMGUIO2 = $(patsubst %, $(BUILDFOLDER)/%, $(IMGUI))
IMGUIO = $(IMGUIO2:.cpp=.o)

########################## generate .o names
OBJ3 = $(patsubst %, $(BUILDFOLDER)/%, $(SRC))
OBJ2 = $(OBJ3:.cpp=.o)
OBJ = $(OBJ2:.c=.o)
NODEPS = clean cleanDBG

##########################
all: $(TARGET)

$(BUILDFOLDER)/%.o: %.c
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGO) $< $(OUTC)$@ $(INC) $(VULKAN_INC)

$(BUILDFOLDER)/src/%.o: src/%.cpp $(HEAD)
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGOP) $< $(OUTC)$@ $(INC) $(VULKAN_INC)

#main compile
$(BUILDFOLDER)/src/main.o: src/main.cpp $(HSCENE)
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGOP) $< $(OUTC)$@ $(INC) $(VULKAN_INC)

#library compile
$(BUILDFOLDER)/lib/%.o: lib/%.cpp
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGOP) $< $(OUTC)$@ $(INC) $(VULKAN_INC)

#link executable
$(TARGET): $(OBJ) $(IMGUIO)
	$(CXX) $(ARGF) $(OBJ) $(IMGUIO) $(OUTF)$(TARGET)$(TEXTENSION) $(LIB) $(INC) $(VULKAN_LIB) $(VULKAN_INC)

##########################
.PHONY: clean
clean:
	rmdir /s /q "$(BUILDFOLDER)"

.PHONY: cleanDBG
cleanDBG:
	rmdir /s /q "$(BUILDFOLDER)dbg"

.PHONY: cleanEXEC
cleanEXEC:
	del einherjarEngine.*
	del einherjarEngineDbg.*
