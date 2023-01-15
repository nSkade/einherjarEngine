CXX = clang
CXXMMD = clang
#make sure to add a custom buildfolder to the .gitignore
BUILDFOLDER = out/mf
TEXTENSION = .exe

space:= $(empty) $(empty)
ARGO = -c -Wall
ARGOP = -std=c++17 -c -Wall
ARGF = -std=c++17 -Wall
OUTC = -o$(space)
OUTF = -o$(space)
TARGET = einherjarEngine
NODEPS = clean cleanDBG

LIB = -Lfolder -lglad -lglfw3
INC = -Ilib/imgui -Ilib/imgui/backends -Ilib/glfw3

IMGUI = $(wildcard lib/imgui/*.cpp) lib/imgui/backends/imgui_impl_opengl3.cpp lib/imgui/backends/imgui_impl_glfw.cpp
IMGUIO2 = $(patsubst %, $(BUILDFOLDER)/%, $(IMGUI))
IMGUIO = $(IMGUIO2:.cpp=.o)

SRC = $(wildcard src/*.cpp) $(wildcard src/scenes/*.cpp) lib/glad/glad.c
HEAD = $(wildcard src/*.hpp) $(wildcard src/scenes/*.hpp)

OBJ3 = $(patsubst %, $(BUILDFOLDER)/%, $(SRC))
OBJ2 = $(OBJ3:.cpp=.o)
OBJ = $(OBJ2:.c=.o)

all: $(TARGET)

$(BUILDFOLDER)/%.o: %.c Makefile
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGO) $< $(OUTC)$@ $(INC)

$(BUILDFOLDER)/src/%.o: src/%.cpp $(HEAD) Makefile
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGOP) $< $(OUTC)$@ $(INC)

$(TARGET): $(OBJ) $(IMGUIO) Makefile
	$(CXX) $(ARGF) $(OBJ) $(IMGUIO) $(OUTF)$(TARGET)$(TEXTENSION) $(LIB) $(INC)

.PHONY: clean
clean:
	rmdir /s /q "$(BUILDFOLDER)"

.PHONY: cleanDBG
cleanDBG:
	rmdir /s /q "$(BUILDFOLDER)dbg"

$(BUILDFOLDER)/lib/%.o: lib/%.cpp Makefile
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGOP) $< $(OUTC)$@ $(INC)

imgui: $(IMGUIO)
