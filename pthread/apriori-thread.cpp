#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <tuple>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <omp.h>
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

class Apriori {
private:
    int nowStep;
    int kFrequentWorkPtr;
    int kItemSetsWorkPtr;
    long double minSupport;
    ItemSets transactions;
    ItemSets kItemSets;
    KFrequentItems kFrequentItems;
    FrequentTable frequentTable;
    vector<tuple<vector<int>, vector<int>, long double, long double>> associationRules;
    const int thread_num;
    // FIX: remove purningLSet
    set<Items> purningLSet[16];

public:
    Apriori (ItemSets _transactions, long double _minSupport, int thread_num = 1) : thread_num(thread_num) {
        nowStep = 0;
        minSupport = _minSupport;
        for(Items &row: _transactions){
            sort(row.begin(), row.end());
            transactions.push_back(row);
        }
    }

    vector<tuple<vector<int>, vector<int>, long double, long double> > getAssociationRules(){
        return associationRules;
    }

    void process() {
        while(true) {
            nowStep++;
            thread workers[thread_num];

            //=================================================================
            // step 1
            //=================================================================
            ItemSets kis[thread_num];
            kItemSetsWorkPtr = 0;
            kItemSets.clear();
            // TODO: fine-tune the parameters
            if (nowStep != 1 && kItemSets.size() > 100) {
                for (int i = 1; i < thread_num; i++) {
                    workers[i] = thread(&Apriori::generateNextKItems, this, &kis[i], i);
                }
                generateNextKItems(&kis[0], 0);

                for (int i = 1; i < thread_num; i++)
                    workers[i].join();

                for (int i = 0; i < thread_num; i++)
                    for (auto t: kis[i])
                        kItemSets.push_back(t);
            }
            else
                generateNextKItems(&kItemSets, 0);
            if (kItemSets.size() == 0) break;

            //=================================================================
            // step 2
            //=================================================================
            KFrequentItems kfi[thread_num];
            kFrequentWorkPtr = 0;
            for (int i = 1; i < thread_num; i++) {
                workers[i] = thread(&Apriori::generateKFrequentItems, this, &kfi[i], i);
            }
            generateKFrequentItems(&kfi[0], 0);

            for (int i = 1; i < thread_num; i++)
                workers[i].join();

            for (int i = 1; i < thread_num; i++)
                for (int j = 0; j < kfi[0].size(); j++)
                    kfi[0][j].support += kfi[i][j].support;

            //=================================================================
            // step 3
            //=================================================================
            kFrequentItems.clear();
            for (FrequentItem fi: kfi[0])
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

    void generateNextKItems (ItemSets *set, int id) {
        if(nowStep == 1) {
            Items element = getElement(transactions);
            for(auto &i: element)
                set->push_back(vector<int>(1, i));
        }
        else {
            // TODO: fine-tune workload
            int size = kFrequentItems.size() / thread_num / 4;
            if (size == 0) {
                if (id != 0) return;
                size = kFrequentItems.size();
            }
            int index = 0;
            purningLSet[id].clear();
            for(FrequentItem &row: kFrequentItems) purningLSet[id].insert(row.items);
            while (true) {
                index = __sync_fetch_and_add(&kItemSetsWorkPtr, size);
                if (index >= kFrequentItems.size()) break;
                int s = index + size <= kFrequentItems.size() ? size : kFrequentItems.size() - index;
                ItemSets tmp = pruning(joining(index, index + s), id);
                for (auto t: tmp)
                    set->push_back(t);
            }
        }
    }

    ItemSets joining (int start, int end) {
        ItemSets ret;
        for(int i = start; i < end; i++){
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

    ItemSets pruning (ItemSets joined, int id) {
        ItemSets ret;

        for(Items &row: joined){
            int i;
            for(i = 0; i < row.size(); i++) {
                Items tmp = row;
                tmp.erase(tmp.begin() + i);
                if(purningLSet[id].find(tmp) == purningLSet[id].end()) {
                    break;
                }
            }
            if(i == row.size()){
                ret.push_back(row);
            }
        }
        return ret;
    }

    inline long double getSupport (Items &item, int start, int size, int id = 0) {
        int ret = 0;
        for(int index = start; index < start + size; index++){
            Items &row = transactions[index];
            int i, j;
            if(row.size() < item.size()) continue;
            for(i=0, j=0; i < row.size(); i++) {
                // throttle
                if(j == item.size()) break;
                if(row[i] == item[j]) j++;
            }
            if(j == item.size()){
                ret++;
            }
        }
        return (long double)ret / transactions.size() * 100.0;
    }

    void generateKFrequentItems (KFrequentItems *kfi, int id) {
        for (Items &items: kItemSets)
            kfi->push_back({items, 0});

        // TODO: fine-tune workload
        int size = transactions.size() / thread_num / 4;
        if (size == 0) {
            if (id != 0) return;
            size = transactions.size();
        }
        int index = 0;
        while (true) {
            index = __sync_fetch_and_add(&kFrequentWorkPtr, size);
            if (index >= transactions.size()) break;
            int s = index + size <= transactions.size() ? size : transactions.size() - index;
            for (int i = 0; i < kfi->size(); i++){
                long double support = getSupport(kfi->at(i).items, index, s);
                kfi->at(i).support = kfi->at(i).support + support;
            }
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

    const int max_thread_num = thread::hardware_concurrency();
    cout << "max number of threads: " << max_thread_num << endl;
    double start[max_thread_num], end[max_thread_num];

    for (int i = 0; 1 << i <= max_thread_num; i++) {
        int threads = 1 << i;
        Apriori apriori(transactions, stold(minSupport), threads);
        start[i] = CycleTimer::currentSeconds();
        apriori.process();
        end[i] = CycleTimer::currentSeconds();
        OutputPrinter outputPrinter(outputFileName, apriori.getAssociationRules());
    }

    for (int i = 0; 1 << i <= max_thread_num; i++) {
        int threads = 1 << i;
        cout << "threads: " << threads
             << ", time(s): " << end[i] - start[i]
             << ", speedup: " << (end[0] - start[0]) / (end[i] - start[i])
             << endl;
    }

    return 0;
}
