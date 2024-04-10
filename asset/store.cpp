/* g++ resize.cpp -o resize `pkg-config --cflags --libs opencv4` */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

using namespace std;

vector<vector<vector<int>>> matToVec(cv::Mat image, int rows, int cols)
{
    vector<vector<vector<int>>>
    result(rows, vector<vector<int>>(cols, vector<int>(4, 0)));

    if (image.empty() || image.channels() != 3) {
        /* Wrong */
        return result;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (i < image.rows && j < image.cols) {
                cv::Vec3b pixel = image.at<cv::Vec3b>(i, j);
                result[i][j][2] = pixel[0]; // B
                result[i][j][1] = pixel[1]; // G
                result[i][j][0] = pixel[2]; // R
            }
        }
    }

    return result;
}

void saveVector(const std::vector<std::vector<std::vector<int>>>& vec, const std::string& filename) {
    std::ofstream ofs(filename, std::ios::binary);
    for (const auto& row : vec) {
        for (const auto& col : row) {
            for (int val : col) {
                ofs.write(reinterpret_cast<const char*>(&val), sizeof(int));
            }
        }
    }
}

int main() {
    string array[4] = {"black", "white", "board_0", "board_1"};

    // Stones: 29*29
    for (int i=0; i<2; i++) {
        cv::Mat image = cv::imread(array[i] + ".png");
        if (image.empty()) {
            cerr << "Could not read " << array[i] << ".png" << endl;
            return 1;
        }

        cv::Mat resizedImage;
        cv::resize(image, resizedImage, cv::Size(29, 29));

        // bool result = cv::imwrite("result/"+array[i]+".png", resizedImage);
        // if (!result) {
        //     cerr << "Failed to save" << array[i] << ".png" << endl;
        //     return 1;
        // }

        /* vector : r, g, b, transparency. */
        vector<vector<vector<int>>> matrix(29, vector<vector<int>>(29, vector<int>(4, 0)));
        matrix = matToVec(resizedImage, 29, 29);
        saveVector(matrix, "result/" + array[i] + ".txt");
    }

    // board: 500*480
    for (int i=2; i<4; i++) {
        cv::Mat image = cv::imread(array[i] + ".png");
        if (image.empty()) {
            cerr << "Could not read " << array[i] << ".png" << endl;
            return 1;
        }

        cv::Mat resizedImage;
        cv::resize(image, resizedImage, cv::Size(500, 480));

        // bool result = cv::imwrite("result/"+array[i]+".png", resizedImage);
        // if (!result) {
        //     cerr << "Failed to save" << array[i] << ".png" << endl;
        //     return 1;
        // }

        vector<vector<vector<int>>> matrix(500, vector<vector<int>>(480, vector<int>(4, 0)));
        matrix = matToVec(resizedImage, 500, 480);
        saveVector(matrix, "result/" + array[i] + ".txt");
    }

    cout << "done" << endl;
    return 0;
}
