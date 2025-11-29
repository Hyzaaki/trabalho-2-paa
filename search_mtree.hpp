#pragma once
#include "image_item.hpp"
#include <vector>
#include <memory>
#include <limits>
#include <cmath>
#include <iostream>
using namespace std;

// Função chi-square para o histograma
inline float chiSquareHist(const vector<float> &h1, const vector<float> &h2)
{
    float sum = 0.0f;
    for (size_t i = 0; i < h1.size(); i++)
    {
        float denom = h1[i] + h2[i];
        if (denom != 0)
        {
            float diff = h1[i] - h2[i];
            sum += (diff * diff) / denom;
        }
    }
    return sum;
}

// Nó da M-Tree
class MTNode
{
public:
    ImageItem obj;        // objeto representativo (pivô)
    float coveringRadius; // raio que cobre seus filhos
    bool leaf;

    vector<ImageItem> items; // usado se for folha
    vector<unique_ptr<MTNode>> children;

    MTNode(const ImageItem &o, bool isLeaf = true)
        : obj(o), leaf(isLeaf), coveringRadius(0.0f) {}
};

// Estrutura de Resultado da busca
class MTreeSearchResult
{
public:
    string id;
    float distance;

    MTreeSearchResult(const string &id_, float dist_)
        : id(id_), distance(dist_) {}

    void print()
    {
        cout << "-> Imagem mais similar encontrada:" << endl;
        cout << id << " | dist = " << distance << endl;
    }
};

// Classe da M-Tree
class MTree
{
private:
    unique_ptr<MTNode> root;
    const int maxLeafSize = 3;

public:
    MTree() {}

    void insert(const ImageItem &item)
    {
        if (!root)
        {
            root = make_unique<MTNode>(item, true);
            root->items.push_back(item);
            return;
        }
        insertRecursive(root.get(), item);
    }

    // Busca: retorna o mais similar
    MTreeSearchResult searchMostSimilar(const ImageItem &query)
    {
        string bestId = "";
        float bestDist = numeric_limits<float>::infinity();
        searchRecursive(root.get(), query, bestId, bestDist);
        return MTreeSearchResult(bestId, bestDist);
    }

private:
    // Inserção recursiva
    void insertRecursive(MTNode *node, const ImageItem &item)
    {
        float dist = chiSquareHist(node->obj.histogram, item.histogram);

        // Atualiza raio de cobertura
        if (dist > node->coveringRadius)
            node->coveringRadius = dist;

        if (node->leaf)
        {
            node->items.push_back(item);

            // split simples (promoção)
            if ((int)node->items.size() > maxLeafSize)
                splitLeaf(node);
        }
        else
        {
            // Escolhe filho mais próximo ao item
            MTNode *bestChild = nullptr;
            float bestChildDist = numeric_limits<float>::infinity();

            for (auto &child : node->children)
            {
                float d = chiSquareHist(child->obj.histogram, item.histogram);
                if (d < bestChildDist)
                {
                    bestChild = child.get();
                    bestChildDist = d;
                }
            }

            insertRecursive(bestChild, item);
        }
    }

    // Split de folha (simples)
    void splitLeaf(MTNode *node)
    {
        if (!node->leaf)
            return;

        // promove 1 como pivô novo
        ImageItem newPivot = node->items.back();
        node->items.pop_back();

        auto newChild = make_unique<MTNode>(newPivot, true);
        newChild->items.push_back(newPivot);

        // distribuir elementos
        vector<ImageItem> remaining = node->items;
        node->items.clear();

        for (auto &it : remaining)
        {
            float d1 = chiSquareHist(node->obj.histogram, it.histogram);
            float d2 = chiSquareHist(newPivot.histogram, it.histogram);

            if (d1 < d2)
                node->items.push_back(it);
            else
                newChild->items.push_back(it);
        }

        // agora vira nó interno (split root)
        if (node == root.get())
        {
            auto newRoot = make_unique<MTNode>(node->obj, false);
            newRoot->children.push_back(move(root));
            newRoot->children.push_back(move(newChild));
            root = move(newRoot);
        }
        else
        {
            node->leaf = false;
            node->children.push_back(make_unique<MTNode>(node->obj, true));
            node->children.push_back(move(newChild));
        }
    }

    // Busca recursiva (k=1)
    void searchRecursive(MTNode *node, const ImageItem &query,
                         string &bestId, float &bestDist)
    {
        float distToPivot = chiSquareHist(node->obj.histogram, query.histogram);

        if (distToPivot < bestDist)
        {
            bestDist = distToPivot;
            bestId = node->obj.id;
        }

        if (node->leaf)
        {
            for (auto &it : node->items)
            {
                float d = chiSquareHist(it.histogram, query.histogram);
                if (d < bestDist)
                {
                    bestDist = d;
                    bestId = it.id;
                }
            }
        }
        else
        {
            for (auto &child : node->children)
            {
                float pivotDist = chiSquareHist(child->obj.histogram, query.histogram);

                // poda
                if (pivotDist - child->coveringRadius > bestDist)
                    continue;

                searchRecursive(child.get(), query, bestId, bestDist);
            }
        }
    }
};
