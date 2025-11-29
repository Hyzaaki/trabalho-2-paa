// search_hash.hpp  — busca por hash via SimHash 128 bits para ImageItem (id + histogram)
#pragma once
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <cstdint>
#include <iostream>

#include "image_item.hpp"   // Usa a ImageItem do projeto (id, histogram)

/* -----------------------------------------------------------------------------
   O que faz:
     - Constrói um SimHash de 128 bits a partir de ImageItem::histogram (std::vector<float>).
     - Compara hashes via distância de Hamming.
     - Retorna top-K itens mais similares.

   API:
     HashSearchResult searchMostSimilarHash(const std::vector<ImageItem>& base,
                                            const ImageItem& query,
                                            int topK = 3);

   Observação:
     - Este header pressupõe que ImageItem possui:
         std::string id;
         std::vector<float> histogram;
-----------------------------------------------------------------------------*/

// ===== utilidades para SimHash deterministicamente =====
static inline uint64_t sh_splitmix64(uint64_t x) {
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}
static inline float sh_rand_sign_for_dim_bit(uint64_t dim, uint64_t bit) {
    uint64_t h = sh_splitmix64(dim * 0x9E3779B97F4A7C15ULL ^ (bit + 0xBF58476D1CE4E5B9ULL));
    return ((h >> 63) ? 1.0f : -1.0f);
}

struct Hash128 { uint64_t hi{0}, lo{0}; };

static inline int sh_hamming128(const Hash128& a, const Hash128& b) {
#if defined(_MSC_VER)
    auto popcnt64 = [](uint64_t x){ int c=0; while(x){ x&=(x-1); ++c; } return c; };
    return popcnt64(a.hi ^ b.hi) + popcnt64(a.lo ^ b.lo);
#else
    return (int)__builtin_popcountll(a.hi ^ b.hi) + (int)__builtin_popcountll(a.lo ^ b.lo);
#endif
}

static inline Hash128 sh_simhash128_from_hist(const std::vector<float>& hist) {
    // 128 acumuladores (um “hiperplano” por bit)
    double acc[128] = {0.0};
    const size_t D = hist.size();
    for (size_t d = 0; d < D; ++d) {
        const double w = (double)hist[d];
        if (w == 0.0) continue;
        for (int b = 0; b < 128; ++b) {
            acc[b] += (double)sh_rand_sign_for_dim_bit((uint64_t)d, (uint64_t)b) * w;
        }
    }
    Hash128 out;
    for (int b = 0; b < 128; ++b) {
        const bool bit = (acc[b] >= 0.0);
        if (b < 64) out.hi = (out.hi << 1) | (bit ? 1ULL : 0ULL);
        else        out.lo = (out.lo << 1) | (bit ? 1ULL : 0ULL);
    }
    return out;
}

// ===== Resultado e busca =====
struct HashSearchResult {
    // pares (id, distancia_hamming)
    std::vector<std::pair<std::string,int>> top;

    void print() const {
        std::cout << "\n\n== BUSCA POR HASH (SimHash 128b) ==\n";
        if (top.empty()) { std::cout << "Nenhum item encontrado.\n"; return; }
        for (size_t i = 0; i < top.size(); ++i) {
            std::cout << i+1 << ") " << top[i].first
                      << "  (hamming=" << top[i].second << ")\n";
        }
    }
};

static inline HashSearchResult searchMostSimilarHash(const std::vector<ImageItem>& base,
                                                     const ImageItem& query,
                                                     int topK = 3)
{
    HashSearchResult result;
    if (base.empty() || query.histogram.empty()) return result;

    const Hash128 qh = sh_simhash128_from_hist(query.histogram);

    std::vector<std::pair<std::string,int>> all;
    all.reserve(base.size());

    for (const auto& it : base) {
        if (it.histogram.empty()) continue;
        const Hash128 hh = sh_simhash128_from_hist(it.histogram);
        int dist = sh_hamming128(qh, hh);
        all.emplace_back(it.id, dist);
    }

    std::sort(all.begin(), all.end(),
              [](const auto& a, const auto& b){ return a.second < b.second; });

    if ((int)all.size() > topK) all.resize(topK);
    result.top = std::move(all);
    return result;
}
