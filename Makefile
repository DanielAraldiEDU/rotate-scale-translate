CXX = g++
CXXFLAGS = -Wall -g
LDFLAGS = -framework OpenGL -framework GLUT

execute: main
	./main

generate: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o main $(LDFLAGS)

clean:
	rm -f main

run:
	make clean && make generate && make execute