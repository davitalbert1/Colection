#pragma once
#include <string>
#include <vector>
#include "imgui.h"
#include "Entities.h"
#include "ImageManager.h"
#include "Model.h"
#include "Item.h"
#include "ImportExport.h"

class Database;

enum class Screen {
    Home,
    Items,
    Folders,
    Models,
    Settings
};

// Estado temporário do editor de itens (Passo 59) — só vai ao banco ao clicar Salvar
struct PendingImage {
    int64_t dbId = -1;
    std::string filename;
    std::vector<uint8_t> data;
    bool isNew = false;
};
struct ItemEditorState {
    bool open = false;
    int64_t editingItemId = -1;
    int64_t selectedModelId = -1;
    std::optional<int64_t> folderId;
    std::vector<PendingImage> images;
    std::map<int64_t, ItemValue> values;
};
struct ModelEditorState {
    bool open = false;
    int64_t editingModelId = -1;
    std::string name, description;
    struct DraftField {
        int64_t id = -1;
        std::string name;
        FieldType type = FieldType::Text;
    };
    std::vector<DraftField> fields;
    bool confirmFieldDelete = false;
    int64_t pendingFieldDelete = -1;
};

class UI {
public:
    UI(Database& db, ImageManager& imgs, ModelRepository& models, ItemRepository& items,
       FolderRepository& folders, ImportExport& ie)
        : m_db(db), m_imgs(imgs), m_models(models), m_items(items), m_folders(folders), m_ie(ie) {}

    void applyDarkTheme(); // Passo 5
    void draw(); // frame completo
    Screen screen() const {
        return m_screen;
    }
    void setScreen(Screen s) {
        m_screen = s;
    }

    // Aviso/mensagem (Passo 54)
    void notify(const std::string& msg, bool isError = false);
    std::string lastError() const {
        return m_error;
    }

private:
    void drawTopBar();
    void drawSidebar();
    void drawScreen();
    void drawHome();
    void drawItems();
    void drawFolders();
    void drawModels();
    void drawSettings();
    void drawItemEditor(); void drawModelEditor();
    void drawItemDetails();
    void drawConfirmDialogs();
    void drawImageStrip(); // miniaturas + drag and drop (Passo 31)
    void refreshItemCache();
    std::string displayTitle(const ItemSummary& it);

    Database& m_db;
    ImageManager& m_imgs;
    ModelRepository& m_models;
    ItemRepository& m_items;
    FolderRepository& m_folders;
    ImportExport& m_ie;

    Screen m_screen = Screen::Home;
    bool m_dirtyItems = true; // cache (Passo 60)
    std::vector<ItemSummary> m_itemCache;
    std::optional<int64_t> m_currentFolder;
    std::vector<int64_t> m_selectedIds; // seleção múltipla (Passo 41)
    char m_search[256] = {0};
    int64_t m_openItem = -1;
    ItemEditorState m_editor;
    ModelEditorState m_modelEditor;

    // Diálogos de confirmação (Passos 63-66)
    enum class ConfirmKind { None, DeleteItem, DeleteFolder, DeleteModel, ReplaceModel };
    ConfirmKind m_confirm = ConfirmKind::None;
    int64_t m_confirmTarget = -1;
    bool m_confirmSecondary = false;

    // Notificações
    std::string m_notify, m_error;
    float m_notifyTime = 0.f;

    // Export/import
    bool m_showImportSummary = false;
    ImportSummary m_import;
    std::string m_importPath;

    // Settings
    bool m_confirmDeletes = true;
    bool m_useImageCache = true;
};
