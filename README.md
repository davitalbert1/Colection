# CollectionArchive — Arquivador de Coleção de Itens

Aplicativo desktop em **C++17** para arquivar e organizar coleções de itens com modelos personalizados, campos personalizados, imagens embutidas no banco, pastas, pesquisa, exportação e importação.

## Stack (decisões registradas)

| Camada | Escolha |
|---|---|
| Interface | **Dear ImGui** (vendor/imgui) |
| Backend gráfico | **GLFW + OpenGL 3** (vendor/glfw) |
| Banco de dados | **SQLite 3** (amalgamação compilada junto, vendor/sqlite) |
| Imagens | **stb_image** (vendor/stb) |
| JSON | **nlohmann/json** (vendor/nlohmann) |

Todas as dependências estão em `vendor/` — não é necessário instalar nada além de um compilador (MinGW-w64) e CMake.

## Como compilar

```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

No Windows/PowerShell, se `mingw32-make` não estiver no PATH, use o shell do MSYS2 (MINGW64) ou informe `-DCMAKE_MAKE_PROGRAM=D:/msys64/mingw64/bin/mingw32-make.exe`.

O executável fica em `build/CollectionArchive.exe`.

## Estrutura

- `src/` — implementação `.cpp` (main, App, Database, Model, Item, Folder, ImageManager, ImportExport, UI)
- `include/` — cabeçalhos `.h`
- `data/` — onde fica o `collection.db` (criado automaticamente)
- `vendor/` — dependências

## Arquitetura

```
UI (UI.cpp)            ← nunca acessa SQLite diretamente
 ↓
Serviços (Model/Item/Folder/ImportExport)
 ↓
Database (Database.cpp) — conexão, transações, prepared statements
 ↓
SQLite
```

## Funcionalidades

- Modelos personalizados (ex.: Livro, Filme) com campos personalizados
- Tipos de campo: Texto, Número inteiro, Número decimal, Checkbox, Data
- Itens criados a partir de modelos, com valores por tipo em colunas próprias
- Imagens ilimitadas por item, armazenadas como BLOB dentro do `.db`
- Reordenação de imagens por drag and drop (posição persistida)
- Pastas com título (opcional), associação item↔pasta
- Grid de itens com miniatura (primeira imagem) e título detectado
- Pesquisa global (título, campos de texto, modelo, pasta)
- Seleção múltipla com ações em lote (exportar / mover / excluir)
- Exportação/importação no formato `.collection` (zip-less: JSON + imagens em um único container)
- Versionamento do banco (`schema_version`) com migrações
- Transações em todas as operações multi-tabela; prepared statements sempre
- Confirmação para ações destrutivas
