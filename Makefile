CXX = clang
#make sure to add a custom buildfolder to the .gitignore
BUILDFOLDER = out/mf
TEXTENSION = .exe

ARG = -c -Wall
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
	$(CXX) $(ARG) $< -o $@ $(LIB) $(INC)

$(BUILDFOLDER)/%.o: %.cpp
	if not exist $(subst /,\\,$(dir $@)) mkdir $(subst /,\\,$(dir $@))
	$(CXX) $(ARG) $< -o $@ $(LIB) $(INC)
	
$(TARGET): $(OBJ)
	$(CXX) -Wall $(OBJ) -o $(TARGET)$(TEXTENSION) $(LIB) $(INC)

.PHONY: clean
clean:
	rmdir /s /q $(BUILDFOLDER)
