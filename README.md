Parallel Apriori Algorithm
==================

C++ Implementation of Parallel Apriori Algorithm

This project modified from [bowbowbow/Apriori](https://github.com/bowbowbow/Apriori). We provide 6 different parallel methods:

1. [Pthread](./pthread)
2. [OpenMP](./omp)
3. [MPI](./mpi)
4. [SIMD](./simd)
5. [CUDA](./cuda)
6. [MapReduce](./map-reduce)

The test case we used is a renowned transactional database [T10I4D100K](https://data.mendeley.com/datasets/4hz2vcvxhp/1).

## Enviroment

- OS: Ubuntu 20.04.3 LTS
- g++: 9.3.0
- MPI: 4.0.3
- CUDA: 11.5
- Hadoop: 3.3.0

## Run Apriori Algorithm

1. Move to the folder you want.

   ```bash
   cd [folder]
   ```



2. Follow the steps in `README.md` to install the dependencies.

3. Build executables.

   ```bash
   make
   ```



4. Run the Apriori algorithm with following command.

   ```bash
   ./[executable] [minimum support] [input transactions] [output result]
   ```

### Example

- For instance, if you want to run the pthread version of the Apriori algorithm, you can use the below commands.

  ```bash
  cd pthread
  make
  ./apriori-thread 1 ../testcases/transactional_T10I4D100K.csv output.txt
  ```

## Details
### Summary of the algorithm
============
Apriori algorithm is to find frequent itemsets using an iterative level-wise approach based on candidate generation.
> **Input**: A database of transactions, the minimum support count threshold
>
> **Output**: frequent itemsets in the database

The algorithm solves the problem with a two-step approach.

**Step1**. Frequent Itemset Generation
> Generate all itemsets whose support with a value of minimum support or greater.
>
> But the proccess requires a lot of computation (O(3^(k)-2^(k+1)+1), k=the number of item).
>
> `The key idea of apriori algorithm`to reduce this operation is that any subset of a frequent itemset must be frequent.
>
> Therefore, if there is any itemset which is infrequent, its superset should not be generated/tested.
>
> As a result, follow the steps below to proceed with this process.
>
> 1. Generate length (k+1)-candidate itemsets from length k frequent itemsets. (This process is called joining.)
>
> 2. And delete newly generated (k+1)-items if the item set that removed one element is not in (k)-candidate. (This process is called pluning.)
>
> 3. Calculate the support of the candidates and remove candidates with support less than min support.
>
> 4. Proceed 1 again until there are no more candidates left.

**Step2**. Associate Rule Generation
> Generate high confidence rules from each frequent itemset, where each rule is a binary partition of a frequent itemset.

### About input file
============

**Input file format**

[item_id]`\t`[item_id]`\n`

[item_id]`\t`[item_id]`\t`[item_id]`\t`[item_id]`\t`[item_id]`\n`

[item_id]`\t`[item_id]`\t`[item_id]`\t`[item_id]`\n`

- Row is transaction
- [item_id] is a numerical value
- There is no duplication of items in each transaction

### About output file
============

**output file format**

[item_set]`\t`[associative_item_set]`\t`[support(%)]`\t`[confidence(%)]`\n`

[item_set]`\t`[associative_item_set]`\t`[support(%)]`\t`[confidence(%)]`\n`

- Support: probability that a transaction contains [item_set] [associative_item_set]
- Confidence: conditional probability that a transaction having [item_set] also contains [associative_item_set]
- The value of support and confidence should be rounded to two decimal places.

