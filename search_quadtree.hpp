#pragma once
#include "image_item.hpp"
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <limits>
#include <cmath>
using namespace std;

// Converte histograma para ponto 2D (R,G médios)
inline pair<float,float> histogramToPoint(const vector<float>& hist) {
    int bins = 8; // mesmo usado na main
    float sumR=0, sumG=0, total=0;

    for (int r=0; r<bins; r++) {
        for (int g=0; g<bins; g++) {
            for (int b=0; b<bins; b++) {
                int idx = (r * bins + g) * bins + b;
                float val = hist[idx];
                sumR += r * val;
                sumG += g * val;
                total += val;
            }
        }
    }
    if (total == 0) return {0,0};
    return {sumR/total / bins, sumG/total / bins}; // normaliza para [0,1]
}

// Distância qui-quadrado entre histogramas
inline float chiSquare(const vector<float>& h1, const vector<float>& h2) {
    float sum = 0.0f;
    for (size_t i = 0; i < h1.size(); i++) {
        float denom = h1[i] + h2[i];
        if (denom != 0.0f) {
            float diff = h1[i] - h2[i];
            sum += (diff * diff) / denom;
        }
    }
    return sum;
}

// Nó da Quadtree
class QuadtreeNode {
public:
    float xMin, xMax, yMin, yMax;   // limites do quadrante
    vector<ImageItem> items;        // imagens armazenadas
    unique_ptr<QuadtreeNode> NE, NO, SE, SO;
    bool subdividido = false;
    int capacidade;

    QuadtreeNode(float xMin_, float xMax_, float yMin_, float yMax_, int cap = 2)
        : xMin(xMin_), xMax(xMax_), yMin(yMin_), yMax(yMax_), capacidade(cap) {}

    bool contem(const pair<float,float>& p) const {
        return (p.first >= xMin && p.first < xMax &&
                p.second >= yMin && p.second < yMax);
    }

    void subdividir() {
        float xMid = (xMin + xMax) / 2.0f;
        float yMid = (yMin + yMax) / 2.0f;

        NE = make_unique<QuadtreeNode>(xMid, xMax, yMid, yMax, capacidade);
        NO = make_unique<QuadtreeNode>(xMin, xMid, yMid, yMax, capacidade);
        SE = make_unique<QuadtreeNode>(xMid, xMax, yMin, yMid, capacidade);
        SO = make_unique<QuadtreeNode>(xMin, xMid, yMin, yMid, capacidade);

        subdividido = true;
    }

    void inserir(const ImageItem& img) {
        auto p = histogramToPoint(img.histogram);
        if (!contem(p)) return;

        if (items.size() < (size_t)capacidade) {
            items.push_back(img);
        } else {
            if (!subdividido) subdividir();
            NE->inserir(img);
            NO->inserir(img);
            SE->inserir(img);
            SO->inserir(img);
        }
    }

    void buscar(const pair<float,float>& queryPoint, const ImageItem& query,
                string& bestId, float& bestDist) const {
        // Verifica itens neste nó
        for (auto& it : items) {
            float d = chiSquare(it.histogram, query.histogram);
            if (d < bestDist) {
                bestDist = d;
                bestId = it.id;
            }
        }

        if (subdividido) {
            if (NE->contem(queryPoint)) NE->buscar(queryPoint, query, bestId, bestDist);
            else if (NO->contem(queryPoint)) NO->buscar(queryPoint, query, bestId, bestDist);
            else if (SE->contem(queryPoint)) SE->buscar(queryPoint, query, bestId, bestDist);
            else if (SO->contem(queryPoint)) SO->buscar(queryPoint, query, bestId, bestDist);
        }
    }
};

// Resultado da busca
class QuadtreeSearchResult {
public:
    string id;
    float distance;

    QuadtreeSearchResult(string id_, float dist_) : id(id_), distance(dist_) {}

    void print() {
        cout << "-> Imagem mais similar encontrada:" << endl;
        cout << id << " | dist = " << distance << endl;
    }
};

// Função principal de busca
inline QuadtreeSearchResult searchMostSimilarQuadtree(vector<ImageItem>& index, ImageItem& queryImage) {
    QuadtreeNode root(0.0f, 1.0f, 0.0f, 1.0f); // limites normalizados

    for (auto& img : index) {
        root.inserir(img);
    }

    string bestId = "";
    float bestDist = numeric_limits<float>::infinity();
    auto queryPoint = histogramToPoint(queryImage.histogram);

    root.buscar(queryPoint, queryImage, bestId, bestDist);

    return QuadtreeSearchResult(bestId, bestDist);
}
