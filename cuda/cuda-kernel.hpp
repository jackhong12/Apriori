#ifndef CUDA_KERNEL_H
#define CUDA_KERNEL_H
#define Bit unsigned long

void getSupport (Bit *transactions, int transaction_size, Bit *itemSets,
                 int itemSetsSize, int unit_len, int *result);
#endif
