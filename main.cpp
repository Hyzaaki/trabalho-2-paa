#include "ppm_loader.hpp"
#include "search_list.hpp"
#include "search_hash.hpp"
#include "search_quadtree.hpp"
#include "search_mtree.hpp"
#include <iostream>
#include <vector>
#include <limits>
#include <chrono>
using namespace std;

// Função para medir tempo
using Clock = chrono::high_resolution_clock;

double ms(chrono::high_resolution_clock::time_point start,
          chrono::high_resolution_clock::time_point end)
{
  return chrono::duration<double, milli>(end - start).count();
}

// Gera histograma RGB
vector<float> computeRGBHistogram(const ImageRGB8 &img)
{
  const int bins = 8;
  vector<int> hist(bins * bins * bins, 0);

  for (int i = 0; i < img.width * img.height; i++)
  {
    unsigned char R = img.data[i * 3 + 0];
    unsigned char G = img.data[i * 3 + 1];
    unsigned char B = img.data[i * 3 + 2];

    int rBin = R * bins / 256;
    int gBin = G * bins / 256;
    int bBin = B * bins / 256;

    int idx = (rBin * bins + gBin) * bins + bBin;
    hist[idx]++;
  }

  float total = static_cast<float>(img.width * img.height);
  vector<float> norm(hist.size());
  for (size_t i = 0; i < hist.size(); i++)
    norm[i] = hist[i] / total;

  return norm;
}

bool loadDescribeAndHistogram(const string &path, ImageRGB8 &outImg, vector<float> &outHist)
{
  if (!loadPPM_P6(path, outImg))
  {
    cerr << "Falha ao carregar PPM: " << path << "\n";
    return false;
  }
  cout << "\nCarregado PPM P6 " << outImg.width << "x" << outImg.height
       << " (" << outImg.data.size() << " bytes)\n";

  if (!outImg.data.empty())
  {
    unsigned char R0 = outImg.data[0], G0 = outImg.data[1], B0 = outImg.data[2];
    cout << "Pixel(0,0) R=" << (int)R0
         << " G=" << (int)G0
         << " B=" << (int)B0 << "\n";
  }

  outHist = computeRGBHistogram(outImg);
  cout << "Histograma calculado! Tamanho = " << outHist.size() << "\n";
  return true;
}

// Distância qui-quadrado
float chiSquare(vector<float> &referenceHistogram, vector<float> &queryHistogram)
{
  float sum = 0.0f;
  size_t binCount = referenceHistogram.size();

  for (size_t i = 0; i < binCount; i++)
  {
    float denominator = referenceHistogram[i] + queryHistogram[i];
    if (denominator != 0.0f)
    {
      float difference = referenceHistogram[i] - queryHistogram[i];
      sum += (difference * difference) / denominator;
    }
  }
  return sum;
}

// Busca linear
ListSearchResult searchMostSimilar(vector<ImageItem> &index, ImageItem &queryImage)
{
  string bestId = "";
  float bestDistance = numeric_limits<float>::infinity();

  for (size_t i = 0; i < index.size(); i++)
  {
    float d = chiSquare(index[i].histogram, queryImage.histogram);
    if (d < bestDistance)
    {
      bestId = index[i].id;
      bestDistance = d;
    }
  }
  return ListSearchResult(bestId, bestDistance);
}

// MAIN
int main()
{
    // Caminhos das imagens (agora em bulk)
    int startIdx = 1;
    int endIdx = 100;

    // Carrega imagens e histogramas
    vector<ImageItem> allImages;

    for (int i = startIdx; i <= endIdx; i++)
    {
        string path = "images/img" + to_string(i) + ".ppm";
        ImageRGB8 img;
        vector<float> hist;

        if (!loadDescribeAndHistogram(path, img, hist))
        {
            cerr << "Erro ao carregar imagem " << path << ". Encerrando.\n";
            return 1;
        }

        string id = "imagem_" + to_string(i);
        allImages.push_back(ImageItem(id, hist));
    }

    if (allImages.size() < 2)
    {
        cerr << "Eh necessario pelo menos 2 imagens (1 base + 1 consulta).\n";
        return 1;
    }

    // última imagem do intervalo será a imagem de consulta
    ImageItem imageQueryItem = allImages[allImages.size() - 1];

    // base = todas menos a última
    vector<ImageItem> imagesList;
    for (int i = 0; i < (int)allImages.size() - 1; i++)
    {
        imagesList.push_back(allImages[i]);
    }

    // BUSCAS NORMAIS (SEM TEMPO)
    cout << "\n\n== BUSCA EM LISTA ==\n";
    ListSearchResult listRes0 = searchMostSimilar(imagesList, imageQueryItem);
    listRes0.print();

    vector<ImageItem> imagesHashBase;
    for (int i = 0; i < (int)imagesList.size(); i++)
        imagesHashBase.push_back(imagesList[i]);

    HashSearchResult hashRes0 = searchMostSimilarHash(imagesHashBase, imageQueryItem, 3);
    hashRes0.print();

    cout << "\n\n== BUSCA POR QUADTREE ==\n";
    vector<ImageItem> imagesQTBase;
    for (int i = 0; i < (int)imagesList.size(); i++)
        imagesQTBase.push_back(imagesList[i]);

    QuadtreeSearchResult qtRes0 = searchMostSimilarQuadtree(imagesQTBase, imageQueryItem);
    qtRes0.print();

    cout << "\n\n== BUSCA EM M-TREE ==\n";
    MTree tree0;
    for (int i = 0; i < (int)imagesList.size(); i++)
        tree0.insert(imagesList[i]);

    MTreeSearchResult mtreeRes0 = tree0.searchMostSimilar(imageQueryItem);
    mtreeRes0.print();

    // =======================================================
    // =============    TESTE DE TEMPO   ======================
    // =======================================================
    cout << "\n\n===== TESTE DE TEMPO =====\n";

    // LISTA -----------------
    auto t1 = Clock::now();
    vector<ImageItem> listBase;
    for (int i = 0; i < (int)imagesList.size(); i++)
        listBase.push_back(imagesList[i]);
    auto t2 = Clock::now();

    auto b1 = Clock::now();
    ListSearchResult listRes = searchMostSimilar(listBase, imageQueryItem);
    auto b2 = Clock::now();

    double listBuild = ms(t1, t2);
    double listSearch = ms(b1, b2);

    // HASH -----------------
    auto h1 = Clock::now();
    vector<ImageItem> hashBase;
    for (int i = 0; i < (int)imagesList.size(); i++)
        hashBase.push_back(imagesList[i]);
    auto h2 = Clock::now();

    auto hb1 = Clock::now();
    HashSearchResult hashRes2 = searchMostSimilarHash(hashBase, imageQueryItem, 1);
    auto hb2 = Clock::now();

    double hashBuild = ms(h1, h2);
    double hashSearch = ms(hb1, hb2);

    // QUADTREE -----------------
    auto q1 = Clock::now();
    vector<ImageItem> qtBase;
    for (int i = 0; i < (int)imagesList.size(); i++)
        qtBase.push_back(imagesList[i]);

    QuadtreeSearchResult qtRes2 = searchMostSimilarQuadtree(qtBase, imageQueryItem);
    auto q2 = Clock::now();

    double qtBuild_Total = ms(q1, q2);
    double qtSearch = qtBuild_Total; // busca integrada

    // M-TREE -----------------
    auto m1 = Clock::now();
    MTree mtree;
    for (int i = 0; i < (int)imagesList.size(); i++)
        mtree.insert(imagesList[i]);
    auto m2 = Clock::now();

    auto mb1 = Clock::now();
    MTreeSearchResult mtreeRes2 = mtree.searchMostSimilar(imageQueryItem);
    auto mb2 = Clock::now();

    double mtBuild = ms(m1, m2);
    double mtSearch = ms(mb1, mb2);

    // RESULTADOS ----------------------
    cout << "\n===== TEMPOS (ms) =====\n";
    cout << "Lista:    build=" << listBuild << " | busca=" << listSearch << "\n";
    cout << "Hash:     build=" << hashBuild << " | busca=" << hashSearch << "\n";
    cout << "Quadtree: build+search=" << qtBuild_Total << " (Não Separado)\n";
    cout << "M-Tree:   build=" << mtBuild << " | busca=" << mtSearch << "\n\n";

    return 0;
}
