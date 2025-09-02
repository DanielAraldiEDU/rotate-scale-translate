CXX = g++
CXXFLAGS = -Wall -g
LDFLAGS = -framework OpenGL -framework GLUT

all: main

main: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o main $(LDFLAGS)

clean:
	rm -f main