#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <cmath>

using namespace std;
typedef vector<int> Items;

long double round(long double value, int pos){
    long double temp;
    temp = value * pow(10, pos);
    temp = floor(temp + 0.5);
    temp *= pow(10, -pos);
    return temp;
}

int main (int argc, char *argv[]) {
    if (argc != 4)
        cerr << "Error:\n\treducer iteration minmum_support transaction_num\n";

    const int iteration = atoi(argv[1]);
    const float support = atof(argv[2]);
    const int transaction_num = atoi(argv[3]);

    string key, num;
    map<Items, int> count;
    map<Items, int>::iterator it;

    string line;
    while (getline(cin, line)) {
        stringstream ss(line);
        Items items;
        for (int i = 0; i < iteration; i++) {
            int tmp;
            if (!(ss >> tmp)) {
                cerr << "mapper format error\n";
                exit(1);
            }
            items.push_back(tmp);
        }

        int value;
        if (!(ss >> value)) {
            cerr << "mapper format error\n";
            exit(1);
        }

        // find items in map
        it = count.find(items);
        if (it != count.end()) {
            it->second += value;
        }
        else {
            count.insert(make_pair(items, value));
        }
    }

    for (it = count.begin(); it != count.end(); it++) {
        float s = (float)it->second / transaction_num * 100;
        s = round(s, 2);
        if (s < support) {
            cerr << s << endl;
            continue;
        }
        for (auto item: it->first) {
            cout << item << " ";
        }
        cout << s << endl;
    }

    return 0;
}
