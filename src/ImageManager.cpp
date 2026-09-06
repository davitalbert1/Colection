// Imagens como BLOB + cache de texturas (Passos 27-34, 61)
#include "ImageManager.h"
#include "Database.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glad/glad.h>
#include <algorithm>

std::vector<ItemImage> ImageManager::getImages(int64_t itemId, bool withData) {
    std::vector<ItemImage> out;
    std::string sql = withData
        ? "SELECT id, item_id, position, filename, mime_type, data FROM images WHERE item_id=? ORDER BY position;"
        : "SELECT id, item_id, position, filename, mime_type, NULL FROM images WHERE item_id=? ORDER BY position;";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(), sql.c_str(), -1, &st, nullptr) != SQLITE_OK) return out;
    sqlite3_bind_int64(st,1,itemId);
    while (sqlite3_step(st) == SQLITE_ROW) {
        ItemImage im;
        im.id = sqlite3_column_int64(st,0);
        im.itemId = sqlite3_column_int64(st,1);
        im.position = sqlite3_column_int(st,2);
        auto* fn = sqlite3_column_text(st,3);
        im.filename = fn ? reinterpret_cast<const char*>(fn) : "";
        auto* mt = sqlite3_column_text(st,4);
        im.mimeType = mt ? reinterpret_cast<const char*>(mt) : "";
        if (withData) {
            const void* blob = sqlite3_column_blob(st,5);
            int n = sqlite3_column_bytes(st,5);
            if (blob && n > 0) im.data.assign((const uint8_t*)blob, (const uint8_t*)blob + n);
        }
        out.push_back(std::move(im));
    }
    sqlite3_finalize(st);
    return out;
}

bool ImageManager::getImageData(int64_t imageId, ItemImage& out) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(),
        "SELECT id, item_id, position, filename, mime_type, data FROM images WHERE id=?;",
        -1, &st, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int64(st,1,imageId);
    bool found = false;
    if (sqlite3_step(st) == SQLITE_ROW) {
        found = true;
        out.id = sqlite3_column_int64(st,0);
        out.itemId = sqlite3_column_int64(st,1);
        out.position = sqlite3_column_int(st,2);
        auto* fn = sqlite3_column_text(st,3);
        out.filename = fn ? reinterpret_cast<const char*>(fn) : "";
        auto* mt = sqlite3_column_text(st,4);
        out.mimeType = mt ? reinterpret_cast<const char*>(mt) : "";
        const void* blob = sqlite3_column_blob(st,5);
        int n = sqlite3_column_bytes(st,5);
        if (blob && n > 0) out.data.assign((const uint8_t*)blob, (const uint8_t*)blob + n);
    }
    sqlite3_finalize(st);
    return found;
}

bool ImageManager::addImages(int64_t itemId, const std::vector<ItemImage>& images, const std::vector<int64_t>& outIds) {
    return m_db.inTransaction([&]{
        int base = 0;
        {
            sqlite3_stmt* st = nullptr;
            if (sqlite3_prepare_v2(m_db.handle(), "SELECT COALESCE(MAX(position)+1,0) FROM images WHERE item_id=?;",
              -1, &st, nullptr) != SQLITE_OK) return false;
            sqlite3_bind_int64(st,1,itemId);
            if (sqlite3_step(st) == SQLITE_ROW) base = sqlite3_column_int(st,0);
            sqlite3_finalize(st);
        }
        for (size_t i = 0; i < images.size(); ++i) {
            sqlite3_stmt* st = nullptr;
            if (sqlite3_prepare_v2(m_db.handle(),
                "INSERT INTO images(item_id, position, filename, mime_type, data) VALUES(?,?,?,?,?);",
                -1, &st, nullptr) != SQLITE_OK) return false;
            sqlite3_bind_int64(st,1,itemId);
            sqlite3_bind_int(st,2,base + (int)i);
            sqlite3_bind_text(st,3,images[i].filename.c_str(),-1,SQLITE_TRANSIENT);
            sqlite3_bind_text(st,4,images[i].mimeType.c_str(),-1,SQLITE_TRANSIENT);
            sqlite3_bind_blob(st,5,images[i].data.data(),(int)images[i].data.size(),SQLITE_TRANSIENT);
            bool ok = sqlite3_step(st) == SQLITE_DONE;
            if (ok && outIds.size() > i) const_cast<std::vector<int64_t>&>(outIds)[i] = m_db.lastInsertId();
            sqlite3_finalize(st);
            if (!ok) return false;
        }
        return true;
    });
}

bool ImageManager::removeImage(int64_t imageId) {
    int64_t itemId = -1;
    int pos = 0;
    {
        sqlite3_stmt* st = nullptr;
      if (sqlite3_prepare_v2(m_db.handle(), "SELECT item_id, position FROM images WHERE id=?;", -1, &st, nullptr) != SQLITE_OK) return false;
      sqlite3_bind_int64(st,1,imageId);
      if (sqlite3_step(st) == SQLITE_ROW) {
        itemId = sqlite3_column_int64(st,0);
        pos = sqlite3_column_int(st,1);
    }
      sqlite3_finalize(st);
    }
    return m_db.inTransaction([&]{
        sqlite3_stmt* st = nullptr;
        if (sqlite3_prepare_v2(m_db.handle(), "DELETE FROM images WHERE id=?;", -1, &st, nullptr) != SQLITE_OK) return false;
        sqlite3_bind_int64(st,1,imageId);
        bool ok = sqlite3_step(st) == SQLITE_DONE;
        sqlite3_finalize(st);
        if (!ok) return false;
        // compacta posições
        if (sqlite3_prepare_v2(m_db.handle(),
            "UPDATE images SET position=position-1 WHERE item_id=? AND position>?;",
            -1, &st, nullptr) != SQLITE_OK) return false;
        sqlite3_bind_int64(st,1,itemId);
        sqlite3_bind_int(st,2,pos);
        ok = sqlite3_step(st) == SQLITE_DONE;
        sqlite3_finalize(st);
        unload(imageId);
        return ok;
    });
}

bool ImageManager::reorder(int64_t itemId, const std::vector<int64_t>& orderedImageIds) {
    return m_db.inTransaction([&]{
        for (size_t i = 0; i < orderedImageIds.size(); ++i) {
            sqlite3_stmt* st = nullptr;
            if (sqlite3_prepare_v2(m_db.handle(), "UPDATE images SET position=? WHERE id=? AND item_id=?;",
                -1, &st, nullptr) != SQLITE_OK) return false;
            sqlite3_bind_int(st,1,(int)i);
            sqlite3_bind_int64(st,2,orderedImageIds[i]);
            sqlite3_bind_int64(st,3,itemId);
            bool ok = sqlite3_step(st) == SQLITE_DONE;
            sqlite3_finalize(st);
            if (!ok) return false;
        }
        return true;
    });
}

TextureHandle ImageManager::texture(int64_t imageId, int maxDim) {
    auto key = imageId * 100000 + maxDim; // separa miniatura do original
    auto it = m_cache.find(key);
    if (it != m_cache.end()) return (TextureHandle)(intptr_t)it->second.id;

    ItemImage im;
    if (!getImageData(imageId, im) || im.data.empty()) return (TextureHandle)nullptr;
    int w=0,h=0,comp=0;
    unsigned char* px = stbi_load_from_memory(im.data.data(), (int)im.data.size(), &w, &h, &comp, 4);
    if (!px) return (TextureHandle)nullptr; // Passo 54: imagem inválida -> placeholder na UI

    // redimensionamento visual é feito via UV na UI (sem dependência extra)
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
    stbi_image_free(px);
    Tex t{tex, w, h};
    m_cache[key] = t;
    return (TextureHandle)(intptr_t)tex;
}

void ImageManager::unload(int64_t imageId) {
    for (auto it = m_cache.begin(); it != m_cache.end();) {
        if (it->first / 100000 == imageId) {
            glDeleteTextures(1, &it->second.id);
            it = m_cache.erase(it);
        } else ++it;
    }
}

void ImageManager::clearCache() {
    for (auto& [k, t] : m_cache) glDeleteTextures(1, &t.id);
    m_cache.clear();
}
