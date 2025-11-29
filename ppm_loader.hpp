#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

struct ImageRGB8 {
    int width = 0, height = 0;
    std::vector<unsigned char> data; // tamanho = width*height*3 (RGB)
};

// Ignora as linhas de comentário
inline void skip_comments(std::istream& in) {
    while (in >> std::ws && in.peek() == '#') {
        std::string dummy;
        std::getline(in, dummy);
    }
}

// Carrega a imagem .ppm
inline bool loadPPM_P6(const std::string& path, ImageRGB8& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;

    std::string magic; f >> magic;
    if (magic != "P6") {
        std::cerr << "Apenas PPM P6 suportado.\n";
        return false;
    }

    skip_comments(f);
    int w, h, maxv;
    f >> w; skip_comments(f);
    f >> h; skip_comments(f);
    f >> maxv;

    if (!f.good() || w <= 0 || h <= 0) return false;
    if (maxv != 255) {
        std::cerr << "Apenas maxval=255 suportado.\n";
        return false;
    }

    // Consome um único whitespace após o cabeçalho
    f.get();

    out.width = w; out.height = h;
    out.data.resize(static_cast<size_t>(w) * h * 3);
    f.read(reinterpret_cast<char*>(out.data.data()), out.data.size());
    return f.good();
}
