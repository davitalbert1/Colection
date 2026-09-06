#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

enum class FieldType {
    Text = 0,
    Integer = 1,
    Decimal = 2,
    Boolean = 3,
    Date = 4
};

const char* fieldTypeName(FieldType t);

struct Model {
    int64_t id = -1;
    std::string name;
    std::string description;
    std::string createdAt;
    std::string updatedAt;
};

struct ModelField {
    int64_t id = -1;
    int64_t modelId = -1;
    std::string name;
    FieldType type = FieldType::Text;
    int position = 0;
};

struct Folder {
    int64_t id = -1;
    std::string title;
    std::string createdAt;
    std::string updatedAt;
};

struct Item {
    int64_t id = -1;
    int64_t modelId = -1;
    std::optional<int64_t> folderId;
    std::string createdAt;
    std::string updatedAt;
};

struct ItemValue {
    int64_t fieldId = -1;
    std::string text;
    int64_t integer = 0;
    double real = 0.0;
    bool boolean = false;
    std::string date; // ISO yyyy-mm-dd
};

struct ItemImage {
    int64_t id = -1;
    int64_t itemId = -1;
    int position = 0;
    std::string filename;
    std::string mimeType;
    std::vector<uint8_t> data;
};

struct ItemSummary {
    Item item;
    std::string modelName;
    std::string title; // detectado (Passo 35)
    int imageCount = 0;
    bool hasImage = false; // possui primeira imagem
};
