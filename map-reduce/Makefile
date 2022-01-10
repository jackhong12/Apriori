CXX := g++
#CXX_FLAG := -g
CXX_FLAG := -g -O3

SRC := $(wildcard *.cpp)
EXE := $(patsubst %.cpp,%,$(SRC))

all: $(EXE)

%: %.cpp
	$(CXX) $^ -o $@ $(CXX_FLAG)

clean:
	rm -rf $(EXE) *.txt
