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

class Apriori {
private:
    int nowStep;
    long double minSupport;
    ItemSets transactions;
    ItemSets kItemSets;
    KFrequentItems kFrequentItems;
    FrequentTable frequentTable;
    vector<tuple<vector<int>, vector<int>, long double, long double>> associationRules;
public:
    Apriori (ItemSets _transactions, long double _minSupport) {
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
            // step 1
            kItemSets = generateNextKItems();
            if (kItemSets.size() == 0) break;

            // step 2
            KFrequentItems frequentItems = generateKFrequentItems();

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

    ItemSets generateNextKItems () {
        if(nowStep == 1) {
            ItemSets ret;
            Items element = getElement(transactions);
            for(auto &i: element)
                ret.push_back(vector<int>(1, i));
            return ret;
        } else {
            return pruning(joining());
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

    ItemSets pruning (ItemSets joined) {
        ItemSets ret;

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
        return ret;
    }

    inline long double getSupport (Items item) {
        int ret = 0;
        // TODO: parallize
        for(auto &row: transactions){
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

    KFrequentItems generateKFrequentItems () {
        KFrequentItems ret;
        for(auto &row: kItemSets){
            long double support = getSupport(row);
            ret.push_back({row, support});
        }
        return ret;
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

    /*
       vector<vector<int> > transactions = {
       {1, 2, 5},
       {2,4},
       {2,3},
       {1, 2, 4},
       {1, 3},
       {2, 3},
       {1, 3},
       {1, 2, 3, 5},
       {1, 2, 3}
       };
       */

    InputReader inputReader(inputFileName);
    ItemSets transactions = inputReader.getTransactions();
    Apriori apriori(transactions, stold(minSupport));

    double start, end;
    start = CycleTimer::currentSeconds();
    apriori.process();
    end = CycleTimer::currentSeconds();
    cout << "time(s): " << end - start << endl;

    OutputPrinter outputPrinter(outputFileName, apriori.getAssociationRules());

    /*
       for test
       Checker checker("output5.txt", "outputRsupport5.txt");
       checker.compare();
       */


    return 0;
}
