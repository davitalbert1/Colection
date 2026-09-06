#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include "Entities.h"
#include "imgui.h"

class Database;

using TextureHandle = ImTextureID; // GLuint convertido

class ImageManager {
public:
    explicit ImageManager(Database& db) : m_db(db) {}

    // Banco (BLOBs)
    std::vector<ItemImage> getImages(int64_t itemId, bool withData = false);
    bool getImageData(int64_t imageId, ItemImage& out);
    bool addImages(int64_t itemId, const std::vector<ItemImage>& images, const std::vector<int64_t>& outIds);
    bool removeImage(int64_t imageId);
    bool reorder(int64_t itemId, const std::vector<int64_t>& orderedImageIds);

    // Cache de texturas (Passos 32, 61) — carrega sob demanda, decodifica com stb_image
    // Retorna 0 se não houver textura. Tenta carregar do banco na 1ª chamada.
    TextureHandle texture(int64_t imageId, int maxDim = 0 /*0 = tamanho original*/);
    void unload(int64_t imageId);
    void clearCache();
    size_t cacheSize() const {
        return m_cache.size();
    }

private:
    struct Tex {
        unsigned int id = 0;
        int w = 0,h = 0;
    };
    Database& m_db;
    std::map<int64_t, Tex> m_cache;
};
