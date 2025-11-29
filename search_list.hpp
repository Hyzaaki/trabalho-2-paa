#pragma once
#include "image_item.hpp"
#include <vector>
#include <string>
#include <iostream>
using namespace std;

// Resultado da busca na lista
class ListSearchResult {
public:
    string id;
    float distance;

    ListSearchResult(string &id_, float distance_) {
        id = id_;
        distance = distance_;
    }

    void print() {
        cout << "-> Imagem mais similar encontrada:" << endl;
        cout << id << " | dist = " << distance << endl;
    }
};

float chiSquare(vector<float> &referenceHistogram, vector<float> &queryHistogram);

ListSearchResult searchMostSimilar(vector<ImageItem> &index, ImageItem &queryImage);
