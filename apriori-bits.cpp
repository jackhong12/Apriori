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
    Apriori (ItemSets _transactions, long double _minSupport) {
        nowStep = 0;
        minSupport = _minSupport;
        for(Items &row: _transactions){
            sort(row.begin(), row.end());
            transactions.push_back(row);
        }
        initItemsBit(&transactionBit, transactions);
    }

    ~Apriori () {
        releaseItemsBit(&transactionBit);
    }

    vector<tuple<vector<int>, vector<int>, long double, long double> > getAssociationRules(){
        return associationRules;
    }

    void process() {
        while(true) {
            nowStep++;
            // step 1
            kItemSets.clear();
            generateNextKItems(kItemSets);
            if (kItemSets.size() == 0) break;

            // step 2
            ItemsBit kItemsBit;
            initItemsBit(&kItemsBit, kItemSets);
            KFrequentItems frequentItems;
            generateKFrequentItems(frequentItems, &kItemsBit);
            releaseItemsBit(&kItemsBit);

            // step 3
            kFrequentItems.clear();
            for (FrequentItem fi: frequentItems)
                if (round(fi.support, 2) > minSupport)
                    kFrequentItems.push_back(fi);
            frequentTable.push_back(kFrequentItems);
        }

        for(KFrequentItems &kfItems: frequentTable) {
            for(FrequentItem &f: kfItems) {
                generateAssociationRule(f.items, {}, {}, 0);
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
        return (long double)ret / transactions.size() * 100.0;

    }

    void generateKFrequentItems (KFrequentItems &ret, ItemsBit *ptr) {
        for (int i = 0; i < ptr->size; i++) {
            int index = i * ptr->unit_len;
            long double support = getSupportSIMD(&ptr->data[index]);
            ret.push_back({kItemSets[i], support});
        }
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

    InputReader inputReader(inputFileName);
    ItemSets transactions = inputReader.getTransactions();
    initBitTable(transactions);

    Apriori apriori(transactions, stold(minSupport));

    double start, end;
    start = CycleTimer::currentSeconds();
    apriori.process();
    end = CycleTimer::currentSeconds();
    cout << "time(s): " << end - start << endl;

    OutputPrinter outputPrinter(outputFileName, apriori.getAssociationRules());
    return 0;
}
