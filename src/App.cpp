// Ciclo de vida da aplicação (Passo 4) — GLFW + OpenGL3 + Dear ImGui
#include "App.h"
#include "Database.h"
#include "ImageManager.h"
#include "Model.h"
#include "Item.h"
#include "ImportExport.h"
#include "UI.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cstdio>

static void glfwErrorCb(int error, const char* desc) {
    std::fprintf(stderr, "GLFW erro %d: %s\n", error, desc);
}

bool App::initialize() {
    glfwSetErrorCallback(glfwErrorCb);
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    m_window = glfwCreateWindow(1280, 800, "CollectionArchive — Arquivador de Coleção", nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return false;

    // Camadas de serviço (Passo 56/57 — UI nunca fala com SQLite direto)
    m_db = new Database();
    if (!m_db->open("data/collection.db")) {
        std::fprintf(stderr, "Não foi possível abrir o banco de dados: %s\n", m_db->lastError());
        return false;
    }
    m_images  = new ImageManager(*m_db);
    m_models  = new ModelRepository(*m_db);
    m_items   = new ItemRepository(*m_db);
    m_folders = new FolderRepository(*m_db);
    m_ie      = new ImportExport(*m_db);
    m_ui      = new UI(*m_db, *m_images, *m_models, *m_items, *m_folders, *m_ie);

    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    m_ui->applyDarkTheme();

    return true;
}

void App::run() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        m_ui->draw();
        ImGui::Render();

        int w, h;
        glfwGetFramebufferSize(m_window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.09f, 0.09f, 0.11f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_window);
    }
}

void App::shutdown() {
    if (m_images) m_images->clearCache();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (m_window) glfwDestroyWindow(m_window);
    glfwTerminate();
    delete m_ui;
    delete m_ie;
    delete m_folders;
    delete m_items;
    delete m_models;
    delete m_images;
    delete m_db;
    m_ui = nullptr;
    m_ie = nullptr;
}
