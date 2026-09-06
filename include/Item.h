#pragma once
#include <string>
#include <vector>
#include <map>
#include "Entities.h"

class Database;

class ItemRepository {
public:
    explicit ItemRepository(Database& db) : m_db(db) {}

    // Lista resumida; se folderId tiver valor, filtra pela pasta; se nullopt, tudo.
    std::vector<ItemSummary> list(const std::optional<int64_t>& folderId);
    std::vector<ItemSummary> search(const std::string& query);
    bool get(int64_t id, Item& out);
    bool create(int64_t modelId, const std::optional<int64_t>& folderId, int64_t& outId);
    bool update(const Item& it);
    bool remove(int64_t id, int& imagesRemoved); // transação: valores + imagens + item
    int64_t countAll();
    int imageCount(int64_t itemId);

    // Valores
    std::map<int64_t, ItemValue> getValues(int64_t itemId);
    bool setValue(int64_t itemId, const ItemValue& v);
    bool removeValue(int64_t itemId, int64_t fieldId);

private:
    Database& m_db;
};

class FolderRepository {
public:
    explicit FolderRepository(Database& db) : m_db(db) {}

    std::vector<Folder> getAll();
    bool get(int64_t id, Folder& out);
    bool create(const std::string& title, int64_t& outId);
    bool rename(int64_t id, const std::string& title);
    bool remove(int64_t id, bool deleteItems, int& itemsAffected);
    int64_t itemCount(int64_t folderId);

private:
    Database& m_db;
};
