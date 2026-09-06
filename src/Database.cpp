#include "Database.h"
#include <filesystem>

Database::~Database() {
    close();
}

bool Database::open(const std::string& path) {
    close();
    std::filesystem::path p(path);
    if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());
    if (sqlite3_open_v2(path.c_str(), &m_db,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK)
        return false;
    sqlite3_busy_timeout(m_db, 5000);
    exec("PRAGMA foreign_keys = ON;");
    if (!migrate()) return false;
    return true;
}

void Database::close() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool Database::exec(const std::string& sql) {
    char* err = nullptr;
    bool ok = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &err) == SQLITE_OK;
    sqlite3_free(err);
    return ok;
}

bool Database::begin() {
    return exec("BEGIN TRANSACTION;");
}
bool Database::commit() {
    return exec("COMMIT;");
}
bool Database::rollback() {
    return exec("ROLLBACK;");
}

bool Database::inTransaction(const std::function<bool()>& fn) {
    if (!begin()) return false;
    bool ok = false;
    try {
        ok = fn();
    } catch (...) {
        rollback();
        throw;
    }
    if (!ok) {
        rollback();
        return false;
    }
    return commit();
}

int64_t Database::lastInsertId() const {
    return sqlite3_last_insert_rowid(m_db);
}

int Database::schemaVersion() {
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(m_db, "PRAGMA user_version;", -1, &st, nullptr) != SQLITE_OK) return -1;
    int v = sqlite3_step(st) == SQLITE_ROW ? sqlite3_column_int(st, 0) : -1;
    sqlite3_finalize(st);
    return v;
}

bool Database::migrate() {
    // Passo 8 — versionamento do banco (PRAGMA user_version)
    int v = schemaVersion();
    if (v > 1) return false; // banco de versão futura: não tocar

    const char* schemaV1 = R"(
        CREATE TABLE IF NOT EXISTS models (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            description TEXT DEFAULT '',
            created_at TEXT DEFAULT (datetime('now','localtime')),
            updated_at TEXT DEFAULT (datetime('now','localtime'))
        );
        CREATE TABLE IF NOT EXISTS model_fields (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            model_id INTEGER NOT NULL REFERENCES models(id) ON DELETE CASCADE,
            name TEXT NOT NULL,
            type INTEGER NOT NULL DEFAULT 0,
            position INTEGER NOT NULL DEFAULT 0,
            created_at TEXT DEFAULT (datetime('now','localtime')),
            updated_at TEXT DEFAULT (datetime('now','localtime'))
        );
        CREATE TABLE IF NOT EXISTS folders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT DEFAULT '',
            created_at TEXT DEFAULT (datetime('now','localtime')),
            updated_at TEXT DEFAULT (datetime('now','localtime'))
        );
        CREATE TABLE IF NOT EXISTS items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            model_id INTEGER NOT NULL REFERENCES models(id) ON DELETE CASCADE,
            folder_id INTEGER REFERENCES folders(id) ON DELETE SET NULL,
            created_at TEXT DEFAULT (datetime('now','localtime')),
            updated_at TEXT DEFAULT (datetime('now','localtime'))
        );
        CREATE INDEX IF NOT EXISTS idx_items_folder ON items(folder_id);
        CREATE TABLE IF NOT EXISTS item_values (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            item_id INTEGER NOT NULL REFERENCES items(id) ON DELETE CASCADE,
            field_id INTEGER NOT NULL REFERENCES model_fields(id) ON DELETE CASCADE,
            value_text TEXT, value_integer INTEGER, value_real REAL,
            value_boolean INTEGER, value_date TEXT,
            UNIQUE(item_id, field_id)
        );
        CREATE TABLE IF NOT EXISTS images (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            item_id INTEGER NOT NULL REFERENCES items(id) ON DELETE CASCADE,
            position INTEGER NOT NULL DEFAULT 0,
            filename TEXT DEFAULT '',
            mime_type TEXT DEFAULT '',
            data BLOB
        );
        CREATE INDEX IF NOT EXISTS idx_images_item ON images(item_id, position);
    )";
    if (!exec(schemaV1)) return false;
    return exec("PRAGMA user_version = 1;");
}
