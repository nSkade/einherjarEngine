all:
	build.bat

#TODO real makefile
#CXX = clang
#ARG = -c -Wall

#LIB = -Lfolder -lglad -lglfw3

#SRC = src/main.cpp src/GPUTimer.cpp glad/glad.c
#OBJ = $($(SRC:.cpp=.o):.c=.o)

#all: $(TARGET)

#$(TARGET): $(OBJ)
#	$(CXX) $(ARG) -o $(OBJ) $(LIB)

#main.o: src/main.cpp
#	$(CMPLR) $(ARG) out/main.cpp

#clean:
#	del out/*.o
