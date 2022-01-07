#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <tuple>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <chrono>

#include "common/input_reader.h"
#include "common/output_printer.h"

using namespace std;

long double round(long double value, int pos){
    long double temp;
    temp = value * pow( 10, pos );
    temp = floor( temp + 0.5 );
    temp *= pow( 10, -pos );
    return temp;
}

class Apriori {
private:
    int nowStep;
    long double minSupport;
    vector<vector<int> > transactions;
    vector<vector<int> > C, L;
    vector<vector<vector<int> > > frequentSet;
    vector<tuple<vector<int>, vector<int>, long double, long double> > associationRules;
public:
    Apriori (vector<vector<int> > _transactions, long double _minSupport) {
        nowStep=0;
        minSupport = _minSupport;
        for(auto&row:_transactions){
            sort(row.begin(), row.end());
            transactions.push_back(row);
        }
        frequentSet.push_back({{}});
    }
    
    vector<tuple<vector<int>, vector<int>, long double, long double> > getAssociationRules(){
        return associationRules;
    }
    
    void process() {
        while(true) {
            C = generateNextC();
            if(C.size()==0) break;
            nowStep++;
            
            L = generateL();
            frequentSet.push_back(L);
        }
        
        for(auto&stepItemSet:frequentSet) {
            for(auto&items:stepItemSet) {
                generateAssociationRule(items, {}, {}, 0);
            }
        }
    }
    
    void generateAssociationRule(vector<int> items, vector<int> X, vector<int> Y, int index) {
        if(index == items.size()) {
            if(X.size()==0 || Y.size() == 0) return;
            long double XYsupport = getSupport(items);
            long double Xsupport = getSupport(X);
            
            if(Xsupport == 0) return;
            
            long double support = (long double)XYsupport;
            long double confidence = (long double)XYsupport/Xsupport*100.0;
            associationRules.push_back({X, Y, support, confidence});
            return;
        }
        
        X.push_back(items[index]);
        generateAssociationRule(items, X, Y, index+1);
        X.pop_back();
        Y.push_back(items[index]);
        generateAssociationRule(items, X, Y, index+1);
    }
    
    vector<int> getElement(vector<vector<int> > itemset) {
        vector<int> element;
        set<int> s;

        for(auto&row:itemset) 
            for(auto&col:row) 
                s.insert(col);

        for(auto iter=s.begin(); iter != s.end(); iter++)
            element.push_back(*iter);
        return element;
    }
    
    vector<vector<int> > generateNextC() {
        if(nowStep==0) {
            vector<vector<int> > ret;
            vector<int> element = getElement(transactions);

            for(auto&i:element) 
                ret.push_back(vector<int>(1, i));
            
            return ret;
        } else {
            return pruning(joining());
        }
    }
    
    vector<vector<int> > joining () {
        vector<vector<int> > ret;
        #pragma omp parallel for 
        for(int i=0;i<L.size();i++){
            for(int j=i+1;j<L.size(); j++) {
                int k;
                for(k=0;k<nowStep-1; k++) {
                    if(L[i][k] != L[j][k]) break;
                }
                if(k == nowStep-1) {
                    vector<int> tmp;
                    for(int k=0;k<nowStep-1; k++) {
                        tmp.push_back(L[i][k]);
                    }
                    int a = L[i][nowStep-1];
                    int b = L[j][nowStep-1];
                    if(a>b) swap(a,b);
                    tmp.push_back(a), tmp.push_back(b);
                    #pragma omp critical
                    ret.push_back(tmp);
                }
            }
        }
        return ret;
    }
    
    vector<vector<int> > pruning (vector<vector<int> > joined) {
        vector<vector<int> > ret;
        
        set<vector<int> > lSet;
        for(auto&row:L) lSet.insert(row);

        #pragma omp parallel for 
        for(auto&row:joined){
            int i;
            for(i=0;i<row.size();i++){
                vector<int> tmp = row;
                tmp.erase(tmp.begin()+i);
                if(lSet.find(tmp) == lSet.end()) {
                    break;
                }
            }
            if(i==row.size()){
                #pragma omp critical
                ret.push_back(row);
            }
        }
        return ret;
    }
    
    long double getSupport(vector<int> item) {
        int ret = 0;
        #pragma omp parallel for reduction(+:ret)
        for(auto&row:transactions){
            int i, j;
            if(row.size() < item.size()) continue;
            for(i=0, j=0; i < row.size();i++) {
                if(j==item.size()) break;
                if(row[i] == item[j]) j++;
            }
            if(j==item.size()){
                ret++;
            }
        }
        return (long double)ret/transactions.size()*100.0;
    }
    
    vector<vector<int> > generateL() {
        vector<vector<int> > ret;
        #pragma omp parallel for 
        for(auto&row:C){
            long double support = getSupport(row);
            if(round(support, 2) < minSupport) continue;
            #pragma omp critical
            ret.push_back(row);
        }
        return ret;
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
    vector<vector<int> > transactions = inputReader.getTransactions();
    Apriori apriori(transactions, stold(minSupport));
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    apriori.process();
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(end - begin).count() << endl;
    OutputPrinter outputPrinter(outputFileName, apriori.getAssociationRules());
    
    /*
    for test
    Checker checker("output5.txt", "outputRsupport5.txt");
    checker.compare();
    */

    
    return 0;
}
