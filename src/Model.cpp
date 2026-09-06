#include "Model.h"
#include "Database.h"

const char* fieldTypeName(FieldType t) {
    switch (t) {
        case FieldType::Text: return "Texto";
        case FieldType::Integer: return "Número inteiro";
        case FieldType::Decimal: return "Número decimal";
        case FieldType::Boolean: return "Checkbox";
        case FieldType::Date: return "Data";
    }
    return "?";
}

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

std::vector<Model> ModelRepository::getAll() {
    std::vector<Model> out;
    queryAll(m_db, "SELECT id, name, description, created_at, updated_at FROM models ORDER BY name COLLATE NOCASE;",
        nullptr, [&](sqlite3_stmt* st) {
            Model m;
            m.id = sqlite3_column_int64(st,0);
            m.name = colText(st,1);
            m.description = colText(st,2);
            m.createdAt = colText(st,3);
            m.updatedAt = colText(st,4);
            out.push_back(std::move(m));
        });
    return out;
}

bool ModelRepository::get(int64_t id, Model& out) {
    bool found = false;
    queryAll(m_db, "SELECT id, name, description, created_at, updated_at FROM models WHERE id=?;",
        [&](sqlite3_stmt* st){
            sqlite3_bind_int64(st,1,id);
        },
        [&](sqlite3_stmt* st){
            found = true;
            out.id = sqlite3_column_int64(st,0);
            out.name = colText(st,1);
            out.description = colText(st,2);
            out.createdAt = colText(st,3);
            out.updatedAt = colText(st,4);
        });
    return found;
}

bool ModelRepository::create(const std::string& name, const std::string& description, int64_t& outId) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(),
            "INSERT INTO models(name, description) VALUES(?,?);", -1, &st, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(st,1,name.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(st,2,description.c_str(),-1,SQLITE_TRANSIENT);
    bool ok = sqlite3_step(st) == SQLITE_DONE;
    if (ok) outId = m_db.lastInsertId();
    sqlite3_finalize(st);
    return ok;
}

bool ModelRepository::update(const Model& m) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(),
            "UPDATE models SET name=?, description=?, updated_at=datetime('now','localtime') WHERE id=?;",
            -1, &st, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(st,1,m.name.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(st,2,m.description.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int64(st,3,m.id);
    bool ok = sqlite3_step(st) == SQLITE_DONE;
    sqlite3_finalize(st);
    return ok;
}

bool ModelRepository::remove(int64_t id) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(), "DELETE FROM models WHERE id=?;", -1, &st, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int64(st,1,id);
    bool ok = sqlite3_step(st) == SQLITE_DONE;
    sqlite3_finalize(st);
    return ok;
}

int64_t ModelRepository::itemCount(int64_t modelId) {
    int64_t n = 0;
    queryAll(m_db, "SELECT COUNT(*) FROM items WHERE model_id=?;",
        [&](sqlite3_stmt* st){
            sqlite3_bind_int64(st,1,modelId);
        },
        [&](sqlite3_stmt* st){
            n = sqlite3_column_int64(st,0);
        });
    return n;
}

std::vector<ModelField> ModelRepository::getFields(int64_t modelId) {
    std::vector<ModelField> out;
    queryAll(m_db, "SELECT id, model_id, name, type, position FROM model_fields WHERE model_id=? ORDER BY position;",
        [&](sqlite3_stmt* st){
            sqlite3_bind_int64(st,1,modelId);
        },
        [&](sqlite3_stmt* st){
            ModelField f;
            f.id = sqlite3_column_int64(st,0);
            f.modelId = sqlite3_column_int64(st,1);
            f.name = colText(st,2);
            f.type = static_cast<FieldType>(sqlite3_column_int(st,3));
            f.position = sqlite3_column_int(st,4);
            out.push_back(std::move(f));
        });
    return out;
}

bool ModelRepository::createField(int64_t modelId, const std::string& name, FieldType type, int position, int64_t& outId) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(),
            "INSERT INTO model_fields(model_id, name, type, position) VALUES(?,?,?,?);",
            -1, &st, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int64(st,1,modelId);
    sqlite3_bind_text(st,2,name.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(st,3,static_cast<int>(type));
    sqlite3_bind_int(st,4,position);
    bool ok = sqlite3_step(st) == SQLITE_DONE;
    if (ok) outId = m_db.lastInsertId();
    sqlite3_finalize(st);
    return ok;
}

bool ModelRepository::updateField(const ModelField& f) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(),
            "UPDATE model_fields SET name=?, type=?, position=?, updated_at=datetime('now','localtime') WHERE id=?;",
            -1, &st, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(st,1,f.name.c_str(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(st,2,static_cast<int>(f.type));
    sqlite3_bind_int(st,3,f.position);
    sqlite3_bind_int64(st,4,f.id);
    bool ok = sqlite3_step(st) == SQLITE_DONE;
    sqlite3_finalize(st);
    return ok;
}

bool ModelRepository::removeField(int64_t fieldId) {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db.handle(), "DELETE FROM model_fields WHERE id=?;", -1, &st, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int64(st,1,fieldId);
    bool ok = sqlite3_step(st) == SQLITE_DONE;
    sqlite3_finalize(st);
    return ok;
}

int64_t ModelRepository::fieldUseCount(int64_t fieldId) {
    int64_t n = 0;
    queryAll(m_db, "SELECT COUNT(*) FROM item_values WHERE field_id=?;",
        [&](sqlite3_stmt* st){
            sqlite3_bind_int64(st,1,fieldId);
        },
        [&](sqlite3_stmt* st){
            n = sqlite3_column_int64(st,0);
        });
    return n;
}

bool ModelRepository::reorderFields(int64_t modelId, const std::vector<int64_t>& orderedIds) {
    return m_db.inTransaction([&]{
        for (size_t i = 0; i < orderedIds.size(); ++i) {
            sqlite3_stmt* st = nullptr;
            if (sqlite3_prepare_v2(m_db.handle(),
                    "UPDATE model_fields SET position=?, updated_at=datetime('now','localtime') WHERE id=? AND model_id=?;",
                    -1, &st, nullptr) != SQLITE_OK) return false;
            sqlite3_bind_int(st,1,static_cast<int>(i));
            sqlite3_bind_int64(st,2,orderedIds[i]);
            sqlite3_bind_int64(st,3,modelId);
            bool ok = sqlite3_step(st) == SQLITE_DONE;
            sqlite3_finalize(st);
            if (!ok) return false;
        }
        return true;
    });
}
