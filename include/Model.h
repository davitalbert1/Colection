#pragma once
#include <string>
#include <vector>
#include "Entities.h"

class Database;

class ModelRepository {
public:
    explicit ModelRepository(Database& db) : m_db(db) {}

    std::vector<Model> getAll();
    bool get(int64_t id, Model& out);
    bool create(const std::string& name, const std::string& description, int64_t& outId);
    bool update(const Model& m);
    bool remove(int64_t id);
    int64_t itemCount(int64_t modelId);

    std::vector<ModelField> getFields(int64_t modelId);
    bool createField(int64_t modelId, const std::string& name, FieldType type, int position, int64_t& outId);
    bool updateField(const ModelField& f);
    bool removeField(int64_t fieldId);
    int64_t fieldUseCount(int64_t fieldId); // itens que possuem valor para o campo
    bool reorderFields(int64_t modelId, const std::vector<int64_t>& orderedIds);

private:
    Database& m_db;
};
