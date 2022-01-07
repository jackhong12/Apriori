#include <vector>
#include <set>
#include <string>
#include <tuple>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <iostream>
using namespace std;

class InputReader {
private:
    ifstream fin;
    vector<vector<int> > transactions;
public:
    InputReader(string filename) {
        fin.open(filename);
        if(!fin) {
            cout << "Input file could not be opened\n";
            exit(0);
        }
        parse();
    }
    void parse() {
        string str;
        while(!getline(fin, str).eof()){
            vector<int> arr;
            int pre = 0;
            for(int i=0;i<str.size();i++){
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
    vector<vector<int> > getTransactions() {
        return transactions;
    }
};