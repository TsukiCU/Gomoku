/* g++ load.cpp -o load `pkg-config --cflags --libs opencv4` */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

#define STONE 1
#define BOARD 2

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

void vecToMat(const vector<vector<vector<int>>>& vec) {
    int rows = vec.size();
    int cols = vec[0].size();
    int channels = vec[0][0].size();

    cv::Mat image(rows, cols, CV_8UC(channels));

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            cv::Vec4b& pixel = image.at<cv::Vec4b>(i, j);
            for (int c = 0; c < channels; c++) {
                pixel[c] = static_cast<uchar>(vec[i][j][c]);
            }
        }
    }

    cv::imshow("Image from Vector", image);
    cv::waitKey(0);
}

int main()
{
    int stone_or_board = STONE;

    if (stone_or_board == STONE) {
        vector<vector<vector<int>>> image(29, vector<vector<int>>(29, vector<int>(4, 0)));
        loadVector(image, "result/white.txt");
        vecToMat(image);
    }

    else {
        vector<vector<vector<int>>> image(500, vector<vector<int>>(480, vector<int>(4, 0)));
        loadVector(image, "result/board_0.txt");
        vecToMat(image);
    }
}