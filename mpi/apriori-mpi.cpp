#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <tuple>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include "CycleTimer.hpp"
#include <mpi.h>
using namespace std;

typedef vector<int> Items;

struct FrequentItem {
    Items items;
    long double support;
};

typedef vector<Items> ItemSets;
typedef vector<FrequentItem> KFrequentItems;
typedef vector<KFrequentItems> FrequentTable;

long double round(long double value, int pos){
    long double temp;
    temp = value * pow(10, pos);
    temp = floor(temp + 0.5);
    temp *= pow(10, -pos);
    return temp;
}

#define Bit unsigned long
#define BitLen (sizeof(Bit) * 8)
#define BitShift __builtin_ctz(BitLen)
#define BitMask (BitLen - 1)
#define lookupBit(item) (bit_map_table[item])

int *bit_map_table;
int bit_map_table_size = 0;

// use each bit to represent a item
struct ItemsBit {
    Bit *data;
    int size;       // the number of transactions
    int unit_len;   // the number of Bits in each transactions
    int bit_len;    // the number of item types
};

inline void setBit(Bit *ptr, int item) {
    int pos = bit_map_table[item];
    int offset = pos >> BitShift;
    int shift = pos & BitMask;
    ptr[offset] |= (Bit)1 << shift;
}


void initBitTable (ItemSets &trans) {
    int max_item = -1;
    for (auto &t: trans) {
        for (auto &i: t) {
            if (max_item < i)
                max_item = i;
        }
    }
    bit_map_table = new int [max_item + 1];
    bit_map_table_size = max_item + 1;

    for (int i = 0; i < bit_map_table_size; i++)
        bit_map_table[i] = i;
}



void initItemsBit (ItemsBit *ptr, ItemSets &isets) {
    ptr->size = isets.size();
    ptr->bit_len = bit_map_table_size;
    ptr->unit_len = (ptr->bit_len + BitMask) >> BitShift;

    ptr->data = new Bit [ptr->size * ptr->unit_len];
    for (int i = 0; i < isets.size(); i++) {
        int index = ptr->unit_len * i;
        for (int j = 0; j < ptr->unit_len; j++)
            ptr->data[index + j] = 0;
        for (auto item: isets[i]) {
            setBit(&(ptr->data[index]), item);
        }
    }

    printf("ptr size=%d\n", ptr->size);
}

void releaseItemsBit (ItemsBit *ptr) {
    delete [] ptr->data;
}

class Apriori {
private:
    int nowStep;
    long double minSupport;
    ItemSets transactions;
    ItemSets kItemSets;
    KFrequentItems kFrequentItems;
    FrequentTable frequentTable;
    vector<tuple<vector<int>, vector<int>, long double, long double>> associationRules;
    ItemsBit transactionBit;

public:
    Apriori (ItemSets _transactions, ItemsBit _transactionBit, long double _minSupport) {
        nowStep = 0;
        minSupport = _minSupport;
        transactions = _transactions;
        transactionBit = _transactionBit;
    }

    ~Apriori () {
        releaseItemsBit(&transactionBit);
    }

    vector<tuple<vector<int>, vector<int>, long double, long double> > getAssociationRules(){
        return associationRules;
    }

    void process() {
        int world_rank, world_size;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
        int num_requests = world_size-1;
        MPI_Request requests[num_requests];
        MPI_Status status[num_requests];
        while(true) {
            nowStep++;
            

            // MPI
            if(world_rank==0){
                // step 1
                kItemSets.clear();
                generateNextKItems(kItemSets);

                bool end_exploring = kItemSets.size() == 0;
                for (int i = 1; i<world_size; i++) {
                    MPI_Isend(&end_exploring, 1, MPI_C_BOOL, i, 0, MPI_COMM_WORLD, &requests[i-1]);
                }
                if (end_exploring) break;

                // step 2
                ItemsBit kItemsBit;
                initItemsBit(&kItemsBit, kItemSets);

                int work_segment[world_size+1];
                work_segment[world_size] = kItemsBit.size;
                for (int i=0; i<world_size; i++) {
                    work_segment[i] = (kItemsBit.size/world_size) * i;
                }
                printf("step2-1\n");

                for (int i = 1; i<world_size; i++) {
                    int work_size = work_segment[i+1] - work_segment[i];
                    MPI_Send(&work_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                    MPI_Send(&kItemsBit.unit_len, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                    MPI_Isend(&kItemsBit.data[work_segment[i]], work_size, MPI_UNSIGNED_LONG, i, 0, MPI_COMM_WORLD, &requests[i-1]);
                }
                printf("step2 send\n");

                long double** supports = (long double**)malloc(world_size*sizeof(long double*));
                ItemsBit sub_kItemsBit = kItemsBit;
                sub_kItemsBit.size = work_segment[1] - work_segment[0];
                supports[0] = generateKFrequentItems(&sub_kItemsBit);
                printf("step2 support\n");

                // merge supports 
                for (int i = 1; i<world_size; i++) {
                    int work_size = work_segment[i+1]-work_segment[i];
                    supports[i] = (long double*)malloc(work_size*sizeof(long double));
                    MPI_Irecv(&supports[i], work_size, MPI_LONG_DOUBLE, i, 0, MPI_COMM_WORLD, &requests[i-1]);
                }
                MPI_Waitall(num_requests, requests, status);
                
                printf("step2 recv\n");

                releaseItemsBit(&kItemsBit);
                KFrequentItems frequentItems;
                int work_size = work_segment[1]-work_segment[0];
                for (int i = 0; i < kItemsBit.size; i++) {
                    int j = i/work_size, k = i%work_size;
                    frequentItems.push_back({kItemSets[i], supports[j][k]});
                }
                printf("step2\n");



                // step 3
                kFrequentItems.clear();
                for (FrequentItem fi: frequentItems)
                    if (round(fi.support, 2) > minSupport)
                        kFrequentItems.push_back(fi);
                frequentTable.push_back(kFrequentItems);

            } else if (world_rank>0) {
                MPI_Status status;
                bool end_exploring;
                MPI_Recv(&end_exploring, 1, MPI_C_BOOL, 0, 0, MPI_COMM_WORLD, &status);
                if (end_exploring) break;

                // step 2
                int work_size, unit_len;
                MPI_Recv(&work_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
                MPI_Recv(&unit_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
                Bit* sub_Bits = (Bit*) malloc(work_size*sizeof(Bit));
                MPI_Recv(sub_Bits, work_size, MPI_UNSIGNED_LONG, 0, 0, MPI_COMM_WORLD, &status);
                
                
                ItemsBit sub_kItemsBit;
                sub_kItemsBit.data = sub_Bits;
                sub_kItemsBit.size = work_size;
                sub_kItemsBit.unit_len = unit_len;

                long double* supports = generateKFrequentItems(&sub_kItemsBit);

                MPI_Send(supports, work_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
                free(supports);
                free(sub_Bits);
            }


            
        }

        if(world_rank==0){
            for(KFrequentItems &kfItems: frequentTable) {
                for(FrequentItem &f: kfItems) {
                    generateAssociationRule(f.items, {}, {}, 0);
                }
            }
        }
    }

    long double lookupFrequentTable (Items items) {
        int k = items.size();
        for (FrequentItem &fi: frequentTable[k - 1]) {
            int i;
            for (i = 0; i < k; i++) {
                if (fi.items[i] != items[i])
                    break;
            }
            if (i == k)
                return fi.support;
        }
        return 0;
    }

    void generateAssociationRule (Items items, Items X, Items Y, int index) {
        if(index == items.size()) {
            if(X.size()==0 || Y.size() == 0) return;
            long double XYsupport = lookupFrequentTable(items);
            long double Xsupport = lookupFrequentTable(X);

            if(Xsupport == 0) return;

            long double support = (long double)XYsupport;
            long double confidence = (long double)XYsupport / Xsupport * 100.0;
            associationRules.push_back({X, Y, support, confidence});
            return;
        }

        X.push_back(items[index]);
        generateAssociationRule(items, X, Y, index+1);
        X.pop_back();
        Y.push_back(items[index]);
        generateAssociationRule(items, X, Y, index+1);
    }

    Items getElement (ItemSets itemset) {
        Items element;
        set<int> s;
        for (Items &row: itemset)
            for (int &col: row) s.insert(col);
        for (auto iter=s.begin(); iter != s.end(); iter++)
            element.push_back(*iter);
        return element;
    }

    void generateNextKItems (ItemSets &ret) {
        if(nowStep == 1) {
            Items element = getElement(transactions);
            for(auto &i: element)
                ret.push_back(vector<int>(1, i));
        } else {
            return pruning(joining(), ret);
        }
    }

    ItemSets joining () {
        ItemSets ret;
        for(int i = 0; i < kFrequentItems.size(); i++){
            for(int j = i + 1; j < kFrequentItems.size(); j++) {
                Items &f1 = kFrequentItems[i].items;
                Items &f2 = kFrequentItems[j].items;
                int k;
                for(k = 0; k < nowStep - 2; k++) {
                    if(f1[k] != f2[k]) break;
                }
                if(k == nowStep - 2) {
                    Items tmp;
                    for(int k=0; k < nowStep - 2; k++) {
                        tmp.push_back(f1[k]);
                    }
                    int a = f1[nowStep - 2];
                    int b = f2[nowStep - 2];
                    if(a > b) swap(a, b);
                    tmp.push_back(a), tmp.push_back(b);
                    ret.push_back(tmp);
                }
            }
        }
        return ret;
    }

    void pruning (ItemSets joined, ItemSets &ret) {
        set<Items> lSet;
        for(FrequentItem &row: kFrequentItems) lSet.insert(row.items);

        for(Items &row: joined){
            int i;
            for(i = 0; i < row.size(); i++) {
                Items tmp = row;
                tmp.erase(tmp.begin() + i);
                if(lSet.find(tmp) == lSet.end()) {
                    break;
                }
            }
            if(i == row.size()){
                ret.push_back(row);
            }
        }
    }

    inline long double getSupportSIMD (Bit *items) {
        int ret = 0;
        for (int i = 0; i < transactionBit.size; i++) {
            Bit *trans = &transactionBit.data[i * transactionBit.unit_len];
            Bit flag = 0;
            for (int j = 0; j < transactionBit.unit_len; j++) {
                flag |= items[j] & trans[j] ^ items[j];
            }
            if (!flag) ret++;
        }
        return (long double)ret /transactionBit.size * 100.0;

    }

    long double* generateKFrequentItems (ItemsBit *ptr) {
        long double* supports = (long double*) malloc(ptr->size*sizeof(long double));
        printf("size %d\n", ptr->size);
        for (int i = 0; i < ptr->size; i++) {
            int index = i * ptr->unit_len;
            long double support = getSupportSIMD(&ptr->data[index]);
            //ret.push_back({kItemSets[i], support});
            supports[i] = support;
            printf("%d\n",i);
        }
        printf("kF done\n");
        return supports;
    }
};

class InputReader {
private:
    ifstream fin;
    ItemSets transactions;
public:
    InputReader (string filename) {
        fin.open(filename);
        if(!fin) {
            cout << "Input file could not be opened\n";
            exit(0);
        }
        parse();
    }
    void parse() {
        string str;
        while (!getline(fin, str).eof()) {
            vector<int> arr;
            int pre = 0;
            for (int i = 0; i < str.size(); i++){
                if(str[i] == '\t') {
                    int num = atoi(str.substr(pre, i).c_str());
                    arr.push_back(num);
                    pre = i+1;
                }
            }
            int num = atoi(str.substr(pre, str.size()).c_str());
            arr.push_back(num);

            transactions.push_back(arr);
        }
    }
    ItemSets getTransactions() {
        return transactions;
    }
};

class OutputPrinter {
private:
    ofstream fout;
    vector<tuple<vector<int>, vector<int>, long double, long double> > associationRules;
public:
    OutputPrinter(string filename, vector<tuple<vector<int>, vector<int>, long double, long double> > _associationRules) {
        fout.open(filename);
        if(!fout) {
            cout << "Ouput file could not be opened\n";
            exit(0);
        }
        associationRules = _associationRules;
        buildOutput();
    }

    void buildOutput() {
        for(auto&i:associationRules) {
            fout << vectorToString(get<0>(i)) << '\t';
            fout << vectorToString(get<1>(i)) << '\t';

            fout << fixed;
            fout.precision(2);
            fout << get<2>(i) << '\t';

            fout << fixed;
            fout.precision(2);
            fout << get<3>(i);

            fout << endl;
        }
    }

    string vectorToString(vector<int> arr) {
        string ret = "{";
        for(int i=0;i<arr.size();i++){
            ret += to_string(arr[i]);
            if(i != arr.size()-1){
                ret += ",";
            }
        }
        ret += "}";
        return ret;
    }
};

class Checker {
public:
    ifstream fin1, fin2;
    set<string> s1;
    Checker(string filename1, string filename2) {
        fin1.open(filename1);
        fin2.open(filename2);

        if(!fin1 || !fin2) {
            cout << "Input file could not be opened\n";
            exit(0);
        }
    }
    void compare() {
        file1ToSet();

        string str;
        while(!getline(fin2, str).eof()){
            str.pop_back();
            if(s1.find(str) == s1.end()) {
                cout << "failed at " << str <<  endl;
                return;
            }
        }
    }
    void file1ToSet() {
        string str;
        while(!getline(fin1, str).eof()){
            s1.insert(str);
        }
    }
};

int main (int argc, char ** argv) {
    if(argc!=4) {
        cout << "error : The number of parameters must be 3";
        return 0;
    }
    string minSupport(argv[1]);
    string inputFileName(argv[2]);
    string outputFileName(argv[3]);

    MPI_Init(NULL, NULL);

    int world_rank, world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


    ItemSets transactions, sorted_transactions;
    ItemsBit transactionBit;
    if (world_rank==0){
        InputReader inputReader(inputFileName);
        transactions = inputReader.getTransactions();
        initBitTable(transactions);

        for(Items &row: transactions){
            sort(row.begin(), row.end());
            sorted_transactions.push_back(row);
        }
        initItemsBit(&transactionBit, sorted_transactions);
    }


    MPI_Bcast(&transactionBit.size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Bcast(&transactionBit.unit_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&transactionBit.bit_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (world_rank>0) transactionBit.data = (Bit*) malloc(transactionBit.size*sizeof(Bit));
    MPI_Bcast(transactionBit.data, transactionBit.size, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);

    MPI_Bcast(&bit_map_table_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (world_rank>0) bit_map_table = (int*) malloc(bit_map_table_size*sizeof(int));
    MPI_Bcast(bit_map_table, bit_map_table_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Only master has transactions and transactionBit,
    // while the others have transactionBit
    Apriori apriori(transactions, transactionBit, stold(minSupport));

    double start, end;
    start = CycleTimer::currentSeconds();
    apriori.process();
    end = CycleTimer::currentSeconds();
    cout << "time(s): " << end - start << endl;
    
    if (world_rank==0) {
        OutputPrinter outputPrinter(outputFileName, apriori.getAssociationRules());
    }

    MPI_Finalize();


    return 0;
}