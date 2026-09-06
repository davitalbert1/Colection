#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "sqlite3.h"

class Database {
public:
    ~Database();

    // Abre (ou cria) o banco em `path`, aplica migrações. Retorna false com lastError preenchido.
    bool open(const std::string& path);
    void close();

    // Executa SQL simples (DDL etc). Retorna false em erro.
    bool exec(const std::string& sql);

    // Transações (Passo 42)
    bool begin();
    bool commit();
    bool rollback();

    // Executa fn() dentro de uma transação; rollback em exceção/falso.
    bool inTransaction(const std::function<bool()>& fn);

    int64_t lastInsertId() const;
    const char* lastError() const {
        return m_db ? sqlite3_errmsg(m_db) : "banco não aberto";
    }

    sqlite3* handle() const {
        return m_db;
    }

    // Migração (Passo 8)
    int schemaVersion();

private:
    bool migrate();
    sqlite3* m_db = nullptr;
};
