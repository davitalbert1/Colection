// ExportaÃ§Ã£o e importaÃ§Ã£o (Passos 43-55)
#include "ImportExport.h"
#include "Database.h"
#include "Model.h"
#include "Item.h"
#include "ImageManager.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

static constexpr char MAGIC[5] = "CLCT";
static constexpr uint32_t FORMAT_VERSION = 1;

static void putU64(std::vector<uint8_t>& v, uint64_t x) {
    uint8_t b[8];
    memcpy(b, &x, 8);
    v.insert(v.end(), b, b + 8);
}
static uint64_t readU64(const uint8_t* p) {
    uint64_t x;
    memcpy(&x, p, 8);
    return x;
}

namespace {
    bool readFile(const std::string& path, std::vector<uint8_t>& out) {
        std::ifstream f(path, std::ios::binary);
        if (!f) return false;
        out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        return true;
    }
    bool writeFile(const std::string& path, const std::vector<uint8_t>& data) {
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        if (!f) return false;
        f.write((const char*)data.data(), (std::streamsize)data.size());
        return (bool)f;
    }
    FieldType toFT(int t) {
        return static_cast<FieldType>(t);
    }
}

// serializaÃ§Ã£o de dados em JSON
static json itemsToJson(Database& db, ImageManager& imgs, const std::vector<int64_t>& itemIds,
                        std::vector<ItemImage>& allImages) {
    ItemRepository items(db);
    ModelRepository models(db);
    json arr = json::array();
    for (int64_t id : itemIds) {
        Item it;
        if (!items.get(id, it)) continue;
        json j;
        j["id"] = it.id;
        j["model_id"] = it.modelId;
        if (it.folderId) j["folder_id"] = *it.folderId;
        else j["folder_id"] = nullptr;
        j["created_at"] = it.createdAt;
        j["updated_at"] = it.updatedAt;
        json vals = json::array();
        for (auto& [fid, v] : items.getValues(id)) {
            vals.push_back({{"field_id", fid}, {"text", v.text}, {"integer", v.integer},
                            {"real", v.real}, {"boolean", v.boolean}, {"date", v.date}});
        }
        j["values"] = vals;
        json imArr = json::array();
        for (auto& im : imgs.getImages(id, true)) {
            allImages.push_back(im);
            imArr.push_back({{"filename", im.filename}, {"mime_type", im.mimeType},
                             {"position", im.position}, {"image_index", (int)allImages.size() - 1}});
        }
        j["images"] = imArr;
        arr.push_back(j);
    }
    return arr;
}

static json buildData(Database& db, ImageManager& imgs,
                      const std::vector<int64_t>* modelFilter,
                      const std::vector<int64_t>* folderFilter,
                      const std::vector<int64_t>* itemFilter,
                      std::vector<ItemImage>& allImages) {
    ModelRepository models(db);
    FolderRepository folders(db);
    json data;
    data["format"] = "collection";
    data["version"] = 1;

    json mArr = json::array();
    std::vector<Model> modelList = models.getAll();
    for (auto& m : modelList) {
        if (modelFilter && std::find(modelFilter->begin(), modelFilter->end(), m.id) == modelFilter->end()) continue;
        json jm = {{"id", m.id}, {"name", m.name}, {"description", m.description}};
        json fArr = json::array();
        for (auto& f : models.getFields(m.id))
            fArr.push_back({{"id", f.id}, {"name", f.name}, {"type", (int)f.type}, {"position", f.position}});
        jm["fields"] = fArr;
        mArr.push_back(jm);
    }
    data["models"] = mArr;

    json foArr = json::array();
    for (auto& f : folders.getAll()) {
        if (folderFilter && std::find(folderFilter->begin(), folderFilter->end(), f.id) == folderFilter->end()) continue;
        foArr.push_back({{"id", f.id}, {"title", f.title}});
    }
    data["folders"] = foArr;

    std::vector<int64_t> ids;
    if (itemFilter) ids = *itemFilter;
    else {
        for (auto& s : ItemRepository(db).list(std::nullopt)) ids.push_back(s.item.id);
        // inclui itens em pastas
        for (auto& f : folders.getAll())
            for (auto& s : ItemRepository(db).list(f.id)) ids.push_back(s.item.id);
    }
    data["items"] = itemsToJson(db, imgs, ids, allImages);
    return data;
}

ExportResult ImportExport::exportData(const std::string& jsonStr, const std::vector<ItemImage>& images, const std::string& path) {
    std::vector<uint8_t> out;
    out.insert(out.end(), MAGIC, MAGIC + 4);
    uint32_t ver = FORMAT_VERSION;
    out.insert(out.end(), (uint8_t*)&ver, (uint8_t*)&ver + 4);
    putU64(out, (uint64_t)jsonStr.size());
    out.insert(out.end(), jsonStr.begin(), jsonStr.end());
    for (auto& im : images) {
        putU64(out, (uint64_t)im.data.size());
        out.insert(out.end(), im.data.begin(), im.data.end());
    }
    ExportResult r;
    r.ok = writeFile(path, out);
    if (!r.ok) r.error = "NÃ£o foi possÃ­vel gravar o arquivo: " + path;
    return r;
}

ExportResult ImportExport::exportAll(const std::string& path) {
    ImageManager imMgr(m_db);
    std::vector<ItemImage> imgs;
    json data = buildData(m_db, imMgr, nullptr, nullptr, nullptr, imgs);
    return exportData(data.dump(), imgs, path);
}

ExportResult ImportExport::exportFolder(int64_t folderId, const std::string& path) {
    ImageManager imMgr(m_db);
    std::vector<ItemImage> imgs;
    ItemRepository items(m_db);
    std::vector<int64_t> ids;
    for (auto& s : items.list(folderId)) ids.push_back(s.item.id);
    std::vector<int64_t> folderIds{folderId};
    json data = buildData(m_db, imMgr, nullptr, &folderIds, &ids, imgs);
    return exportData(data.dump(), imgs, path);
}

ExportResult ImportExport::exportItems(const std::vector<int64_t>& itemIds, const std::string& path) {
    ImageManager imMgr(m_db);
    std::vector<ItemImage> imgs;
    json data = buildData(m_db, imMgr, nullptr, nullptr, &itemIds, imgs);
    return exportData(data.dump(), imgs, path);
}

ExportResult ImportExport::exportModels(const std::vector<int64_t>& modelIds, const std::string& path) {
    ImageManager imMgr(m_db);
    std::vector<ItemImage> imgs;
    json data = buildData(m_db, imMgr, &modelIds, nullptr, nullptr, imgs);
    return exportData(data.dump(), imgs, path);
}

ImportSummary ImportExport::inspect(const std::string& path) {
    ImportSummary s;
    std::vector<uint8_t> buf;
    if (!readFile(path, buf) || buf.size() < 16) {
        s.error = "Arquivo inválido ou vazio.";
        return s;
    }
    if (memcmp(buf.data(), MAGIC, 4) != 0) {
        s.error = "Este arquivo não é um arquivo .collection válido.";
        return s;
    }
    uint32_t ver;
    memcpy(&ver, buf.data() + 4, 4);
    if (ver != FORMAT_VERSION) {
        s.error = "Versão do formato incompatível (" + std::to_string(ver) + ").";
        return s;
    }
    uint64_t jsz = readU64(buf.data() + 8);
    if (jsz > buf.size() - 16) {
        s.error = "Estrutura do arquivo corrompida.";
        return s;
    }
    std::string jsonStr((const char*)buf.data() + 16, (size_t)jsz);
    try {
        json data = json::parse(jsonStr);
        if (!data.contains("format") || data["format"] != "collection") {
            s.error = "Manifest inválido.";
            return s;
        }
        s.models = data.value("models", json::array()).size();
        s.folders = data.value("folders", json::array()).size();
        const json& items = data.value("items", json::array());
        s.items = items.size();
        for (auto& it : items) s.images += it.value("images", json::array()).size();
        s.json = std::move(jsonStr);
        s.ok = true;
    } catch (const std::exception& e) {
        s.error = std::string("JSON inválido: ") + e.what();
    }
    return s;
}

bool ImportExport::importFile(const std::string& path, const std::vector<ConflictChoice>& choices, std::string& error) {
    ImportSummary s = inspect(path);
    if (!s.ok) {
        error = s.error;
        return false;
    }
    json data;
    try {
        data = json::parse(s.json);
    }
    catch (const std::exception& e) {
        error = e.what();
        return false;
    }

    ModelRepository models(m_db);
    FolderRepository folders(m_db);
    ImageManager imgs(m_db);
    ItemRepository items(m_db);
    std::vector<uint8_t> buf;
    if (!readFile(path, buf)) {
        error = "Falha ao reler o arquivo.";
        return false;
    }
    const uint8_t* imgBase = buf.data() + 16 + readU64(buf.data() + 8);
    auto readImage = [&](int idx, std::vector<uint8_t>& out)->bool {
        const uint8_t* p = imgBase;
        for (int i = 0; i < idx; ++i) {
            uint64_t sz = readU64(p);
            p += 8 + sz;
        }
        uint64_t sz = readU64(p);
        out.assign(p + 8, p + 8 + sz);
        return true;
    };

    std::map<int64_t, int64_t> modelMap, folderMap, fieldMap, itemMap;
    Database& db = m_db;
    bool ok = db.inTransaction([&]{
        // 1. Modelos (Passo 50 â€” conflitos)
        for (auto& jm : data.value("models", json::array())) {
            int64_t oldId = jm["id"];
            std::string name = jm.value("name", "");
            Conflict choice = Conflict::Copy;
            for (auto& c : choices) if (c.modelName == name) choice = c.choice;
            int64_t newId = -1;
            // procura existente com mesmo nome
            int64_t existing = -1;
            for (auto& m : models.getAll()) if (m.name == name) {
                existing = m.id;
                break;
            }
            if (existing > 0 && choice == Conflict::Use) {
                modelMap[oldId] = existing;
                continue;
            }
            if (existing > 0 && choice == Conflict::Replace) {
                models.remove(existing);
            }
            models.create(name, jm.value("description", ""), newId);
            modelMap[oldId] = newId;
            int pos = 0;
            for (auto& jf : jm.value("fields", json::array())) {
                int64_t fid;
                models.createField(newId, jf.value("name",""), toFT(jf.value("type",0)), jf.value("position", pos++), fid);
                fieldMap[jf["id"]] = fid;
            }
        }
        // 2. Pastas
        for (auto& jf : data.value("folders", json::array())) {
            int64_t newId;
            folders.create(jf.value("title",""), newId);
            folderMap[jf["id"]] = newId;
        }
        // 3. Itens + valores + imagens
        for (auto& ji : data.value("items", json::array())) {
            int64_t mid = modelMap.count(ji["model_id"]) ? modelMap[ji["model_id"]] : ji["model_id"].get<int64_t>();
            std::optional<int64_t> fid;
            if (!ji["folder_id"].is_null() && folderMap.count(ji["folder_id"])) fid = folderMap[ji["folder_id"]];
            int64_t newId;
            if (!items.create(mid, fid, newId)) return false;
            itemMap[ji["id"]] = newId;
            for (auto& jv : ji.value("values", json::array())) {
                ItemValue v;
                v.fieldId = fieldMap.count(jv["field_id"]) ? fieldMap[jv["field_id"]] : jv["field_id"].get<int64_t>();
                v.text = jv.value("text",""); v.integer = jv.value("integer",(int64_t)0);
                v.real = jv.value("real",0.0); v.boolean = jv.value("boolean",false); v.date = jv.value("date","");
                items.setValue(newId, v);
            }
            std::vector<ItemImage> newImages;
            for (auto& jim : ji.value("images", json::array())) {
                ItemImage im;
                im.itemId = newId;
                im.position = jim.value("position", 0);
                im.filename = jim.value("filename","");
                im.mimeType = jim.value("mime_type","");
                readImage(jim.value("image_index", 0), im.data);
                newImages.push_back(std::move(im));
            }
            std::vector<int64_t> dummy;
            if (!newImages.empty() && !imgs.addImages(newId, newImages, dummy)) return false;
        }
        return true;
    });
    if (!ok) {
        error = "Falha na importação — banco permaneceu intacto (rollback).";
        return false;
    }
    return true;
}
