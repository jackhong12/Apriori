SAN := -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
CXX := g++ -std=c++14
CXXFLAG := -O3 -g3
#CXXFLAG := -g3 -lpthread -fopenmp -mavx2 -mavx
#CXXFLAG := -O3 -g3 -lpthread -fopenmp
#CXXFLAG := -g3 -lpthread -fopenmp
#CXXFLAG += $(SAN)

NVCC := nvcc
CUDA_LINK_FLAGS =  -rdc=true -gencode=arch=compute_61,code=sm_61 -Xcompiler '-fPIC'
CUDA_COMPILE_FLAGS = --device-c -gencode=arch=compute_61,code=sm_61 -Xcompiler '-fPIC' -g -O3

EXE = apriori-serial
EXE += apriori-bits
EXE += apriori-thread
EXE += apriori-simd
EXE += apriori-cuda
CLOCK_HEADER := CycleTimer.hpp

all: $(EXE)

apriori-serial: apriori-serial.cpp $(CLOCK_HEADER)
	$(CXX) $< -o $@ $(CXXFLAG)

apriori-bits: apriori-bits.cpp $(CLOCK_HEADER)
	$(CXX) $< -o $@ $(CXXFLAG)

apriori-thread: apriori-thread.cpp $(CLOCK_HEADER)
	$(CXX) $< -o $@ $(CXXFLAG) -lpthread

apriori-simd: apriori-simd.cpp $(CLOCK_HEADER)
	$(CXX) $< -o $@ $(CXXFLAG) -mavx2 -mavx

#apriori-cuda: apriori-cuda.cpp $(CLOCK_HEADER)
#	$(NVCC) $(CUDA_COMPILE_FLAGS) $< -o $@

apriori-cuda: cuda-kernel.o apriori-cuda.o
	$(NVCC) $^ -o $@ $(CUDA_LINK_FLAGS)

apriori-cuda.o: apriori-cuda.cpp $(CLOCK_HEADER) cuda-kernel.hpp
	$(CXX) -c $< -o $@ $(CXXFLAG) 

cuda-kernel.o: cuda-kernel.cu cuda-kernel.hpp
	$(NVCC) -c $< -o $@ $(CUDA_COMPILE_FLAGS) 

clean:
	rm -f $(EXE)
	rm -f *.o
	rm -f callgrind.out.*
