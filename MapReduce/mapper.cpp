#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

typedef vector<int> Items;
typedef vector<Items> ItemsSet;

int main (int argc, char *argv[]) {
    if (argc != 2)
        cerr << "Error:\n\tmapper iteration\n";

    if (atoi(argv[1]) == 1) {
        int item;
        while (cin >> item) {
            cout << item << "\t" << 1 << endl;
        }
        return 0;
    }

    const int iteration = atoi(argv[1]);
    char path[256];
    if (!getcwd(path, 256)) {
        cerr << "Fail to get current path\n";
        exit(1);
    }
    string fileName(path);
    fileName += "/itemset";
    fileName += to_string(iteration - 1) + ".txt";

    ifstream fin;
    fin.open(fileName);
    string line;
    ItemsSet preSet, candidates;
    while (getline(fin, line)) {
        stringstream ss(line);
        Items items;
        for (int i = 0; i < iteration - 1; i++) {
            int item;
            ss >> item;
            items.push_back(item);
        }
        preSet.push_back(items);
    }

    // create new candidates
    for (int i = 0; i < preSet.size(); i++) {
        for (int j = i + 1; j < preSet.size(); j++) {
            int k = 0;
            for (; k < iteration - 2; k++)
                if (preSet[i][k] != preSet[j][k])
                    break;

            if (k == iteration - 2) {
                Items newSet;
                int a = preSet[i][k];
                int b = preSet[j][k];
                if (a > b) swap(a, b);
                for (int k = 0; k < iteration - 2; k++)
                    newSet.push_back(preSet[i][k]);
                newSet.push_back(a);
                newSet.push_back(b);
                candidates.push_back(newSet);
            }
        }
    }

    // match candidates and transactions
    while (getline(cin, line)) {
        Items transaction;
        stringstream ss(line);
        int tmp;
        while (ss >> tmp) transaction.push_back(tmp);
        if (transaction.size() < iteration)
            continue;

        for (auto candidate: candidates) {
            int i, j;
            for (i = 0, j = 0; i < transaction.size(); i++) {
                if (j == iteration) break;
                if (candidate[j] == transaction[i]) j++;
            }

            if (j == iteration) {
                for (auto c: candidate)
                    cout << c << " ";
                cout << "\t1\n";
            }
        }
    }
    return 0;
}
