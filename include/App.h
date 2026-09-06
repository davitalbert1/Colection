#pragma once
#include <string>
#include "imgui.h"

struct GLFWwindow;

class Database;
class ImageManager;
class ModelRepository;
class ItemRepository;
class FolderRepository;
class ImportExport;
class UI;

class App {
public:
    bool initialize(); // cria janela GLFW + contexto OpenGL/ImGui + abre banco
    void run(); // loop principal
    void shutdown(); // encerra tudo corretamente

    Database* database = nullptr;
private:
    GLFWwindow* m_window = nullptr;
    Database* m_db = nullptr;
    ImageManager* m_images = nullptr;
    ModelRepository* m_models = nullptr;
    ItemRepository* m_items = nullptr;
    FolderRepository* m_folders = nullptr;
    ImportExport* m_ie = nullptr;
    UI* m_ui = nullptr;
};
