SAN := -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
SRC := $(wildcard *.cpp)
TARGET := $(patsubst %.cpp,%,$(SRC))

NVCC := nvcc
NVCC_SRC := $(wildcard *.cu)
NVCC_OBJ := $(patsubst %.cu,%.o,$(NVCC_SRC))
NVCC_CCOMPILE_FLAGS := --device-c -gencode=arch=compute_61,code=sm_61 -Xcompiler '-fPIC' -g -O3 -I../lib
NVCC_LINK_FLAGS := -rdc=true -gencode=arch=compute_61,code=sm_61 -Xcompiler '-fPIC' -I../lib

all: $(TARGET)

$(TARGET): $(NVCC_OBJ) $(SRC)
	$(NVCC) $^ -o $(TARGET) $(NVCC_LINK_FLAGS)

%.o: %.cu
	$(NVCC) $^ -c -o $@ $(NVCC_CCOMPILE_FLAGS)

clean:
	rm -rf $(TARGET) $(NVCC_OBJ) *.d
