#include <iostream>
#include <fstream>

using namespace std;

void loadVector(vector<vector<vector<int>>>& vec, const string& file) {
    ifstream ifs(file, ios::binary);
    for (auto& row : vec) {
        for (auto& col : row) {
            for (int& val : col) {
                ifs.read(reinterpret_cast<char*>(&val), sizeof(int));
            }
        }
    }
}

int main()
{}