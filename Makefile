########################## useful Variables
space:= $(empty) $(empty)
OUTC = -o$(space)
OUTF = -o$(space)
TRUE = 1
FALSE = 0
#TODO make non changeable variables full lower case
########################## general settings

CXX = clang
# requires VOLK to be installed with VULKAN SDK
VULKAN_ENABLED = $(TRUE)
#VULKAN_ENABLED = $(FALSE)
VULKAN_PATH = C:/VulkanSDK/1.3.231.1

#make sure to add a custom buildfolder to the .gitignore
BUILDFOLDER = out/mf
#use /myScenes/main.cpp instead of /src/main.cpp and myScenes instead of scenes
MYENVIR = $(FALSE)
#TODO somehow move out of Makefile

ARGO = -c -Wall
ARGOP = -std=c++17 -c -Wall
ARGF = -std=c++17 -Wall

TARGET = einherjarEngine
TEXTENSION = .exe

########################## source
LIB = -Lfolder -lglad -lglfw3
INC = -Ilib/imgui -Ilib/imgui/backends -Ilib/glfw3 -Ilib

SRC2 = $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*/*.cpp) lib/glad/glad.c
HEAD = $(wildcard src/*.hpp) $(wildcard src/*/*.hpp) $(wildcard src/*/*/*.hpp)
HSCENE = $(wildcard scenes/*.hpp)

ifeq ($(MYENVIR),$(TRUE))
#SRC3 = $(filter src/main.cpp,$(space), $(SRC2)) $(myScenes/main.cpp)
SRC3 = $(SRC2:src/main.cpp=myScenes/main.cpp)
HSCENE = $(wildcard myScenes/*.hpp)
SRC = $(SRC3)
else
SRC = $(SRC2)
endif

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

#main compile
$(BUILDFOLDER)/myScenes/main.o: myScenes/main.cpp $(HSCENE) $(HEAD)
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGOP) $< $(OUTC)$@ $(INC) $(VULKAN_INC)
	
#main compile
$(BUILDFOLDER)/src/main.o: src/main.cpp $(HSCENE) $(HEAD)
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGOP) $< $(OUTC)$@ $(INC) $(VULKAN_INC)

$(BUILDFOLDER)/src/%.o: src/%.cpp $(HEAD)
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGOP) $< $(OUTC)$@ $(INC) $(VULKAN_INC)

#library compile
$(BUILDFOLDER)/lib/%.o: lib/%.cpp
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGOP) $< $(OUTC)$@ $(INC) $(VULKAN_INC)

$(BUILDFOLDER)/lib/%.o: lib/%.c
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGO) $< $(OUTC)$@ $(INC) $(VULKAN_INC)

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
