#pragma once
#include <string>
#include <vector>
#include "Entities.h"

class Database;

// Formato .collection: JSON (manifest + data) + imagens embutidas em um container próprio
// Layout: "CLCT" (4 bytes) + versão u32 + tamanho JSON u64 + JSON + imagens brutas sequenciais
// O data.json referencia imagens por índice/offset.

struct ExportResult {
    bool ok = false;
    std::string error;
};
struct ImportSummary {
    int models = 0, folders = 0, items = 0, images = 0;
    std::string json; // dados lidos para pré-visualização/confirmação
    bool ok = false;
    std::string error;
};

class ImportExport {
public:
    explicit ImportExport(Database& db) : m_db(db) {}

    ExportResult exportAll(const std::string& path);
    ExportResult exportFolder(int64_t folderId, const std::string& path);
    ExportResult exportItems(const std::vector<int64_t>& itemIds, const std::string& path);
    ExportResult exportModels(const std::vector<int64_t>& modelIds, const std::string& path);

    // Valida e lê o arquivo sem gravar nada (Passos 48, 49, 52)
    ImportSummary inspect(const std::string& path);

    // Confirmação (Passo 50): conflitos podem ser "use" (usar existente), "copy" ou "replace"
    enum class Conflict { Use, Copy, Replace };
    struct ConflictChoice {
        std::string modelName;
        Conflict choice = Conflict::Copy;
    };

    // Executa a importação inteira dentro de uma transação (Passo 53)
    bool importFile(const std::string& path, const std::vector<ConflictChoice>& choices, std::string& error);

private:
    ExportResult exportData(const std::string& json, const std::vector<ItemImage>& images, const std::string& path);
    Database& m_db;
};
