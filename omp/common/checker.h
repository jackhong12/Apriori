#include <vector>
#include <set>
#include <string>
#include <tuple>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <iostream>
using namespace std;

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
            //str.pop_back();
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
