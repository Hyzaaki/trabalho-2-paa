#include "ppm_loader.hpp"
#include "search_list.hpp"
#include "search_hash.hpp"
#include "search_quadtree.hpp"
#include "search_mtree.hpp"
#include <iostream>
#include <vector>
#include <limits>
#include <cmath>
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
  // Caminhos das imagens
  string path1 = "images/img1.ppm";
  string path2 = "images/img2.ppm";
  string path3 = "images/img3.ppm";
  string path4 = "images/img4.ppm";

  // Carrega imagens e histogramas
  ImageRGB8 img1, img2, img3, img4;
  vector<float> hist1, hist2, hist3, hist4;

  if (!loadDescribeAndHistogram(path1, img1, hist1))
    return 1;
  if (!loadDescribeAndHistogram(path2, img2, hist2))
    return 1;
  if (!loadDescribeAndHistogram(path3, img3, hist3))
    return 1;
  if (!loadDescribeAndHistogram(path4, img4, hist4))
    return 1;

  ImageItem imageItem1("imagem_1", hist1);
  ImageItem imageItem2("imagem_2", hist2);
  ImageItem imageItem3("imagem_3", hist3);
  ImageItem imageQueryItem("imagem_4", hist4);

  // BUSCAS NORMAIS (SEM TEMPO)
  vector<ImageItem> imagesList = {imageItem1, imageItem2, imageItem3};

  cout << "\n\n== BUSCA EM LISTA ==\n";
  ListSearchResult listRes0 = searchMostSimilar(imagesList, imageQueryItem);
  listRes0.print();

  vector<ImageItem> imagesHashBase = {imageItem1, imageItem2, imageItem3};
  HashSearchResult hashRes0 = searchMostSimilarHash(imagesHashBase, imageQueryItem, 3);
  hashRes0.print();

  cout << "\n\n== BUSCA POR QUADTREE ==\n";
  vector<ImageItem> imagesQTBase = {imageItem1, imageItem2, imageItem3};
  QuadtreeSearchResult qtRes0 = searchMostSimilarQuadtree(imagesQTBase, imageQueryItem);
  qtRes0.print();

  cout << "\n\n== BUSCA EM M-TREE ==\n";
  MTree tree0;
  tree0.insert(imageItem1);
  tree0.insert(imageItem2);
  tree0.insert(imageItem3);
  MTreeSearchResult mtreeRes0 = tree0.searchMostSimilar(imageQueryItem);
  mtreeRes0.print();

  // =======================================================
  // =============    TESTE DE TEMPO   ======================
  // =======================================================
  cout << "\n\n===== TESTE DE TEMPO =====\n";

  // LISTA -----------------
  auto t1 = Clock::now();
  vector<ImageItem> listBase = {imageItem1, imageItem2, imageItem3};
  auto t2 = Clock::now();

  auto b1 = Clock::now();
  ListSearchResult listRes = searchMostSimilar(listBase, imageQueryItem);
  auto b2 = Clock::now();

  double listBuild = ms(t1, t2);
  double listSearch = ms(b1, b2);

  // HASH -----------------
  auto h1 = Clock::now();
  vector<ImageItem> hashBase = {imageItem1, imageItem2, imageItem3};
  auto h2 = Clock::now();

  auto hb1 = Clock::now();
  HashSearchResult hashRes2 = searchMostSimilarHash(hashBase, imageQueryItem, 1);
  auto hb2 = Clock::now();

  double hashBuild = ms(h1, h2);
  double hashSearch = ms(hb1, hb2);

  // QUADTREE -----------------
  auto q1 = Clock::now();
  vector<ImageItem> qtBase = {imageItem1, imageItem2, imageItem3};
  QuadtreeSearchResult qtRes2 = searchMostSimilarQuadtree(qtBase, imageQueryItem);
  auto q2 = Clock::now();

  double qtBuild_Total = ms(q1, q2);
  double qtSearch = qtBuild_Total; // busca integrada

  // M-TREE -----------------
  auto m1 = Clock::now();
  MTree mtree;
  mtree.insert(imageItem1);
  mtree.insert(imageItem2);
  mtree.insert(imageItem3);
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
