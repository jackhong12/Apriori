#include <vector>
#include <set>
#include <string>
#include <tuple>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <iostream>
using namespace std;

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
            fout << vectorToString(get<0>(i)) << ' ';
            fout << vectorToString(get<1>(i)) << ' ';
            
            fout << fixed;
            fout.precision(2);
            fout << get<2>(i) << ' ';
            
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