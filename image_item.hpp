#pragma once
#include <vector>
#include <string>
using namespace std;

class ImageItem {
public:
    string id;               
    vector<float> histogram;

    ImageItem(const string &id_, const vector<float> &histogram_) {
        id = id_;
        histogram = histogram_;
    }
};
