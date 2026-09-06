# Sistema de Arquivamento de Coleção de Itens — Guia de Implementação

## 1. Objetivo

Criar um aplicativo desktop em **C++** para arquivar e organizar uma coleção de itens.

O sistema deverá permitir:

- Criar modelos personalizados de itens;
- Criar campos personalizados dentro dos modelos;
- Criar itens a partir dos modelos;
- Adicionar várias imagens aos itens;
- Reordenar imagens por arrastar e soltar;
- Organizar itens em pastas;
- Exibir itens em formato de grid;
- Pesquisar itens;
- Salvar todos os dados em um arquivo `.db`;
- Salvar as imagens dentro do próprio `.db`;
- Exportar itens, pastas e modelos;
- Importar itens, pastas e modelos previamente exportados.

A interface deverá utilizar **Dear ImGui**, possuir tema escuro e ter aparência moderna, limpa e fácil de utilizar.

---

# 2. Regras obrigatórias do projeto

Antes de escrever qualquer código, seguir estas regras.

## 2.1 Linguagem

Utilizar:

```text
C++17 ou superior
```

Preferencialmente utilizar recursos modernos de C++.

Evitar código monolítico.

A aplicação deverá ser dividida em classes e módulos.

---

## 2.2 Organização dos arquivos

Todos os arquivos de implementação `.cpp` devem ficar em:

```text
src/
```

Todos os arquivos de cabeçalho `.h` devem ficar em:

```text
include/
```

### Estrutura obrigatória

```text
CollectionArchive/
│
├── CMakeLists.txt
├── README.md
│
├── src/
│   ├── main.cpp
│   ├── App.cpp
│   ├── Database.cpp
│   ├── Model.cpp
│   ├── Item.cpp
│   ├── Folder.cpp
│   ├── ImageManager.cpp
│   ├── ImportExport.cpp
│   └── UI.cpp
│
├── include/
│   ├── App.h
│   ├── Database.h
│   ├── Model.h
│   ├── Item.h
│   ├── Folder.h
│   ├── ImageManager.h
│   ├── ImportExport.h
│   └── UI.h
│
├── assets/
│
├── data/
│
└── build/
```

### Regra importante

Nunca colocar `.cpp` dentro de `include/`.

Nunca colocar `.h` dentro de `src/`.

---

# 3. Passo 1 — Preparar o ambiente

Primeiro configurar o ambiente de desenvolvimento.

Instalar:

- Compilador C++;
- CMake;
- Git, opcionalmente;
- Dear ImGui;
- SQLite3;
- Biblioteca para carregar imagens;
- Biblioteca JSON.

O projeto deverá ser compilado utilizando CMake.

O objetivo é permitir:

```bash
cmake -S . -B build
cmake --build build
```

---

# 4. Passo 2 — Escolher as bibliotecas

Utilizar:

## Interface gráfica

```text
Dear ImGui
```

## Banco de dados

```text
SQLite3
```

## Imagens

Pode utilizar:

```text
stb_image
```

ou outra biblioteca adequada.

## JSON

Pode utilizar:

```text
nlohmann/json
```

## Backend gráfico

Escolher uma solução adequada, por exemplo:

```text
GLFW + OpenGL
```

ou:

```text
SDL2 + OpenGL
```

A escolha deve ser registrada no `README.md`.

---

# 5. Passo 3 — Criar o CMakeLists.txt

Criar o arquivo:

```text
CMakeLists.txt
```

Ele deverá:

1. Definir a versão mínima do CMake;
2. Definir o projeto;
3. Configurar C++17 ou superior;
4. Adicionar `include/` aos diretórios de inclusão;
5. Adicionar todos os `.cpp` de `src/`;
6. Configurar Dear ImGui;
7. Configurar SQLite;
8. Configurar as bibliotecas auxiliares;
9. Criar o executável.

Não adicionar manualmente arquivos `.cpp` espalhados em vários locais.

A fonte principal deverá estar em:

```text
src/main.cpp
```

---

# 6. Passo 4 — Criar o programa mínimo

Antes de implementar o sistema completo, criar uma janela básica utilizando Dear ImGui.

O primeiro objetivo é conseguir:

```text
Abrir programa
↓
Abrir janela
↓
Inicializar ImGui
↓
Mostrar interface
↓
Fechar janela
↓
Encerrar corretamente
```

Criar:

```text
src/main.cpp
src/App.cpp
include/App.h
```

A classe `App` deverá controlar o ciclo de vida da aplicação.

Exemplo de responsabilidade:

```cpp
class App
{
public:
    bool initialize();
    void run();
    void shutdown();
};
```

Não colocar toda a lógica do programa dentro de `main.cpp`.

---

# 7. Passo 5 — Criar o tema visual

Criar um tema escuro para Dear ImGui.

A interface deverá ter:

- Fundo escuro;
- Painéis escuros;
- Campos ligeiramente mais claros;
- Texto claro;
- Texto secundário em cinza;
- Bordas discretas;
- Espaçamento adequado;
- Botões modernos;
- Cantos e elementos visualmente consistentes.

Evitar excesso de cores.

A paleta deve ser predominantemente escura.

---

# 8. Passo 6 — Criar a estrutura da interface

Criar uma interface principal semelhante a:

```text
┌──────────────────────────────────────────────────────────────┐
│ Coleção                         🔍 Pesquisar       ⚙         │
├──────────────┬───────────────────────────────────────────────┤
│              │                                               │
│ Início       │                                               │
│ Itens        │                 Conteúdo                      │
│ Pastas       │                                               │
│ Modelos      │                                               │
│              │                                               │
│ Configurações│                                               │
│              │                                               │
└──────────────┴───────────────────────────────────────────────┘
```

A estrutura exata pode ser alterada, mas deverá ser limpa e intuitiva.

Criar um sistema de navegação.

Exemplo:

```cpp
enum class Screen
{
    Home,
    Items,
    Folders,
    Models,
    Settings
};
```

---

# 9. Passo 7 — Criar o banco de dados

O programa deverá utilizar SQLite.

O arquivo principal deverá ser:

```text
collection.db
```

Por padrão, salvar em:

```text
data/collection.db
```

O programa deverá:

1. Verificar se o banco existe;
2. Criá-lo caso não exista;
3. Abrir a conexão;
4. Criar as tabelas;
5. Verificar a versão do banco;
6. Executar migrações quando necessário.

Criar:

```text
include/Database.h
src/Database.cpp
```

---

# 10. Passo 8 — Criar o sistema de versionamento do banco

O banco deve possuir uma versão.

Exemplo:

```text
schema_version = 1
```

No futuro poderá existir:

```text
schema_version = 2
schema_version = 3
```

Criar um mecanismo de migração.

Exemplo conceitual:

```cpp
void migrateDatabase();
```

Nunca modificar uma estrutura antiga de banco sem considerar os bancos já existentes dos usuários.

---

# 11. Passo 9 — Criar o sistema de modelos

O conceito central do programa será o **modelo**.

Um modelo representa um tipo de item.

Exemplos:

```text
Livro
Filme
Jogo
Quadrinho
Documento
Colecionável
```

O usuário deverá conseguir criar seus próprios modelos.

---

# 12. Passo 10 — Criar a tabela de modelos

Criar uma tabela semelhante a:

```text
models
----------------
id
name
description
created_at
updated_at
```

O modelo deverá possuir:

```cpp
struct Model
{
    int64_t id;
    std::string name;
    std::string description;
};
```

---

# 13. Passo 11 — Criar campos personalizados

Cada modelo poderá possuir vários campos.

Exemplo:

```text
Modelo: Livro

Título
Autor
Páginas
Terminado de ler
Nota
```

Criar uma tabela semelhante a:

```text
model_fields
----------------
id
model_id
name
type
position
created_at
updated_at
```

---

# 14. Passo 12 — Definir os tipos de campos

Inicialmente implementar:

```text
Texto
Número inteiro
Número decimal
Checkbox
Data
```

Criar um enum:

```cpp
enum class FieldType
{
    Text,
    Integer,
    Decimal,
    Boolean,
    Date
};
```

A arquitetura deve permitir adicionar outros tipos futuramente.

---

# 15. Passo 13 — Criar a tela de modelos

Criar uma tela:

```text
Modelos

┌─────────────────────────────────────────────┐
│ Livro                                       │
│ 5 campos                       [Editar]     │
├─────────────────────────────────────────────┤
│ Jogos                                       │
│ 8 campos                       [Editar]     │
├─────────────────────────────────────────────┤
│ Filmes                                      │
│ 6 campos                       [Editar]     │
└─────────────────────────────────────────────┘

              [ + Novo modelo ]
```

---

# 16. Passo 14 — Criar um novo modelo

Ao clicar em:

```text
+ Novo modelo
```

abrir um editor.

Exemplo:

```text
Criar modelo

Nome:
[ Livro ]

Descrição:
[ Livros da minha coleção ]

Campos:

┌──────────────────────────────────────────────┐
│ Título        │ Texto      │ ↑ ↓ │ 🗑       │
│ Páginas       │ Número     │ ↑ ↓ │ 🗑       │
│ Lido          │ Checkbox   │ ↑ ↓ │ 🗑       │
└──────────────────────────────────────────────┘

[ + Adicionar campo ]

             [Cancelar] [Salvar]
```

---

# 17. Passo 15 — Permitir adicionar campos

O botão:

```text
+ Adicionar campo
```

deve criar um novo campo.

O usuário deve escolher:

```text
Título do campo
Tipo do campo
```

Exemplo:

```text
Nome:
[ Páginas ]

Tipo:
[ Número inteiro ▼ ]
```

---

# 18. Passo 16 — Permitir remover campos

Cada campo deverá possuir uma ação de exclusão.

Exemplo:

```text
Páginas     Número inteiro     [Excluir]
```

Antes de remover um campo que já possui valores em itens, mostrar uma confirmação.

Exemplo:

```text
Este campo possui dados em 37 itens.

Ao remover o campo, esses dados serão perdidos.

[Cancelar] [Excluir campo]
```

Nunca excluir silenciosamente dados do usuário.

---

# 19. Passo 17 — Permitir alterar campos

O usuário deve poder:

- Alterar o título;
- Alterar o tipo;
- Alterar a ordem.

Exemplo:

```text
Título
Páginas
Autor
Lido
```

Poder reorganizar para:

```text
Título
Autor
Páginas
Lido
```

A posição deve ser salva no banco.

---

# 20. Passo 18 — Cuidado ao alterar o tipo de campo

Se o usuário tentar mudar:

```text
Páginas
Número inteiro
```

para:

```text
Páginas
Texto
```

o sistema deve verificar se os dados existentes podem ser convertidos.

Se não puderem, mostrar um aviso.

Exemplo:

```text
Alguns valores não podem ser convertidos.

Deseja continuar?

[Cancelar] [Continuar]
```

---

# 21. Passo 19 — Criar o sistema de itens

Criar:

```text
include/Item.h
src/Item.cpp
```

Um item deverá possuir:

```cpp
struct Item
{
    int64_t id;
    int64_t modelId;
    std::optional<int64_t> folderId;
    std::string createdAt;
    std::string updatedAt;
};
```

O item deverá estar relacionado a um modelo.

---

# 22. Passo 20 — Criar a tabela de itens

Criar uma tabela semelhante a:

```text
items
----------------
id
model_id
folder_id
created_at
updated_at
```

A relação:

```text
Item → Modelo
```

é obrigatória.

A relação:

```text
Item → Pasta
```

pode ser opcional.

---

# 23. Passo 21 — Criar os valores dos campos

Os valores dos campos devem ser armazenados separadamente.

Criar tabela semelhante a:

```text
item_values
----------------
id
item_id
field_id
value_text
value_integer
value_real
value_boolean
value_date
```

Cada valor deverá utilizar a coluna adequada ao tipo do campo.

Exemplo:

```text
Páginas → value_integer
Preço → value_real
Título → value_text
Lido → value_boolean
Data → value_date
```

Isso permitirá realizar consultas posteriormente.

---

# 24. Passo 22 — Criar a tela de novo item

Ao clicar em:

```text
+ Novo item
```

primeiro mostrar a seleção do modelo.

Exemplo:

```text
Novo item

Modelo:
[ Livro ▼ ]
```

Depois que o usuário selecionar:

```text
Livro
```

mostrar automaticamente os campos cadastrados no modelo.

---

# 25. Passo 23 — Exemplo de criação de um livro

Se o modelo for:

```text
Livro
```

e tiver:

```text
Título       Texto
Autor        Texto
Páginas      Número inteiro
Lido         Checkbox
```

o editor deverá mostrar:

```text
Título:
[ O Hobbit ]

Autor:
[ J. R. R. Tolkien ]

Páginas:
[ 310 ]

Lido:
[ ✓ ]
```

Não criar esses campos diretamente no código.

Eles deverão vir do modelo salvo no banco.

---

# 26. Passo 24 — Criar sistema de pastas

Criar:

```text
include/Folder.h
src/Folder.cpp
```

Uma pasta deverá possuir:

```cpp
struct Folder
{
    int64_t id;
    std::string title;
};
```

Criar tabela:

```text
folders
----------------
id
title
created_at
updated_at
```

---

# 27. Passo 25 — Criar e editar pastas

O usuário deverá poder:

```text
+ Nova pasta
```

Exemplo:

```text
Nome da pasta:

[ Livros favoritos ]

[Cancelar] [Criar]
```

O título poderá ficar vazio.

Também deve ser possível editar o título posteriormente.

---

# 28. Passo 26 — Associar itens às pastas

Ao criar ou editar um item, mostrar:

```text
Pasta:
[ Livros favoritos ▼ ]
```

Também deverá existir uma opção:

```text
Sem pasta
```

O usuário poderá mover itens entre pastas.

---

# 29. Passo 27 — Criar sistema de imagens

Criar:

```text
include/ImageManager.h
src/ImageManager.cpp
```

Cada item poderá possuir **quantas imagens forem necessárias**.

Exemplo:

```text
Item
├── imagem 1
├── imagem 2
├── imagem 3
├── imagem 4
└── imagem 5
```

Não impor um limite artificial de imagens por item.

---

# 30. Passo 28 — Criar tabela de imagens

Criar tabela semelhante a:

```text
images
----------------
id
item_id
position
filename
mime_type
data
```

A coluna:

```text
data
```

deverá armazenar os bytes da imagem como `BLOB`.

Assim, a imagem ficará dentro do `.db`.

---

# 31. Passo 29 — Adicionar imagens

No editor do item criar:

```text
Imagens

[ + Adicionar imagens ]
```

Permitir selecionar:

- Uma imagem;
- Várias imagens.

Depois de selecionadas, mostrar miniaturas.

Exemplo:

```text
┌────────┐ ┌────────┐ ┌────────┐
│ imagem │ │ imagem │ │ imagem │
│   1    │ │   2    │ │   3    │
└────────┘ └────────┘ └────────┘

[ + Adicionar imagens ]
```

---

# 32. Passo 30 — Permitir remover imagens

Cada imagem deverá possuir uma ação de remoção.

Exemplo:

```text
┌────────────┐
│            │
│   imagem   │
│            │
├────────────┤
│  [Excluir] │
└────────────┘
```

Antes de excluir, pode ser utilizada confirmação dependendo do contexto.

---

# 33. Passo 31 — Implementar drag and drop das imagens

O usuário deverá poder arrastar imagens para modificar a ordem.

Exemplo:

```text
Antes:

[ Capa ] [ Verso ] [ Página ]

Depois:

[ Página ] [ Capa ] [ Verso ]
```

A nova ordem deverá ser salva na coluna:

```text
images.position
```

A interface deve utilizar os recursos de drag-and-drop do Dear ImGui sempre que possível.

---

# 34. Passo 32 — Gerenciar texturas

Não criar uma textura gráfica nova a cada frame.

Criar cache de imagens.

Exemplo conceitual:

```cpp
class ImageManager
{
public:
    TextureHandle loadImage(int64_t imageId);
    TextureHandle loadThumbnail(int64_t imageId);

    void unload(int64_t imageId);
    void clearCache();
};
```

O sistema deverá:

- Carregar imagens quando necessário;
- Reutilizar texturas;
- Criar miniaturas;
- Evitar carregamentos repetidos;
- Liberar recursos quando necessário.

---

# 35. Passo 33 — Visualização dos itens

Os itens de uma pasta deverão ser exibidos em um grid.

Exemplo:

```text
Livros

┌─────────────┐ ┌─────────────┐ ┌─────────────┐
│             │ │             │ │             │
│    capa     │ │    capa     │ │    capa     │
│             │ │             │ │             │
├─────────────┤ ├─────────────┤ ├─────────────┤
│ O Hobbit    │ │ Duna        │ │ Fundação    │
└─────────────┘ └─────────────┘ └─────────────┘
```

---

# 36. Passo 34 — Primeira imagem

Quando um item possuir imagens:

```text
imagem[0]
```

deverá ser utilizada como imagem principal no grid.

Se não possuir imagens:

```text
[ Sem imagem ]
```

mostrar um placeholder.

---

# 37. Passo 35 — Título no grid

O sistema deve tentar encontrar um campo apropriado para representar o título.

Por exemplo:

```text
Título
Nome
Nome do item
```

Se existir:

```text
O Hobbit
```

mostrar esse texto.

Se não existir um campo de título:

```text
Livro #123
```

ou outra identificação adequada.

A regra deve ser documentada e consistente.

---

# 38. Passo 36 — Visualização de pastas

A pasta deve mostrar seu título quando houver.

Exemplo:

```text
Livros favoritos

┌─────────┐ ┌─────────┐ ┌─────────┐
│ imagem  │ │ imagem  │ │ imagem  │
│ O Hobbit│ │ Duna    │ │ Fundação│
└─────────┘ └─────────┘ └─────────┘
```

Se a pasta não tiver título, utilizar uma identificação como:

```text
Pasta sem título
```

---

# 39. Passo 37 — Tela de detalhes do item

Ao clicar em um item, abrir seus detalhes.

Exemplo:

```text
┌─────────────────────────────────────────────┐
│ O Hobbit                                    │
├─────────────────────────────────────────────┤
│                                             │
│             IMAGEM PRINCIPAL                │
│                                             │
├─────────────────────────────────────────────┤
│ Imagens                                     │
│                                             │
│ [img] [img] [img] [img]                    │
│                                             │
│ Título: O Hobbit                            │
│ Autor: J. R. R. Tolkien                     │
│ Páginas: 310                                │
│ Lido: Sim                                   │
│                                             │
│ Pasta: Livros                               │
│                                             │
│ [Editar]                         [Excluir]  │
└─────────────────────────────────────────────┘
```

---

# 40. Passo 38 — Editar itens

Ao clicar em:

```text
Editar
```

abrir o mesmo editor utilizado para criação.

Permitir alterar:

- Valores dos campos;
- Pasta;
- Imagens;
- Ordem das imagens.

Não permitir alterar o modelo de um item de maneira silenciosa.

Se futuramente for implementada a troca de modelo, ela deverá possuir um processo específico de conversão.

---

# 41. Passo 39 — Exclusão de itens

Ao excluir um item:

1. Pedir confirmação;
2. Excluir os valores;
3. Excluir as imagens;
4. Excluir o item;
5. Utilizar transação.

Exemplo:

```text
Excluir item?

Esta ação excluirá o item e suas 8 imagens.

[Cancelar] [Excluir]
```

---

# 42. Passo 40 — Implementar pesquisa

Criar pesquisa global.

Exemplo:

```text
🔍 [ Tolkien                         ]
```

Pesquisar pelo menos:

- Título;
- Nome do modelo;
- Campos de texto;
- Pasta.

No futuro, permitir filtros por tipos de dados.

---

# 43. Passo 41 — Implementar seleção múltipla

O grid deverá permitir selecionar vários itens.

Exemplo:

```text
[✓] O Hobbit
[✓] Duna
[ ] Fundação
```

Quando houver seleção:

```text
2 itens selecionados

[Exportar]
[Mover]
[Excluir]
```

---

# 44. Passo 42 — Utilizar transações SQLite

Operações que modificam várias tabelas deverão utilizar transações.

Exemplo ao salvar um item:

```text
BEGIN TRANSACTION

Criar item
Salvar valores
Salvar imagens
Salvar posições
Atualizar pasta

COMMIT
```

Se qualquer operação falhar:

```text
ROLLBACK
```

Isso evita dados parcialmente salvos.

---

# 45. Passo 43 — Criar sistema de exportação

O usuário deverá conseguir exportar:

- Um item;
- Vários itens;
- Uma pasta;
- Várias pastas;
- Todos os itens;
- Um modelo;
- Vários modelos;
- Toda a coleção.

---

# 46. Passo 44 — Criar formato de exportação

Criar um formato próprio, por exemplo:

```text
.collection
```

Recomenda-se utilizar um arquivo compactado contendo:

```text
manifest.json
data.json
images/
    image_001.bin
    image_002.bin
    image_003.bin
```

O formato deverá conter uma versão.

Exemplo:

```json
{
    "format": "collection",
    "version": 1
}
```

---

# 47. Passo 45 — O que preservar na exportação

Ao exportar itens, preservar:

- Modelo;
- Campos;
- Valores;
- Pasta;
- Imagens;
- Ordem das imagens;
- Metadados necessários.

Ao exportar modelos, preservar:

- Nome;
- Descrição;
- Campos;
- Tipos;
- Ordem dos campos.

---

# 48. Passo 46 — Exportar pastas

Ao exportar uma pasta, incluir:

```text
Pasta
+
Itens da pasta
+
Dados dos itens
+
Imagens
```

Preservar a relação:

```text
Item → Pasta
```

---

# 49. Passo 47 — Exportar todos os dados

Criar opção:

```text
Exportar toda a coleção
```

Deve incluir:

```text
Todos os modelos
Todas as pastas
Todos os itens
Todas as imagens
Todos os relacionamentos
```

O arquivo exportado deverá ser suficiente para reconstruir a coleção.

---

# 50. Passo 48 — Criar sistema de importação

Permitir:

```text
Importar
```

Fluxo:

```text
Selecionar arquivo
       ↓
Validar arquivo
       ↓
Ler manifest
       ↓
Verificar versão
       ↓
Ler dados
       ↓
Detectar conflitos
       ↓
Mostrar resumo
       ↓
Confirmar
       ↓
Importar
```

---

# 51. Passo 49 — Mostrar resumo antes da importação

Antes de importar, mostrar:

```text
Importação

Modelos: 3
Pastas: 5
Itens: 128
Imagens: 342

[Cancelar] [Importar]
```

Nunca importar imediatamente sem permitir ao usuário confirmar.

---

# 52. Passo 50 — Resolver conflitos

Prever conflitos como:

```text
Modelo "Livro" já existe.
```

Oferecer opções:

```text
○ Usar modelo existente
○ Criar cópia
○ Substituir
```

Não sobrescrever dados silenciosamente.

---

# 53. Passo 51 — Importar imagens

As imagens exportadas deverão ser lidas e inseridas novamente no SQLite como:

```text
BLOB
```

Depois disso, o arquivo importado poderá ser removido sem quebrar os itens.

---

# 54. Passo 52 — Validar arquivos de importação

Antes de importar:

- Verificar extensão;
- Verificar estrutura;
- Verificar `manifest.json`;
- Verificar versão;
- Verificar integridade;
- Verificar campos obrigatórios;
- Verificar imagens;
- Verificar IDs;
- Verificar referências.

Um arquivo inválido não deve corromper o banco.

---

# 55. Passo 53 — Fazer a importação em uma transação

A importação completa deverá ocorrer dentro de uma transação.

Exemplo:

```text
BEGIN TRANSACTION

Importar modelos
Importar campos
Importar pastas
Importar itens
Importar valores
Importar imagens

COMMIT
```

Se ocorrer qualquer problema:

```text
ROLLBACK
```

Assim, a importação não deixará metade dos dados no banco.

---

# 56. Passo 54 — Sistema de erros

Criar mensagens claras.

Exemplos:

```text
✓ Item salvo com sucesso.
```

```text
✓ Importação concluída.
```

```text
⚠ A imagem não pôde ser carregada.
```

```text
✕ Não foi possível abrir o banco de dados.
```

Evitar mostrar apenas mensagens técnicas como:

```text
SQLITE_ERROR 19
```

Quando possível, apresentar também uma explicação compreensível.

---

# 57. Passo 55 — Segurança do banco

Utilizar:

```text
Prepared Statements
```

Nunca construir SQL desta maneira:

```cpp
"SELECT * FROM items WHERE title = '" + title + "'"
```

Utilizar parâmetros.

Isso evita problemas de SQL injection e erros causados por caracteres especiais.

---

# 58. Passo 56 — Separar interface e banco

A UI não deverá acessar SQLite diretamente.

Evitar:

```cpp
void drawScreen()
{
    sqlite3_exec(...);
}
```

Preferir uma arquitetura semelhante a:

```text
UI
 ↓
Service
 ↓
Repository
 ↓
Database
 ↓
SQLite
```

Exemplo:

```cpp
auto items = itemService.getItemsByFolder(folderId);
```

---

# 59. Passo 57 — Separar responsabilidades

Responsabilidades sugeridas:

```text
App
    Ciclo de vida da aplicação

Database
    Conexão SQLite
    Transações
    Prepared statements

Model
    Modelos e campos

Item
    Itens e valores

Folder
    Pastas

ImageManager
    Imagens e texturas

ImportExport
    Exportação e importação

UI
    Interface gráfica
```

---

# 60. Passo 58 — Criar estruturas de dados

Exemplo:

```cpp
struct Model
{
    int64_t id;
    std::string name;
    std::string description;
};

enum class FieldType
{
    Text,
    Integer,
    Decimal,
    Boolean,
    Date
};

struct ModelField
{
    int64_t id;
    int64_t modelId;
    std::string name;
    FieldType type;
    int position;
};

struct Folder
{
    int64_t id;
    std::string title;
};

struct Item
{
    int64_t id;
    int64_t modelId;
    std::optional<int64_t> folderId;
};

struct ItemImage
{
    int64_t id;
    int64_t itemId;
    int position;

    std::string filename;
    std::string mimeType;

    std::vector<std::uint8_t> data;
};
```

---

# 61. Passo 59 — Separar estado temporário da interface

O estado do editor não deve ser diretamente o banco de dados.

Exemplo:

```cpp
struct ItemEditorState
{
    bool isOpen = false;

    int64_t editingItemId = -1;

    int64_t selectedModelId = -1;

    std::vector<PendingImage> images;
};
```

O usuário pode editar os dados.

Somente ao clicar:

```text
Salvar
```

os dados deverão ser enviados ao banco.

Se clicar:

```text
Cancelar
```

as alterações temporárias deverão ser descartadas.

---

# 62. Passo 60 — Criar cache de dados

Evitar consultar o banco desnecessariamente a cada frame do ImGui.

Errado:

```text
Frame 1 → SELECT
Frame 2 → SELECT
Frame 3 → SELECT
Frame 4 → SELECT
...
```

Preferir:

```text
Abrir tela
↓
Carregar dados
↓
Guardar em memória
↓
Atualizar somente quando necessário
```

---

# 63. Passo 61 — Otimizar o grid

O grid poderá conter centenas ou milhares de itens.

Não carregar todas as imagens originais simultaneamente.

Utilizar:

- Miniaturas;
- Cache;
- Carregamento sob demanda;
- Liberação de recursos;
- Paginação ou virtualização, se necessário.

---

# 64. Passo 62 — Atalhos

Adicionar atalhos úteis:

```text
Ctrl + N → Novo item
Ctrl + F → Pesquisar
Ctrl + S → Salvar
Esc      → Fechar janela/diálogo
Delete   → Excluir seleção
```

Não implementar atalhos que conflitem com edição de texto.

---

# 65. Passo 63 — Confirmações

Ações destrutivas devem pedir confirmação.

Exemplos:

```text
Excluir item
Excluir pasta
Excluir modelo
Excluir campo
Excluir imagem
Substituir dados durante importação
```

---

# 66. Passo 64 — Excluir modelo

Antes de excluir um modelo:

```text
Este modelo possui 27 itens.

O que deseja fazer?

[Cancelar]
[Excluir somente modelo]
[Excluir modelo e itens]
```

Se a arquitetura não permitir excluir um modelo que possui itens, bloquear a operação e explicar o motivo.

Nunca apagar dados inesperadamente.

---

# 67. Passo 65 — Criar uma página inicial

A página inicial pode mostrar:

```text
Coleção

Modelos: 8
Pastas: 14
Itens: 1.284
Imagens: 3.821

Últimos itens adicionados
```

Também pode conter:

```text
[ + Novo item ]
[ + Nova pasta ]
[ Gerenciar modelos ]
[ Importar ]
[ Exportar ]
```

---

# 68. Passo 66 — Implementar a tela de configurações

Criar configurações básicas.

Por exemplo:

```text
Configurações

Banco de dados:
data/collection.db

Tema:
Escuro

Confirmação antes de excluir:
[ ✓ ]

Cache de imagens:
[ ✓ ]

[Salvar]
```

O tema escuro deve ser o padrão.

---

# 69. Passo 67 — Testar o banco

Criar testes para:

- Criar banco;
- Criar modelo;
- Criar campo;
- Alterar campo;
- Excluir campo;
- Criar item;
- Salvar valores;
- Criar pasta;
- Mover item;
- Salvar imagens;
- Reordenar imagens;
- Excluir item;
- Exportar;
- Importar.

---

# 70. Passo 68 — Testar casos extremos

Testar:

```text
Modelo sem campos
Modelo com muitos campos
Item sem imagem
Item com uma imagem
Item com muitas imagens
Pasta sem título
Pasta vazia
Muitos itens
Imagem muito grande
Imagem inválida
Banco inexistente
Banco corrompido
Arquivo de importação inválido
Arquivo de importação de versão diferente
Campos vazios
Títulos muito grandes
Caracteres especiais
```

---

# 71. Passo 69 — Testar persistência

Fechar o programa e abrir novamente.

Verificar se:

- Modelos continuam presentes;
- Campos continuam presentes;
- Itens continuam presentes;
- Pastas continuam presentes;
- Imagens continuam presentes;
- Ordem das imagens continua correta;
- Relações entre itens e pastas continuam corretas.

---

# 72. Passo 70 — Testar exportação e importação

Criar uma coleção de teste:

```text
3 modelos
5 pastas
20 itens
50 imagens
```

Exportar tudo.

Depois:

1. Fechar o programa;
2. Criar uma nova base;
3. Importar o arquivo;
4. Conferir todos os dados.

O resultado deverá ser equivalente ao original.

---

# 73. Passo 71 — Melhorar a UX

Depois que todas as funções estiverem funcionando, revisar a interface.

Verificar:

- Espaçamento;
- Tamanho dos botões;
- Hierarquia visual;
- Legibilidade;
- Navegação;
- Mensagens;
- Diálogos;
- Grid;
- Miniaturas;
- Drag and drop.

A interface deve parecer um aplicativo finalizado, não uma demonstração técnica.

---

# 74. Passo 72 — Não implementar tudo de uma vez

O desenvolvimento deve seguir etapas.

A IA programadora deve implementar **uma etapa por vez**.

Após cada etapa:

1. Compilar;
2. Corrigir erros;
3. Executar;
4. Testar;
5. Só então avançar.

Não gerar centenas de arquivos e milhares de linhas de código de uma única vez.

---

# 75. Ordem recomendada de desenvolvimento

Seguir exatamente esta ordem:

```text
1. Criar projeto CMake
        ↓
2. Configurar C++
        ↓
3. Configurar Dear ImGui
        ↓
4. Criar janela
        ↓
5. Criar tema escuro
        ↓
6. Criar navegação
        ↓
7. Configurar SQLite
        ↓
8. Criar Database
        ↓
9. Criar sistema de migração
        ↓
10. Criar Model
        ↓
11. Criar ModelField
        ↓
12. Criar tela de modelos
        ↓
13. Criar editor de campos
        ↓
14. Criar Item
        ↓
15. Criar ItemValue
        ↓
16. Criar Folder
        ↓
17. Criar criação/edição de itens
        ↓
18. Criar imagens
        ↓
19. Criar BLOBs
        ↓
20. Criar ImageManager
        ↓
21. Criar drag and drop
        ↓
22. Criar grid
        ↓
23. Criar tela de detalhes
        ↓
24. Criar pesquisa
        ↓
25. Criar seleção múltipla
        ↓
26. Criar exportação
        ↓
27. Criar importação
        ↓
28. Criar resolução de conflitos
        ↓
29. Criar testes
        ↓
30. Otimizar
        ↓
31. Polir interface
```

---

# 76. Regra para a IA programadora

Ao implementar este projeto, seguir estas regras:

## Regra 1

Não alterar a arquitetura sem explicar o motivo.

## Regra 2

Não colocar código `.cpp` dentro de `include/`.

## Regra 3

Não colocar código `.h` dentro de `src/`.

## Regra 4

Não colocar toda a lógica em `main.cpp`.

## Regra 5

A interface não deve acessar SQLite diretamente.

## Regra 6

Não armazenar imagens somente como caminhos de arquivos.

As imagens devem ser armazenadas no `.db`.

## Regra 7

Não limitar artificialmente a quantidade de imagens por item.

## Regra 8

A ordem das imagens deve ser persistida.

## Regra 9

Os campos dos itens devem vir do modelo selecionado.

## Regra 10

Não criar campos específicos de `Livro`, `Filme`, `Jogo` etc. diretamente no código.

## Regra 11

A exclusão de dados deve exigir confirmação quando houver risco de perda.

## Regra 12

Operações que alteram várias tabelas devem usar transações.

## Regra 13

Utilizar prepared statements.

## Regra 14

Não executar consultas ao banco a cada frame do ImGui sem necessidade.

## Regra 15

Após cada etapa, compilar e corrigir os erros antes de continuar.

---

# 77. Critérios de conclusão

O projeto será considerado funcional quando for possível realizar todo o fluxo abaixo:

```text
Abrir programa
      ↓
Criar modelo "Livro"
      ↓
Adicionar campo "Título" → Texto
      ↓
Adicionar campo "Páginas" → Número
      ↓
Adicionar campo "Lido" → Checkbox
      ↓
Salvar modelo
      ↓
Criar pasta "Livros"
      ↓
Criar item
      ↓
Selecionar modelo "Livro"
      ↓
Preencher os campos
      ↓
Adicionar várias imagens
      ↓
Reordenar imagens por drag and drop
      ↓
Escolher pasta "Livros"
      ↓
Salvar item
      ↓
Visualizar item no grid
      ↓
Abrir detalhes
      ↓
Editar item
      ↓
Pesquisar item
      ↓
Selecionar itens
      ↓
Exportar
      ↓
Importar
      ↓
Confirmar que todos os dados e imagens continuam funcionando
```

---

# 78. Resultado final esperado

O resultado deverá ser um aplicativo desktop de gerenciamento de coleções com:

- C++;
- C++17 ou superior;
- CMake;
- Dear ImGui;
- SQLite;
- Arquivos `.cpp` exclusivamente em `src/`;
- Arquivos `.h` exclusivamente em `include/`;
- Tema escuro;
- Interface moderna;
- Interface limpa;
- Modelos personalizados;
- Campos personalizados;
- Tipos de dados diferentes;
- Criação de itens baseada em modelos;
- Quantidade ilimitada de imagens por item;
- Drag and drop para ordenar imagens;
- Imagens armazenadas no `.db`;
- Pastas;
- Títulos de pastas;
- Grid de itens;
- Primeira imagem como miniatura;
- Título do item quando disponível;
- Pesquisa;
- Seleção múltipla;
- Exportação;
- Importação;
- Detecção de conflitos;
- Transações;
- Validação;
- Versionamento do banco;
- Tratamento de erros;
- Arquitetura modular.

---

# 79. Instrução final para a IA programadora

**Implemente este projeto seguindo o documento passo a passo.**

Não pule diretamente para a implementação completa.

Comece pelo **Passo 1** e avance sequencialmente.

Para cada passo:

1. Explique brevemente o que será feito;
2. Crie ou altere somente os arquivos necessários;
3. Mostre o código completo dos arquivos novos ou alterados;
4. Explique onde cada arquivo deve ser colocado;
5. Garanta que `.cpp` esteja em `src/`;
6. Garanta que `.h` esteja em `include/`;
7. Compile o projeto;
8. Corrija os erros encontrados;
9. Verifique se o passo funciona;
10. Somente depois avance para o próximo passo.

Não substituir funcionalidades existentes por versões simplificadas sem autorização.

Não remover funcionalidades já implementadas.

Quando uma decisão de arquitetura não estiver especificada, escolha uma solução simples, modular, extensível e adequada para um aplicativo desktop C++.

Priorize primeiro **funcionalidade e estabilidade**, depois **otimização**, e por último **polimento visual**.

O resultado final deve ser um aplicativo real e utilizável, e não apenas um protótipo visual.
