CXX := g++ -std=c++14
CXXFLAG := -O3 -g3 -MMD -I../lib
SAN := -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
SRC := $(wildcard *.cpp)
OBJ := $(patsubst %.cpp,%.o,$(SRC))
TARGET := $(patsubst %.cpp,%,$(SRC))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $^ -o $(TARGET) -lpthread

%.o: %.cpp
	$(CXX) $< -c -o $@ $(CXXFLAG)

clean:
	rm -rf $(TARGET) $(OBJ) *.d
