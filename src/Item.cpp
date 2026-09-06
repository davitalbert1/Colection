#include "Item.h"
#include "Database.h"
#include <functional>

// fieldTypeName(FieldType) tem definição única em Model.cpp (declaração em Entities.h)

namespace {
    bool queryAll(Database& db, const char* sql,
                const std::function<void(sqlite3_stmt*)>& bind,
                const std::function<void(sqlite3_stmt*)>& row) {
        sqlite3_stmt* st = nullptr;
        if (sqlite3_prepare_v2(db.handle(), sql, -1, &st, nullptr) != SQLITE_OK) return false;
        if (bind) bind(st);
        while (sqlite3_step(st) == SQLITE_ROW) row(st);
        sqlite3_finalize(st);
        return true;
    }
    std::string colText(sqlite3_stmt* st, int c) {
        auto* t = sqlite3_column_text(st, c);
        return t ? reinterpret_cast<const char*>(t) : "";
    }
}

std::vector<ItemSummary> ItemRepository::list(const std::optional<int64_t>& folderId) {
    std::vector<ItemSummary> out;
    const char* sql =
        "SELECT i.id, i.model_id, i.folder_id, i.created_at, i.updated_at, m.name,"
        " COALESCE((SELECT v.value_text FROM item_values v JOIN model_fields f ON f.id=v.field_id"
        "   WHERE v.item_id=i.id AND f.type=0 AND (f.name='Título' OR f.name='Titulo' OR f.name='Nome' OR f.name='Nome do item')"
        "   ORDER BY f.position LIMIT 1),''),"
        " (SELECT COUNT(*) FROM images im WHERE im.item_id=i.id)"
        " FROM items i JOIN models m ON m.id=i.model_id";
    std::string q = sql;
    if (folderId.has_value()) q += " WHERE i.folder_id = ?";
    else q += " WHERE i.folder_id IS NULL";
    q += " ORDER BY i.id DESC;";
    queryAll(m_db, q.c_str(), [&](sqlite3_stmt* st){
        if (folderId.has_value()) sqlite3_bind_int64(st,1,*folderId);
    }, [&](sqlite3_stmt* st){
        ItemSummary s;
        s.item.id = sqlite3_column_int64(st,0);
        s.item.modelId = sqlite3_column_int64(st,1);
        if (sqlite3_column_type(st,2) != SQLITE_NULL)
            s.item.folderId = sqlite3_column_int64(st,2);
        s.item.createdAt = colText(st,3);
        s.item.updatedAt = colText(st,4);
        s.modelName = colText(st,5);
        s.title = colText(st,6);
        s.imageCount = sqlite3_column_int(st,7);
        s.hasImage = s.imageCount > 0;
        out.push_back(std::move(s));
    });
    return out;
}

std::vector<ItemSummary> ItemRepository::search(const std::string& query) {
    // Passo 40 — pesquisa por título, campos de texto, modelo e pasta
    std::vector<ItemSummary> out;
    const char* sql =
        "SELECT DISTINCT i.id, i.model_id, i.folder_id, i.created_at, i.updated_at, m.name,"
        " COALESCE((SELECT v.value_text FROM item_values v JOIN model_fields f ON f.id=v.field_id"
        "   WHERE v.item_id=i.id AND f.type=0 AND (f.name='Título' OR f.name='Titulo' OR f.name='Nome' OR f.name='Nome do item')"
        "   ORDER BY f.position LIMIT 1),''),"
        " (SELECT COUNT(*) FROM images im WHERE im.item_id=i.id)"
        " FROM items i JOIN models m ON m.id=i.model_id"
        " LEFT JOIN folders fo ON fo.id=i.folder_id"
        " WHERE i.id IN (SELECT item_id FROM item_values WHERE value_text LIKE ?)"
        "    OR m.name LIKE ? OR fo.title LIKE ?"
        "    OR i.id IN (SELECT v.item_id FROM item_values v JOIN model_fields f ON f.id=v.field_id"
        "                WHERE f.type=0 AND (f.name='Título' OR f.name='Titulo' OR f.name='Nome') AND v.value_text LIKE ?)"
        " ORDER BY i.id DESC LIMIT 500;";
    std::string like = "%" + query + "%";
    sqlite3_stmt* st0 = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(), sql, -1, &st0, nullptr) != SQLITE_OK) return out;
    sqlite3_finalize(st0);
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(), sql, -1, &st, nullptr) != SQLITE_OK) return out;
    for (int c = 1; c <= 4; ++c) sqlite3_bind_text(st, c, like.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(st) == SQLITE_ROW) {
        ItemSummary s;
        s.item.id = sqlite3_column_int64(st,0);
        s.item.modelId = sqlite3_column_int64(st,1);
        if (sqlite3_column_type(st,2) != SQLITE_NULL) s.item.folderId = sqlite3_column_int64(st,2);
        s.item.createdAt = colText(st,3);
        s.item.updatedAt = colText(st,4);
        s.modelName = colText(st,5);
        s.title = colText(st,6);
        s.imageCount = sqlite3_column_int(st,7);
        s.hasImage = s.imageCount > 0;
        out.push_back(std::move(s));
    }
    sqlite3_finalize(st);
    return out;
}

bool ItemRepository::get(int64_t id, Item& out) {
    bool found = false;
    queryAll(m_db, "SELECT id, model_id, folder_id, created_at, updated_at FROM items WHERE id=?;",
        [&](sqlite3_stmt* st){
            sqlite3_bind_int64(st,1,id);
        },
        [&](sqlite3_stmt* st){
            found = true;
            out.id = sqlite3_column_int64(st,0);
            out.modelId = sqlite3_column_int64(st,1);
            if (sqlite3_column_type(st,2) != SQLITE_NULL) out.folderId = sqlite3_column_int64(st,2);
            out.createdAt = colText(st,3);
            out.updatedAt = colText(st,4);
        });
    return found;
}

bool ItemRepository::create(int64_t modelId, const std::optional<int64_t>& folderId, int64_t& outId) {
    return m_db.inTransaction([&]{
        sqlite3_stmt* st = nullptr;
        const char* sql = folderId.has_value()
            ? "INSERT INTO items(model_id, folder_id) VALUES(?,?);"
            : "INSERT INTO items(model_id) VALUES(?);";
        if (sqlite3_prepare_v2(m_db.handle(), sql, -1, &st, nullptr) != SQLITE_OK) return false;
        sqlite3_bind_int64(st,1,modelId);
        if (folderId.has_value()) sqlite3_bind_int64(st,2,*folderId);
        bool ok = sqlite3_step(st) == SQLITE_DONE;
        if (ok) outId = m_db.lastInsertId();
        sqlite3_finalize(st);
        return ok;
    });
}

bool ItemRepository::update(const Item& it) {
    return m_db.inTransaction([&]{
        sqlite3_stmt* st = nullptr;
        const char* sql = it.folderId.has_value()
            ? "UPDATE items SET folder_id=?, updated_at=datetime('now','localtime') WHERE id=?;"
            : "UPDATE items SET folder_id=NULL, updated_at=datetime('now','localtime') WHERE id=?;";
        if (sqlite3_prepare_v2(m_db.handle(), sql, -1, &st, nullptr) != SQLITE_OK) return false;
        if (it.folderId.has_value()) {
            sqlite3_bind_int64(st,1,*it.folderId);
            sqlite3_bind_int64(st,2,it.id);
        } else {
            sqlite3_bind_int64(st,1,it.id);
        }
        bool ok = sqlite3_step(st) == SQLITE_DONE;
        sqlite3_finalize(st);
        return ok;
    });
}

bool ItemRepository::remove(int64_t id, int& imagesRemoved) {
    // Passo 39 — transação: valores + imagens + item
    imagesRemoved = 0;
    return m_db.inTransaction([&]{
        sqlite3_stmt* st = nullptr;
        if (sqlite3_prepare_v2(m_db.handle(), "SELECT COUNT(*) FROM images WHERE item_id=?;", -1, &st, nullptr) != SQLITE_OK) return false;
        sqlite3_bind_int64(st,1,id);
        if (sqlite3_step(st) == SQLITE_ROW) imagesRemoved = sqlite3_column_int(st,0);
        sqlite3_finalize(st);
        if (sqlite3_prepare_v2(m_db.handle(), "DELETE FROM items WHERE id=?;", -1, &st, nullptr) != SQLITE_OK) return false;
        sqlite3_bind_int64(st,1,id);
        bool ok = sqlite3_step(st) == SQLITE_DONE; // CASCADE remove valores e imagens
        sqlite3_finalize(st);
        return ok;
    });
}

int64_t ItemRepository::countAll() {
    int64_t n = 0;
    queryAll(m_db, "SELECT COUNT(*) FROM items;", nullptr, [&](sqlite3_stmt* st){
        n = sqlite3_column_int64(st,0);
    });
    return n;
}

int ItemRepository::imageCount(int64_t itemId) {
    int n = 0;
    queryAll(m_db, "SELECT COUNT(*) FROM images WHERE item_id=?;",
        [&](sqlite3_stmt* st){
            sqlite3_bind_int64(st,1,itemId);
        },
        [&](sqlite3_stmt* st){
            n = sqlite3_column_int(st,0);
        });
    return n;
}

std::map<int64_t, ItemValue> ItemRepository::getValues(int64_t itemId) {
    std::map<int64_t, ItemValue> out;
    queryAll(m_db,
        "SELECT field_id, value_text, value_integer, value_real, value_boolean, value_date FROM item_values WHERE item_id=?;",
        [&](sqlite3_stmt* st){
            sqlite3_bind_int64(st,1,itemId);
        },
        [&](sqlite3_stmt* st){
            ItemValue v;
            v.fieldId = sqlite3_column_int64(st,0);
            v.text = colText(st,1);
            v.integer = sqlite3_column_int64(st,2);
            v.real = sqlite3_column_double(st,3);
            v.boolean = sqlite3_column_int(st,4) != 0;
            v.date = colText(st,5);
            out[v.fieldId] = v;
        });
    return out;
}

bool ItemRepository::setValue(int64_t itemId, const ItemValue& v) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(),
        "INSERT INTO item_values(item_id, field_id, value_text, value_integer, value_real, value_boolean, value_date)"
        " VALUES(?,?,?,?,?,?,?) ON CONFLICT(item_id, field_id) DO UPDATE SET"
        " value_text=excluded.value_text, value_integer=excluded.value_integer,"
        " value_real=excluded.value_real, value_boolean=excluded.value_boolean, value_date=excluded.value_date;",
        -1, &st, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int64(st,1,itemId);
    sqlite3_bind_int64(st,2,v.fieldId);
    sqlite3_bind_text(st,3,v.text.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int64(st,4,v.integer);
    sqlite3_bind_double(st,5,v.real);
    sqlite3_bind_int(st,6,v.boolean ? 1 : 0);
    sqlite3_bind_text(st,7,v.date.c_str(),-1,SQLITE_TRANSIENT);
    bool ok = sqlite3_step(st) == SQLITE_DONE;
    sqlite3_finalize(st);
    return ok;
}

bool ItemRepository::removeValue(int64_t itemId, int64_t fieldId) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(), "DELETE FROM item_values WHERE item_id=? AND field_id=?;",
        -1, &st, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int64(st,1,itemId);
    sqlite3_bind_int64(st,2,fieldId);
    bool ok = sqlite3_step(st) == SQLITE_DONE;
    sqlite3_finalize(st);
    return ok;
}

// Pastas
std::vector<Folder> FolderRepository::getAll() {
    std::vector<Folder> out;
    queryAll(m_db, "SELECT id, title, created_at, updated_at FROM folders ORDER BY title COLLATE NOCASE, id;",
        nullptr, [&](sqlite3_stmt* st){
            Folder f;
            f.id = sqlite3_column_int64(st,0);
            f.title = colText(st,1);
            f.createdAt = colText(st,2);
            f.updatedAt = colText(st,3);
            out.push_back(std::move(f));
        });
    return out;
}

bool FolderRepository::get(int64_t id, Folder& out) {
    bool found = false;
    queryAll(m_db, "SELECT id, title FROM folders WHERE id=?;",
        [&](sqlite3_stmt* st){
            sqlite3_bind_int64(st,1,id);
        },
        [&](sqlite3_stmt* st){
            found = true;
            out.id = sqlite3_column_int64(st,0);
            out.title = colText(st,1);
        });
    return found;
}

bool FolderRepository::create(const std::string& title, int64_t& outId) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(), "INSERT INTO folders(title) VALUES(?);", -1, &st, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(st,1,title.c_str(),-1,SQLITE_TRANSIENT);
    bool ok = sqlite3_step(st) == SQLITE_DONE;
    if (ok) outId = m_db.lastInsertId();
    sqlite3_finalize(st);
    return ok;
}

bool FolderRepository::rename(int64_t id, const std::string& title) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(),
        "UPDATE folders SET title=?, updated_at=datetime('now','localtime') WHERE id=?;", -1, &st, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(st,1,title.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int64(st,2,id);
    bool ok = sqlite3_step(st) == SQLITE_DONE;
    sqlite3_finalize(st);
    return ok;
}

bool FolderRepository::remove(int64_t id, bool deleteItems, int& itemsAffected) {
    return m_db.inTransaction([&]{
        itemsAffected = itemCount(id);
        if (deleteItems) {
            sqlite3_stmt* st = nullptr;
            if (sqlite3_prepare_v2(m_db.handle(), "DELETE FROM items WHERE folder_id=?;", -1, &st, nullptr) != SQLITE_OK) return false;
            sqlite3_bind_int64(st,1,id);
            bool ok = sqlite3_step(st) == SQLITE_DONE;
            sqlite3_finalize(st);
            if (!ok) return false;
        } else {
            sqlite3_stmt* st = nullptr;
            if (sqlite3_prepare_v2(m_db.handle(), "UPDATE items SET folder_id=NULL WHERE folder_id=?;", -1, &st, nullptr) != SQLITE_OK) return false;
            sqlite3_bind_int64(st,1,id);
            bool ok = sqlite3_step(st) == SQLITE_DONE;
            sqlite3_finalize(st);
            if (!ok) return false;
        }
        sqlite3_stmt* st = nullptr;
        if (sqlite3_prepare_v2(m_db.handle(), "DELETE FROM folders WHERE id=?;", -1, &st, nullptr) != SQLITE_OK) return false;
        sqlite3_bind_int64(st,1,id);
        bool ok = sqlite3_step(st) == SQLITE_DONE;
        sqlite3_finalize(st);
        return ok;
    });
}

int64_t FolderRepository::itemCount(int64_t folderId) {
    int64_t n = 0;
    queryAll(m_db, "SELECT COUNT(*) FROM items WHERE folder_id=?;",
        [&](sqlite3_stmt* st){
            sqlite3_bind_int64(st,1,folderId);
        },
        [&](sqlite3_stmt* st){
            n = sqlite3_column_int64(st,0);
        });
    return n;
}
