
#TODO real makefile
CXX = clang
ARG = -c -Wall
TEXTENSION = .exe
TARGET = einherjarEngine

LIB = -Lfolder -lglad -lglfw3
INC = -Ilib/imgui -Ilib/imgui/backends -Ilib/glfw3

IMGUI = $(wildcard lib/imgui/*.cpp) lib/imgui/backends/imgui_impl_opengl3.cpp lib/imgui/backends/imgui_impl_glfw.cpp
SRC = $(wildcard src/*.cpp) lib/glad/glad.c $(IMGUI)
OBJ2 = $(SRC:.cpp=.o)
OBJ = $(OBJ2:.c=.o)

all: $(TARGET)

%.o: %.c
	$(CXX) $(ARG) $< -o $@ $(LIB) $(INC)

%.o: %.cpp
	$(CXX) $(ARG) $< -o $@ $(LIB) $(INC)
	
$(TARGET): $(OBJ)
	$(CXX) -Wall $(OBJ) -o $(TARGET)$(TEXTENSION) $(LIB) $(INC)

.PHONY: clean
clean:
	erase /s *.o
	del imgui.ini
