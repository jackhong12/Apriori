#include "cuda-kernel.hpp"
#include <stdio.h>

#define THREAD_WORKLOAD 100 
#define THREAD_PREBLOCK 32
__global__ void runKernel (Bit *transactions, int transaction_size, 
                           Bit *itemSets, int itemSetsSize, int unit_len,
                           int *cuda_result) {
    int size = (transaction_size + THREAD_PREBLOCK - 1) / THREAD_PREBLOCK;
    int start = threadIdx.x * size;
    if (threadIdx.x == THREAD_PREBLOCK - 1)
        size = transaction_size - (THREAD_PREBLOCK - 1) * size;
    int item_start = blockIdx.x * unit_len;

    int result = 0;
    for (int i = 0; i < size; i++) {
        Bit *tran = &transactions[(start + i) * unit_len];
        Bit flag = 0;
        for (int j = 0; j < unit_len; j++) {
            flag |= (itemSets[item_start + j] & tran[j]) ^ itemSets[item_start + j]; 
        }
        if (!flag)
            result++;
    }

    int r = atomicAdd(&cuda_result[blockIdx.x], result);
}

void getSupport (Bit *transactions, int transaction_size, Bit *itemSets,
                 int itemSetsSize, int unit_len, int *result) {
    dim3 dimGrid(itemSetsSize);
    dim3 dimBlock(THREAD_PREBLOCK);
    int *cuda_result;
    
    for (int i = 0; i < itemSetsSize; i++)
        result[i] = 0;
    cudaMalloc((void **)&cuda_result, sizeof(int) * itemSetsSize);
    cudaMemcpy(cuda_result, result, itemSetsSize * sizeof(int), cudaMemcpyHostToDevice);

    runKernel<<<dimGrid, dimBlock>>>(transactions, transaction_size, itemSets,
                                     itemSetsSize, unit_len, cuda_result);

    cudaMemcpy(result, cuda_result, itemSetsSize * sizeof(int), cudaMemcpyDeviceToHost);
}
