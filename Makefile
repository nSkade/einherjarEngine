CXX = clang
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

LIB = -Lfolder -lglad -lglfw3
INC = -Ilib/imgui -Ilib/imgui/backends -Ilib/glfw3

IMGUI = $(wildcard lib/imgui/*.cpp) lib/imgui/backends/imgui_impl_opengl3.cpp lib/imgui/backends/imgui_impl_glfw.cpp
SRC = $(wildcard src/*.cpp) lib/glad/glad.c $(IMGUI)

OBJ3 = $(patsubst %, $(BUILDFOLDER)/%, $(SRC))
OBJ2 = $(OBJ3:.cpp=.o)
OBJ = $(OBJ2:.c=.o)

all: $(TARGET)

$(BUILDFOLDER)/%.o: %.c
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGO) $< $(OUTC)$@ $(LIB) $(INC)

$(BUILDFOLDER)/%.o: %.cpp
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARGOP) $< $(OUTC)$@ $(LIB) $(INC)
	
$(TARGET): $(OBJ)
	$(CXX) $(ARGF) $(OBJ) $(OUTF)$(TARGET)$(TEXTENSION) $(LIB) $(INC)

.PHONY: clean
clean:
	rmdir /s /q "$(BUILDFOLDER)"
