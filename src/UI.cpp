#include "UI.h"
#include "Database.h"
#include "nlohmann/json.hpp"
#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstring>

// caminho do arquivo selecionado para importaÃ§Ã£o
static std::string g_importPath;

using json = nlohmann::json;

// Tema escuro (Passo 5)
void UI::applyDarkTheme() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = 6.f;
    s.FrameRounding = 5.f;
    s.GrabRounding = 5.f;
    s.PopupRounding = 6.f;
    s.ScrollbarRounding = 6.f;
    s.ChildRounding = 6.f;
    s.FramePadding = ImVec2(10, 6);
    s.ItemSpacing = ImVec2(10, 8);
    s.WindowPadding = ImVec2(12, 12);
    s.WindowBorderSize = 0.f;
    s.FrameBorderSize = 0.f;

    ImVec4 bg(0.09f,0.09f,0.11f,1.f), panel(0.12f,0.12f,0.15f,1.f),
           field(0.16f,0.16f,0.20f,1.f), fieldHover(0.20f,0.20f,0.25f,1.f),
           accent(0.26f,0.52f,0.96f,1.f), txt(0.92f,0.92f,0.94f,1.f),
           dim(0.55f,0.56f,0.60f,1.f), border(0.22f,0.22f,0.27f,1.f);
    auto set = [&](ImGuiCol i, ImVec4 v){
        s.Colors[i] = v;
    };
    set(ImGuiCol_WindowBg, bg);
    set(ImGuiCol_ChildBg, panel);
    set(ImGuiCol_PopupBg, panel);
    set(ImGuiCol_FrameBg, field);
    set(ImGuiCol_FrameBgHovered, fieldHover);
    set(ImGuiCol_FrameBgActive, fieldHover);
    set(ImGuiCol_Button, field);
    set(ImGuiCol_ButtonHovered, accent);
    set(ImGuiCol_ButtonActive, accent);
    set(ImGuiCol_Header, fieldHover);
    set(ImGuiCol_HeaderHovered, accent);
    set(ImGuiCol_HeaderActive, accent);
    set(ImGuiCol_Text, txt);
    set(ImGuiCol_TextDisabled, dim);
    set(ImGuiCol_TitleBg, bg);
    set(ImGuiCol_TitleBgActive, bg);
    set(ImGuiCol_Border, border);
    set(ImGuiCol_Separator, border);
    set(ImGuiCol_ScrollbarBg, panel);
    set(ImGuiCol_ScrollbarGrab, fieldHover);
    set(ImGuiCol_ScrollbarGrabHovered, accent);
    set(ImGuiCol_CheckMark, accent);
    set(ImGuiCol_SliderGrab, accent);
    set(ImGuiCol_SliderGrabActive, accent);
    set(ImGuiCol_TextSelectedBg, ImVec4(accent.x,accent.y,accent.z,0.35f));
    set(ImGuiCol_TableHeaderBg, panel);
    set(ImGuiCol_TableRowBg, bg);
    set(ImGuiCol_TableRowBgAlt, panel);
}

void UI::notify(const std::string& msg, bool isError) {
    if (isError) {
        m_error = msg;
    } else {
        m_notify = msg;
        m_error.clear();
    }
    m_notifyTime = 4.f;
}

void UI::refreshItemCache() {
    if (!m_search[0]) m_itemCache = m_items.list(m_currentFolder);
    else m_itemCache = m_items.search(m_search);
    m_dirtyItems = false;
}

// tÃ­tulo detectado (Passo 35)
std::string UI::displayTitle(const ItemSummary& it) {
    if (!it.title.empty()) return it.title;
    return it.modelName + " #" + std::to_string(it.item.id);
}

void UI::draw() {
    // Atalhos (Passo 62)
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_N) && !m_editor.open) {
        m_editor = ItemEditorState{};
        m_editor.open = true;
        m_dirtyItems = true;
    }
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_F)) ImGui::SetKeyboardFocusHere(-1);

    drawTopBar();
    drawSidebar();
    ImGui::SameLine();
    ImGui::BeginChild("##content", ImVec2(0,0), ImGuiChildFlags_Borders);
    drawScreen();
    ImGui::EndChild();

    drawItemEditor();
    drawModelEditor();
    drawConfirmDialogs();

    // DiÃ¡logo de importaÃ§Ã£o (resumo â€” Passo 49)
    if (m_showImportSummary && m_import.ok) {
        ImGui::OpenPopup("ImportaÃ§Ã£o");
        m_showImportSummary = false;
        m_confirmSecondary = true;
    }
    if (m_confirmSecondary && ImGui::BeginPopupModal("ImportaÃ§Ã£o", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Modelos: %d", m_import.models);
        ImGui::Text("Pastas: %d", m_import.folders);
        ImGui::Text("Itens: %d", m_import.items);
        ImGui::Text("Imagens: %d", m_import.images);
        ImGui::Separator();
        if (ImGui::Button("Cancelar", ImVec2(120,0))) {
            m_confirmSecondary = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Importar", ImVec2(120,0))) {
            std::string err;
            if (m_ie.importFile(g_importPath, {}, err)) notify("âœ“ ImportaÃ§Ã£o concluÃ­da.");
            else notify("âœ• " + err, true);
            m_confirmSecondary = false;
            m_dirtyItems = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // NotificaÃ§Ã£o temporÃ¡ria (Passo 54)
    if (m_notifyTime > 0.f) {
        m_notifyTime -= io.DeltaTime;
        if (!m_error.empty()) {
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x*0.5f, 30), ImGuiCond_Always, ImVec2(0.5f,0.f));
            if (ImGui::Begin("##toast", nullptr, ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoNav|
                ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings)) {
                ImGui::TextColored(ImVec4(0.95f,0.4f,0.4f,1.f), "%s", m_error.c_str());
            }
            ImGui::End();
        } else if (!m_notify.empty()) {
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x*0.5f, 30), ImGuiCond_Always, ImVec2(0.5f,0.f));
            if (ImGui::Begin("##toast2", nullptr, ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoNav|
                ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings)) {
                ImGui::TextColored(ImVec4(0.4f,0.9f,0.5f,1.f), "%s", m_notify.c_str());
            }
            ImGui::End();
        }
        if (m_notifyTime <= 0.f) {
            m_notify.clear();
            m_error.clear();
        }
    }
}

void UI::drawTopBar() {
    ImGui::BeginMainMenuBar();
    ImGui::TextUnformatted("Coleção");
    ImGui::Spacing();
    float w = ImGui::GetWindowWidth();
    ImGui::SetNextItemWidth(320);
    ImGui::SameLine(w - 700);
    if (ImGui::InputTextWithHint("##search", "Pesquisar (Ctrl+F)", m_search, sizeof(m_search))) {
        m_dirtyItems = true;
        setScreen(Screen::Items);
    }
    ImGui::EndMainMenuBar();
}

void UI::drawSidebar() {
    ImGui::BeginChild("##sidebar", ImVec2(190,0), ImGuiChildFlags_Borders);
    auto nav = [&](const char* label, Screen s) {
        if (ImGui::Selectable(label, m_screen == s, ImGuiSelectableFlags_SpanAllColumns)) {
            m_screen = s;
            m_dirtyItems = true;
        }
    };
    nav("Início", Screen::Home);
    nav("Itens", Screen::Items);
    nav("Pastas", Screen::Folders);
    nav("Modelos", Screen::Models);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    nav("Configurações", Screen::Settings);
    ImGui::EndChild();
}

// Telas (Passos 6, 33-36, 37, 65, 66)
void UI::drawScreen() {
    switch (m_screen) {
        case Screen::Home:
            drawHome();
            break;
        case Screen::Items:
            drawItems();
            break;
        case Screen::Folders:
            drawFolders();
            break;
        case Screen::Models:
            drawModels();
            break;
        case Screen::Settings:
            drawSettings();
            break;
    }
}

void UI::drawHome() {
    // Passo 65 â€” pÃ¡gina inicial com estatÃ­sticas
    int64_t nModels = m_models.getAll().size();
    int64_t nFolders = m_folders.getAll().size();
    int64_t nItems = m_items.countAll();
    int64_t nImages = 0;
    {
        sqlite3_stmt* st;
        if (sqlite3_prepare_v2(m_db.handle(), "SELECT COUNT(*) FROM images;", -1, &st, nullptr) == SQLITE_OK) {
            if (sqlite3_step(st) == SQLITE_ROW) nImages = sqlite3_column_int64(st,0);
            sqlite3_finalize(st);
        }
    }
    ImGui::Dummy(ImVec2(0, 10));
    ImGui::TextUnformatted("Coleção");
    ImGui::Dummy(ImVec2(0, 6));
    ImGui::TextColored(ImVec4(0.55f,0.56f,0.60f,1.f), "Modelos: %lld    Pastas: %lld    Itens: %lld    Imagens: %lld",
        (long long)nModels, (long long)nFolders, (long long)nItems, (long long)nImages);
    ImGui::Dummy(ImVec2(0, 10));
    if (ImGui::Button("+ Novo item", ImVec2(150,0))) {
        m_editor = ItemEditorState{};
        m_editor.open = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Gerenciar modelos", ImVec2(170,0))) setScreen(Screen::Models);
    ImGui::SameLine();
    if (ImGui::Button("Importar", ImVec2(110,0))) {
        // diÃ¡logos nativos simples via popup de caminho
        ImGui::OpenPopup("##importpath");
    }
    if (ImGui::BeginPopup("##importpath")) {
        static char p[512] = "";
        ImGui::InputTextWithHint("##ip", "Caminho do arquivo .collection", p, sizeof(p));
        if (ImGui::Button("Continuar") && p[0]) {
            g_importPath = p;
            m_import = m_ie.inspect(p);
            if (m_import.ok) {
                m_confirmSecondary = true;
                ImGui::CloseCurrentPopup();
            }
            else notify("âœ• " + m_import.error, true);
        }
        ImGui::SameLine();
        if (ImGui::Button("Fechar")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::Dummy(ImVec2(0, 14));
    ImGui::TextColored(ImVec4(0.55f,0.56f,0.60f,1.f), "Últimos itens adicionados");
    if (m_dirtyItems) refreshItemCache();
    int shown = 0;
    for (auto& s : m_itemCache) {
        if (shown++ >= 8) break;
        if (ImGui::Selectable(displayTitle(s).c_str())) {
            m_openItem = s.item.id;
        }
    }
}

void UI::drawItems() {
    if (m_dirtyItems) refreshItemCache();
    ImGui::TextUnformatted(m_currentFolder ? "Itens da pasta" : "Itens");
    ImGui::SameLine();
    if (m_currentFolder) {
        Folder f;
        if (m_folders.get(*m_currentFolder, f) && !f.title.empty()) ImGui::TextColored(ImVec4(0.55f,0.56f,0.60f,1.f), "Â· %s", f.title.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton("â† voltar")) {
            m_currentFolder.reset();
            m_dirtyItems = true;
        }
    }
    ImGui::SameLine(ImGui::GetWindowWidth() - 130);
    if (ImGui::Button("+ Novo item")) {
        m_editor = ItemEditorState{};
        m_editor.open = true;
        m_editor.folderId = m_currentFolder;
    }

    // SeleÃ§Ã£o mÃºltipla (Passo 41)
    if (!m_selectedIds.empty()) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.16f,0.20f,0.30f,1.f));
        ImGui::BeginChild("##selbar", ImVec2(0, 44), ImGuiChildFlags_Borders);
        ImGui::Text("%d item(ns) selecionado(s)", (int)m_selectedIds.size());
        ImGui::SameLine();
        if (ImGui::Button("Exportar")) {
            m_ie.exportItems(m_selectedIds, "export.collection");
            notify("âœ“ Exportado para export.collection");
        }
        ImGui::SameLine();
        if (ImGui::Button("Mover para pasta")) ImGui::OpenPopup("##movesel");
        if (ImGui::BeginPopup("##movesel")) {
            if (ImGui::MenuItem("Sem pasta")) {
                for (auto id : m_selectedIds) {
                    Item it;
                    if (m_items.get(id, it)) {
                        it.folderId.reset();
                        m_items.update(it);
                    }
                }
                m_selectedIds.clear();
                m_dirtyItems = true;
                ImGui::CloseCurrentPopup();
            }
            for (auto& f : m_folders.getAll()) {
                if (ImGui::MenuItem(f.title.empty() ? "Pasta sem tÃ­tulo" : f.title.c_str())) {
                    for (auto id : m_selectedIds) {
                        Item it;
                        if (m_items.get(id, it)) {
                            it.folderId = f.id;
                            m_items.update(it);
                        }
                    }
                    m_selectedIds.clear();
                    m_dirtyItems = true;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Excluir")) {
            m_confirm = ConfirmKind::DeleteItem;
            m_confirmTarget = -1;
            m_confirmSecondary = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Limpar seleÃ§Ã£o")) m_selectedIds.clear();
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    // Grid (Passo 33) â€” cÃ©lula fixa, miniatura = primeira imagem (Passo 34)
    const float cellW = 170, cellH = 210, thumbH = 130;
    int cols = std::max(1, (int)((ImGui::GetContentRegionAvail().x) / (cellW + 10)));
    int idx = 0;
    for (auto& s : m_itemCache) {
        if (idx % cols != 0) ImGui::SameLine();
        ImGui::PushID((int)s.item.id);
        ImGui::BeginChild("cell", ImVec2(cellW, cellH), ImGuiChildFlags_Borders);
        ImVec2 c0 = ImGui::GetCursorPos();
        bool sel = std::find(m_selectedIds.begin(), m_selectedIds.end(), s.item.id) != m_selectedIds.end();
        if (ImGui::Selectable("##sel", &sel, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(cellW, cellH))) {
            if (ImGui::IsMouseDoubleClicked(0)) m_openItem = s.item.id;
            if (sel) {
                if (std::find(m_selectedIds.begin(), m_selectedIds.end(), s.item.id) == m_selectedIds.end()) m_selectedIds.push_back(s.item.id);
            } else {
                m_selectedIds.erase(std::remove(m_selectedIds.begin(), m_selectedIds.end(), s.item.id), m_selectedIds.end());
            }
        }
        if (s.hasImage) {
            int64_t firstId = -1;
            auto imgs = m_imgs.getImages(s.item.id, false);
            if (!imgs.empty()) firstId = imgs[0].id;
            if (firstId > 0) {
                TextureHandle tex = m_imgs.texture(firstId, 256);
                if (tex) ImGui::SetCursorPos(ImVec2(c0.x + 5, c0.y + 5));
                ImGui::Image(tex, ImVec2(cellW - 10, thumbH - 10));
            }
        } else {
            ImGui::SetCursorPos(ImVec2(c0.x + 5, c0.y + 5));
            ImGui::BeginChild("##ph", ImVec2(cellW-10, thumbH-10), ImGuiChildFlags_Borders);
            ImVec2 sz = ImGui::GetContentRegionAvail();
            ImGui::SetCursorPos(ImVec2(sz.x*0.5f - 30, sz.y*0.5f - 8));
            ImGui::TextColored(ImVec4(0.4f,0.4f,0.45f,1.f), "Sem imagem");
            ImGui::EndChild();
        }
        ImGui::SetCursorPos(ImVec2(8, c0.y + thumbH));
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + cellW - 16);
        ImGui::TextWrapped("%s", displayTitle(s).c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndChild();
        ImGui::PopID();
        ++idx;
    }
    if (m_itemCache.empty()) ImGui::TextColored(ImVec4(0.5f,0.5f,0.55f,1.f), "Nenhum item. Clique em '+ Novo item'.");
}

void UI::drawFolders() {
    ImGui::TextUnformatted("Pastas");
    ImGui::SameLine(ImGui::GetWindowWidth() - 130);
    if (ImGui::Button("+ Nova pasta")) ImGui::OpenPopup("##newfolder");
    if (ImGui::BeginPopupModal("Nova pasta", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char name[256] = "";
        ImGui::InputTextWithHint("##fn", "Nome da pasta (opcional)", name, sizeof(name));
        ImGui::Separator();
        if (ImGui::Button("Cancelar")) {
            name[0]=0;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Criar")) {
            int64_t id;
            m_folders.create(name, id);
            name[0]=0;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::Separator();
    for (auto& f : m_folders.getAll()) {
        ImGui::PushID((int)f.id);
        const char* title = f.title.empty() ? "Pasta sem tÃ­tulo" : f.title.c_str(); // Passo 36
        if (ImGui::SmallButton("Abrir")) {
            m_currentFolder = f.id;
            setScreen(Screen::Items);
            m_dirtyItems = true;
        }
        ImGui::SameLine();
        ImGui::Text("%s (%lld itens)", title, (long long)m_folders.itemCount(f.id));
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);
        if (ImGui::SmallButton("Renomear")) ImGui::OpenPopup("##ren");
        if (ImGui::BeginPopup("##ren")) {
            static char nm[256];
            ImGui::InputText("##rn", nm, sizeof(nm));
            if (ImGui::Button("Salvar")) {
                m_folders.rename(f.id, nm);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Exportar")) {
            m_ie.exportFolder(f.id, "pasta_" + std::to_string(f.id) + ".collection");
            notify("âœ“ Pasta exportada.");
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Excluir")) {
            m_confirm = ConfirmKind::DeleteFolder;
            m_confirmTarget = f.id;
            m_confirmSecondary = false;
        }
        ImGui::PopID();
    }
}

void UI::drawSettings() {
    // Passo 66
    ImGui::TextUnformatted("ConfiguraÃ§Ãµes");
    ImGui::Separator();
    ImGui::Text("Banco de dados: data/collection.db");
    ImGui::Text("VersÃ£o do schema: %d", m_db.schemaVersion());
    ImGui::Text("Tema: Escuro (padrÃ£o)");
    ImGui::Checkbox("ConfirmaÃ§Ã£o antes de excluir", &m_confirmDeletes);
    ImGui::Checkbox("Cache de imagens", &m_useImageCache);
    ImGui::Separator();
    ImGui::Text("Itens em cache de texturas: %zu", m_imgs.cacheSize());
    ImGui::SameLine();
    if (ImGui::SmallButton("Limpar cache")) {
        m_imgs.clearCache();
        notify("âœ“ Cache limpo.");
    }
    ImGui::Dummy(ImVec2(0,10));
    ImGui::TextColored(ImVec4(0.55f,0.56f,0.60f,1.f), "Exportar / Importar");
    if (ImGui::Button("Exportar toda a coleÃ§Ã£o")) {
        auto r = m_ie.exportAll("colecao_completa.collection");
        notify(r.ok ? "âœ“ ColeÃ§Ã£o exportada." : "âœ• " + r.error, !r.ok);
    }
    ImGui::SameLine();
    if (ImGui::Button("Importar")) {
        char fn[512] = "";
        FILE* f = _popen("powershell -STA -Command \"Add-Type -AssemblyName System.Windows.Forms; $d=New-Object System.Windows.Forms.OpenFileDialog; $d.Filter='Collection (*.collection)|*.collection'; if($d.ShowDialog() -eq 'OK'){$d.FileName}\"", "r");
        if (f && fgets(fn, sizeof(fn), f)) {
            fn[strcspn(fn, "\r\n")] = 0;
            g_importPath = fn;
            m_import = m_ie.inspect(fn);
            if (m_import.ok) m_confirmSecondary = true;
            else notify("âœ• " + m_import.error, true);
        }
        if (f) _pclose(f);
    }
}

// Modelos (Passos 13-18, 64)
void UI::drawModels() {
    ImGui::TextUnformatted("Modelos");
    ImGui::SameLine(ImGui::GetWindowWidth() - 150);
    if (ImGui::Button("+ Novo modelo")) {
        m_modelEditor = ModelEditorState{};
        m_modelEditor.open = true;
    }
    ImGui::Separator();
    for (auto& m : m_models.getAll()) {
        ImGui::PushID((int)m.id);
        ImGui::BeginChild("mrow", ImVec2(0, 64), ImGuiChildFlags_Borders);
        ImGui::TextUnformatted(m.name.c_str());
        ImGui::TextColored(ImVec4(0.55f,0.56f,0.60f,1.f), "%lld campo(s) Â· %lld item(ns)",
            (long long)m_models.getFields(m.id).size(), (long long)m_models.itemCount(m.id));
        ImGui::SameLine(ImGui::GetWindowWidth() - 100);
        if (ImGui::Button("Editar")) {
            m_modelEditor = ModelEditorState{};
            m_modelEditor.open = true;
            m_modelEditor.editingModelId = m.id;
            m_modelEditor.name = m.name;
            m_modelEditor.description = m.description;
            for (auto& f : m_models.getFields(m.id))
                m_modelEditor.fields.push_back({f.id, f.name, f.type});
        }
        ImGui::SameLine();
        if (ImGui::Button("Exportar")) {
            m_ie.exportModels({m.id}, "modelo_" + std::to_string(m.id) + ".collection");
            notify("âœ“ Modelo exportado.");
        }
        ImGui::SameLine();
        if (ImGui::Button("Excluir")) {
            m_confirm = ConfirmKind::DeleteModel;
            m_confirmTarget = m.id;
            m_confirmSecondary = false;
        }
        ImGui::EndChild();
        ImGui::PopID();
    }
}

void UI::drawModelEditor() {
    if (!m_modelEditor.open) return;
    ImGui::SetNextWindowSize(ImVec2(560, 480), ImGuiCond_Appearing);
    if (!ImGui::Begin("Editor de modelo", &m_modelEditor.open)) {
        ImGui::End();
        return;
    }
    auto& e = m_modelEditor;
    {
        static char nb[256];
        strncpy(nb, e.name.c_str(), 255);
        if (ImGui::InputTextWithHint("Nome", "ex.: Livro", nb, sizeof(nb))) e.name = nb;
    }
    {
        static char db2[1024];
        strncpy(db2, e.description.c_str(), 1023);
        if (ImGui::InputTextMultiline("Descricao", db2, sizeof(db2), ImVec2(-1, 50))) e.description = db2;
    }
    ImGui::Separator();
    ImGui::TextUnformatted("Campos:");
    int rm = -1, up = -1, down = -1;
    for (size_t i = 0; i < e.fields.size(); ++i) {
        ImGui::PushID((int)i);
        ImGui::PushItemWidth(200);
        {
            static char fb[128];
            strncpy(fb, e.fields[i].name.c_str(), 127);
            if (ImGui::InputText("##n", fb, sizeof(fb))) e.fields[i].name = fb;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(140);
        int t = (int)e.fields[i].type;
        if (ImGui::Combo("##t", &t, "Texto\0NÃºmero inteiro\0NÃºmero decimal\0Checkbox\0Data\0"))
            e.fields[i].type = (FieldType)t;
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::ArrowButton("##u", ImGuiDir_Up) && i > 0) up = (int)i;
        ImGui::SameLine();
        if (ImGui::ArrowButton("##d", ImGuiDir_Down) && i + 1 < e.fields.size()) down = (int)i;
        ImGui::SameLine();
        if (ImGui::SmallButton("Excluir")) {
            // Passo 16 â€” confirmar se campo possui dados (somente campos jÃ¡ salvos)
            if (e.fields[i].id > 0 && m_models.fieldUseCount(e.fields[i].id) > 0) {
                e.confirmFieldDelete = true;
                e.pendingFieldDelete = e.fields[i].id;
            } else rm = (int)i;
        }
        ImGui::PopID();
    }
    if (rm >= 0) e.fields.erase(e.fields.begin() + rm);
    if (up >= 0) std::swap(e.fields[up-1], e.fields[up]);
    if (down >= 0) std::swap(e.fields[down+1], e.fields[down]);
    if (e.confirmFieldDelete && ImGui::BeginPopupModal("Excluir campo", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        int64_t n = m_models.fieldUseCount(e.pendingFieldDelete);
        ImGui::Text("Este campo possui dados em %lld item(s).", (long long)n);
        ImGui::TextUnformatted("Ao remover o campo, esses dados serÃ£o perdidos.");
        ImGui::Separator();
        if (ImGui::Button("Cancelar")) {
            e.confirmFieldDelete = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Excluir campo")) {
            m_models.removeField(e.pendingFieldDelete);
            e.fields.erase(std::remove_if(e.fields.begin(), e.fields.end(), [&](auto& f){
                return f.id == e.pendingFieldDelete;
            }), e.fields.end());
            e.confirmFieldDelete = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    } else if (e.confirmFieldDelete) ImGui::OpenPopup("Excluir campo");

    if (ImGui::Button("+ Adicionar campo"))
        e.fields.push_back({-1, "", FieldType::Text});
    ImGui::Separator();
    if (ImGui::Button("Cancelar", ImVec2(110,0))) e.open = false;
    ImGui::SameLine();
    if (ImGui::Button("Salvar", ImVec2(110,0))) {
        m_db.inTransaction([&]{
            if (e.editingModelId < 0) {
                int64_t mid;
                if (!m_models.create(e.name, e.description, mid)) return false;
                e.editingModelId = mid;
            } else {
                Model m;
                m.id = e.editingModelId;
                m.name = e.name;
                m.description = e.description;
                if (!m_models.update(m)) return false;
            }
            std::vector<int64_t> order;
            int pos = 0;
            for (auto& f : e.fields) {
                if (f.id < 0) {
                    int64_t fid;
                    m_models.createField(e.editingModelId, f.name, f.type, pos++, fid);
                    order.push_back(fid);
                } else {
                    ModelField mf;
                    mf.id = f.id;
                    mf.modelId = e.editingModelId;
                    mf.name = f.name;
                    mf.type = f.type;
                    mf.position = pos++;
                    m_models.updateField(mf);
                    order.push_back(f.id);
                }
            }
            return true;
        });
        e.open = false;
        notify("âœ“ Modelo salvo com sucesso.");
    }
    ImGui::End();
}

// Editor de item (Passos 22-23, 26, 29-31, 38, 59)
void UI::drawItemEditor() {
    if (!m_editor.open) return;
    ImGui::SetNextWindowSize(ImVec2(620, 600), ImGuiCond_Appearing);
    if (!ImGui::Begin(m_editor.editingItemId < 0 ? "Novo item" : "Editar item", &m_editor.open)) {
        ImGui::End();
        return;
    }
    auto& e = m_editor;

    // SeleÃ§Ã£o de modelo (Passo 22)
    if (e.editingItemId < 0) {
        static int selModel = -1;
        auto all = m_models.getAll();
        std::vector<const char*> names;
        for (auto& m : all) names.push_back(m.name.c_str());
        if (!names.empty()) {
            if (selModel < 0) selModel = 0;
            if (ImGui::Combo("Modelo", &selModel, names.data(), (int)names.size()))
                e.selectedModelId = all[selModel].id;
            e.selectedModelId = all[selModel].id;
        } else {
            ImGui::TextColored(ImVec4(0.95f,0.7f,0.4f,1.f), "Crie um modelo primeiro (tela Modelos).");
        }
    }

    // Pasta (Passo 26)
    auto folders = m_folders.getAll();
    int fidx = 0;
    std::vector<const char*> fnames = {"Sem pasta"};
    int cur = 0;
    for (size_t i = 0; i < folders.size(); ++i) {
        fnames.push_back(folders[i].title.empty() ? "Pasta sem tÃ­tulo" : folders[i].title.c_str());
        if (e.folderId && *e.folderId == folders[i].id) cur = (int)i + 1;
    }
    fidx = cur;
    if (ImGui::Combo("Pasta", &fidx, fnames.data(), (int)fnames.size()))
        e.folderId = (fidx == 0) ? std::nullopt : std::optional<int64_t>(folders[fidx-1].id);

    // Campos do modelo â€” vindos do banco (Regra 9/10)
    if (e.selectedModelId > 0 || e.editingItemId > 0) {
        int64_t mid = e.editingItemId > 0 ? e.selectedModelId : e.selectedModelId;
        if (e.editingItemId > 0 && mid <= 0) {
            Item it;
            if (m_items.get(e.editingItemId, it)) mid = it.modelId;
        }
        auto fields = m_models.getFields(mid);
        if (e.values.empty() && e.editingItemId > 0) e.values = m_items.getValues(e.editingItemId);
        ImGui::Separator();
        for (auto& f : fields) {
            ItemValue& v = e.values[f.id];
            v.fieldId = f.id;
            ImGui::PushID((int)f.id);
            switch (f.type) {
                case FieldType::Text: {
                        char tb[512];
                        strncpy(tb, v.text.c_str(), 511);
                        if (ImGui::InputText(f.name.c_str(), tb, sizeof(tb))) v.text = tb;
                    }
                    break;
                case FieldType::Integer: {
                    int64_t i64 = v.integer;
                    if (ImGui::InputScalar(f.name.c_str(), ImGuiDataType_S64, &i64)) v.integer = i64;
                    break;
                }
                case FieldType::Decimal: {
                    float fl = (float)v.real;
                    if (ImGui::InputFloat(f.name.c_str(), &fl)) v.real = fl;
                    break;
                }
                case FieldType::Boolean:
                    ImGui::Checkbox(f.name.c_str(), &v.boolean);
                    break;
                case FieldType::Date:
                {
                    char dtb[32];
                    strncpy(dtb, v.date.c_str(), 31);
                    if (ImGui::InputTextWithHint(f.name.c_str(), "AAAA-MM-DD", dtb, sizeof(dtb))) v.date = dtb;
                }
                break;
            }
            ImGui::PopID();
        }
    }

    // Imagens (Passos 29-31)
    drawImageStrip();

    ImGui::Separator();
    if (ImGui::Button("Cancelar", ImVec2(110,0))) {
        e.open = false;
        e.values.clear();
        e.images.clear();
    }
    ImGui::SameLine();
    if (ImGui::Button("Salvar", ImVec2(110,0))) {
        bool ok = m_db.inTransaction([&]{
            if (e.editingItemId < 0) {
                int64_t nid;
                if (!m_items.create(e.selectedModelId, e.folderId, nid)) return false;
                e.editingItemId = nid;
            } else {
                Item it;
                if (!m_items.get(e.editingItemId, it)) return false;
                it.folderId = e.folderId;
                if (!m_items.update(it)) return false;
            }
            for (auto& [fid, v] : e.values) if (!m_items.setValue(e.editingItemId, v)) return false;
            // imagens novas
            std::vector<ItemImage> news;
            for (auto& p : e.images) if (p.isNew) {
                ItemImage im;
                im.itemId = e.editingItemId;
                im.filename = p.filename;
                im.data = p.data;
                news.push_back(std::move(im));
            }
            if (!news.empty()) {
                std::vector<int64_t> ids(news.size());
                if (!m_imgs.addImages(e.editingItemId, news, ids)) return false;
            }
            // ordem final das imagens (inclui existentes + novas)
            std::vector<int64_t> order;
            for (auto& p : e.images) {
                if (p.dbId > 0) order.push_back(p.dbId);
            }
            if (!order.empty() && !m_imgs.reorder(e.editingItemId, order)) return false;
            return true;
        });
        notify(ok ? "âœ“ Item salvo com sucesso." : "âœ• Falha ao salvar o item.", !ok);
        if (ok) {
            e.open = false;
            e.values.clear();
            e.images.clear();
            m_dirtyItems = true;
        }
    }
    ImGui::End();
}

void UI::drawImageStrip() {
    auto& e = m_editor;
    ImGui::TextUnformatted("Imagens");
    if (e.editingItemId > 0 && e.images.empty()) {
        for (auto& im : m_imgs.getImages(e.editingItemId, true))
            e.images.push_back({im.id, im.filename, std::move(im.data), false});
    }
    if (ImGui::Button("+ Adicionar imagens")) ImGui::OpenPopup("##addimgs");
    if (ImGui::BeginPopup("##addimgs")) {
        static char p[512] = "";
        ImGui::InputTextWithHint("##ap", "Caminho(s) separados por ;", p, sizeof(p));
        ImGui::SameLine();
        if (ImGui::Button("Procurar...")) {
            FILE* f = _popen("powershell -STA -Command \"Add-Type -AssemblyName System.Windows.Forms; $d=New-Object System.Windows.Forms.OpenFileDialog; $d.Multiselect=$true; $d.Filter='Imagens|*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.webp'; if($d.ShowDialog() -eq 'OK'){$d.FileNames -join ';'}\"", "r");
            char buf[1024];
            if (f && fgets(buf, sizeof(buf), f)) {
                buf[strcspn(buf,"\r\n")]=0;
                snprintf(p, sizeof(p), "%s", buf);
            }
            if (f) _pclose(f);
        }
        if (ImGui::Button("Adicionar") && p[0]) {
            std::stringstream ss(p);
            std::string tok;
            while (std::getline(ss, tok, ';')) {
                if (tok.empty()) continue;
                std::ifstream f(tok, std::ios::binary);
                if (!f) continue;
                std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                std::filesystem::path fp(tok);
                e.images.push_back({-1, fp.filename().string(), std::move(data), true});
            }
            p[0] = 0;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // miniaturas + drag and drop (Passo 31)
    ImGui::BeginChild("##imgstrip", ImVec2(0, 160), ImGuiChildFlags_Borders);
    int rm = -1;
    for (size_t i = 0; i < e.images.size(); ++i) {
        auto& p = e.images[i];
        ImGui::PushID((int)i);
        ImGui::BeginGroup();
        TextureHandle tex = (TextureHandle)nullptr;
        if (p.dbId > 0) tex = m_imgs.texture(p.dbId, 128);
        if (!tex && !p.data.empty()) {
            // decodifica temporariamente para exibir novas imagens (sem upload dedicado â€” usa cache do db)
            int w,h,c;
            unsigned char* px = stbi_load_from_memory(p.data.data(), (int)p.data.size(), &w, &h, &c, 4);
            if (px) stbi_image_free(px);
        }
        ImGui::Image(tex, ImVec2(96, 96), ImVec2(0,0), ImVec2(1,1),
                     ImVec4(1,1,1,1), ImVec4(0.2f,0.2f,0.25f,1));
        if ((intptr_t)tex == 0) ImGui::TextColored(ImVec4(0.95f,0.6f,0.3f,1.f), "âš  imagem");
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("IMG_REORDER", &i, sizeof(size_t));
            ImGui::Text("%s", p.filename.c_str());
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* pl = ImGui::AcceptDragDropPayload("IMG_REORDER")) {
                size_t src = *(const size_t*)pl->Data;
                if (src != i) std::swap(e.images[src], e.images[i]); // mantÃ©m troca simples e persistida
            }
            ImGui::EndDragDropTarget();
        }
        if (ImGui::SmallButton("Excluir")) rm = (int)i;
        ImGui::EndGroup();
        ImGui::PopID();
        if (i + 1 < e.images.size()) ImGui::SameLine();
    }
    if (rm >= 0) {
        if (e.images[rm].dbId > 0) m_imgs.removeImage(e.images[rm].dbId);
        e.images.erase(e.images.begin() + rm);
    }
    ImGui::EndChild();
}

// Detalhes do item (Passo 37) e exclusÃµes (Passos 39, 63-66)
void UI::drawItemDetails() {
    Item it;
    if (!m_items.get(m_openItem, it)) {
        m_openItem = -1;
        return;
    }
    auto vals = m_items.getValues(m_openItem);
    auto fields = m_models.getFields(it.modelId);
    Model md;
    m_models.get(it.modelId, md);
    ImGui::TextUnformatted("Detalhes do item");
    ImGui::SameLine(ImGui::GetWindowWidth() - 40);
    if (ImGui::SmallButton("X")) {
        m_openItem = -1;
        return;
    }
    ImGui::Separator();
    // imagem principal
    auto imgs = m_imgs.getImages(m_openItem, false);
    if (!imgs.empty()) {
        TextureHandle tex = m_imgs.texture(imgs[0].id, 512);
        ImGui::Image(tex, ImVec2(280, 200), ImVec2(0,0), ImVec2(1,1), ImVec4(1,1,1,1), ImVec4(0.2f,0.2f,0.25f,1));
        ImGui::SameLine();
    }
    ImGui::BeginGroup();
    for (auto& f : fields) {
        auto itr = vals.find(f.id);
        if (itr == vals.end()) continue;
        const ItemValue& v = itr->second;
        switch (f.type) {
            case FieldType::Text:
                ImGui::Text("%s: %s", f.name.c_str(), v.text.c_str());
                break;
            case FieldType::Integer:
                ImGui::Text("%s: %lld", f.name.c_str(), (long long)v.integer);
                break;
            case FieldType::Decimal:
                ImGui::Text("%s: %.2f", f.name.c_str(), v.real);
                break;
            case FieldType::Boolean:
                ImGui::Text("%s: %s", f.name.c_str(), v.boolean ? "Sim" : "Não");
                break;
            case FieldType::Date:
                ImGui::Text("%s: %s", f.name.c_str(), v.date.c_str());
                break;
        }
    }
    Folder fo;
    if (it.folderId && m_folders.get(*it.folderId, fo))
        ImGui::Text("Pasta: %s", fo.title.empty() ? "Pasta sem título" : fo.title.c_str());
    else ImGui::TextUnformatted("Pasta: —");
    ImGui::EndGroup();
    ImGui::Separator();
    if (ImGui::Button("Editar")) {
        m_editor = ItemEditorState{};
        m_editor.open = true;
        m_editor.editingItemId = it.id;
        m_editor.selectedModelId = it.modelId;
        m_editor.folderId = it.folderId;
        m_editor.values = vals;
    }
    ImGui::SameLine(ImGui::GetWindowWidth() - 110);
    if (ImGui::Button("Excluir")) {
        m_confirm = ConfirmKind::DeleteItem;
        m_confirmTarget = it.id;
        m_confirmSecondary = false;
        ImGui::OpenPopup("Confirmar exclusão");
    }
}

void UI::drawConfirmDialogs() {
    // overlay de detalhes quando um item foi aberto (Passo 37)
    if (m_openItem > 0) drawItemDetails();

    if (m_confirm == ConfirmKind::None && !m_confirmSecondary) return;
    ImGui::OpenPopup("Confirmar exclusão");

    if (ImGui::BeginPopupModal("Confirmar exclusão", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        bool done = false;
        if (m_confirm == ConfirmKind::DeleteItem) {
            if (m_confirmTarget > 0) {
                int nImgs = m_items.imageCount(m_confirmTarget);
                ImGui::Text("Excluir item?");
                ImGui::Text("Esta ação excluirá o item e suas %d imagem(ns).", nImgs);
            } else {
                ImGui::Text("Excluir %d item(ns) selecionado(s)?", (int)m_selectedIds.size());
            }
            if (ImGui::Button("Cancelar")) {
                m_confirm = ConfirmKind::None;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Excluir")) {
                if (m_confirmTarget > 0) {
                    int n;
                    m_items.remove(m_confirmTarget, n);
                    m_openItem = -1;
                } else {
                    for (auto id : m_selectedIds) {
                        int n;
                        m_items.remove(id, n);
                    }
                    m_selectedIds.clear();
                }
                m_dirtyItems = true;
                m_confirm = ConfirmKind::None;
                done = true;
                ImGui::CloseCurrentPopup();
                notify("âœ“ Excluído.");
            }
        } else if (m_confirm == ConfirmKind::DeleteFolder) {
            Folder f;
            m_folders.get(m_confirmTarget, f);
            int64_t n = m_folders.itemCount(m_confirmTarget);
            ImGui::Text("Excluir a pasta \"%s\"?", f.title.c_str());
            ImGui::Text("Esta pasta possui %lld item(s).", (long long)n);
            bool delItems = false;
            ImGui::Checkbox("Excluir tambémt os itens", &delItems);
            if (ImGui::Button("Cancelar")) {
                m_confirm = ConfirmKind::None;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Excluir")) {
                int aff;
                m_folders.remove(m_confirmTarget, delItems, aff);
                if (m_currentFolder == m_confirmTarget) m_currentFolder.reset();
                m_dirtyItems = true;
                m_confirm = ConfirmKind::None;
                done = true;
                ImGui::CloseCurrentPopup();
                notify("âœ“ Pasta excluÃ­da.");
            }
        } else if (m_confirm == ConfirmKind::DeleteModel) {
            int64_t n = m_models.itemCount(m_confirmTarget);
            Model m;
            m_models.get(m_confirmTarget, m);
            ImGui::Text("O modelo \"%s\" possui %lld item(s).", m.name.c_str(), (long long)n);
            ImGui::TextUnformatted("O que deseja fazer?"); // Passo 64
            if (ImGui::Button("Cancelar")) {
                m_confirm = ConfirmKind::None;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Excluir somente modelo")) {
                // itens ficam Ã³rfÃ£os do modelo â€” bloqueado por FK; mover para "sem modelo" nÃ£o existe,
                // entÃ£o exigimos escolha explÃ­cita: (FK ON DELETE CASCADE removeria os itens)
                notify("âš  Use 'Excluir modelo e itens' quando houver itens.", true);
                m_confirm = ConfirmKind::None;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (n == 0 && ImGui::Button("Excluir modelo")) {
                m_models.remove(m_confirmTarget);
                m_confirm = ConfirmKind::None;
                done = true;
                ImGui::CloseCurrentPopup();
                notify("âœ“ Modelo excluído.");
            }
        }
        if (done) m_confirmSecondary = false;
        ImGui::EndPopup();
    }
}

