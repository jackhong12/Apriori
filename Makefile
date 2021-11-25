SAN := -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
CXX := g++ -std=c++14
CXXFLAG := -O3 -g3 -lpthread -fopenmp
#CXXFLAG += $(SAN)

SRC := $(notdir $(wildcard *.cpp))
EXE := $(patsubst %.cpp,%.exe,$(SRC))

all: $(EXE)

%.exe: %.cpp
	$(CXX) $< -o $@ $(CXXFLAG)

vprofiling:
	valgrind --tool=callgrind ./apriori.exe 3 ./input.txt ./output.txt
	callgrind_annotate callgrind.out.* apriori.cpp

clean:
	rm -f $(EXE)
	rm -f callgrind.out.*
