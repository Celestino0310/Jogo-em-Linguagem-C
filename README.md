# Althea 🎮

> Um jogo de plataforma 2D desenvolvido em **C puro** utilizando **OpenGL + GLUT**, inspirado em *Celeste* e *Super Meat Boy*.

<br>

## 📖 Sobre o Projeto

**Althea** é um jogo de plataforma 2D desenvolvido como projeto acadêmico, com foco em programação gráfica de baixo nível utilizando OpenGL imediato.

O jogador explora fases 2D pulando entre plataformas, evitando obstáculos e coletando moedas até alcançar o final de cada mapa.

O projeto foi construído sem engine, utilizando apenas lógica própria em C.

---

## 🕹️ Gameplay

O jogo conta com:

- 4 fases
- Plataformas e colisão AABB
- Espinhos como obstáculo
- Sistema de moedas
- Física básica de movimento
- Efeitos visuais (neve e brilho)
- Sistema de menus
- Sistema de screenshots
- Áudio e música ambiente

---

## 🎮 Controles

- **A / D** → Movimentação lateral  
- **W** → Interação / ação vertical  
- **Espaço** → Pulo  
- **Q** → Dash  
- **O** → Screenshot  
- **P** → Pause  

---

## ⚙️ Como Rodar o Jogo

### 📌 Requisitos

- Compilador C (Dev-C++, MinGW ou similar)
- OpenGL (geralmente já incluso no Windows)
- GLUT (glut32)
- BASS Audio Library (DLL inclusa no projeto)

---

## 🛠️ Configuração no Dev-C++ (IMPORTANTE)

O Dev-C++ NÃO configura automaticamente includes e bibliotecas, então você precisa fazer isso manualmente.

### 🔧 Passo a passo

1. Abra o projeto no Dev-C++
2. Vá em:
    Project → Project Options
3. Na aba Parameters, configure:

### 📌 Includes (Compiler)
Adicione o caminho do include do projeto:
-IC:\SEU_CAMINHO_DO_PROJETO\include
### 📌 Libraries (Linker)

Adicione as bibliotecas necessárias:
-lopengl32
-lglu32
-lglut32
-lbass
---

### 📌 DLLs obrigatórias

Coloque as DLLs na mesma pasta do executável:

- glut32.dll  
- bass.dll  

---

## ▶️ Compilação e Execução

### No Dev-C++

1. Compile o projeto inteiro (Build All)
2. Execute o main.c ou o .exe gerado
3. Se der erro, verifique includes e DLLs

---

### ▶️ Execução direta

Se já compilado:
/build/Althea.exe

## 🧠 Como o Jogo Funciona (Arquitetura)

O jogo é dividido em módulos:

- **main.c** → inicialização do OpenGL e loop principal
- **game.c** → lógica principal do jogo
- **mapa.c** → construção das fases e blocos
- **menu.c** → telas de menu e navegação
- **espinho.c** → renderização e colisão dos espinhos
- **Screenshot.c** → captura de tela
- **GalleryScreenshot.c** → leitura e organização de imagens salvas

### 🧩 Renderização

O jogo usa OpenGL imediato:

- `glBegin(GL_QUADS)` → blocos e plataformas
- `glBegin(GL_TRIANGLES)` → efeitos visuais
- `glVertex2f()` → posicionamento dos elementos

---

## ✨ Sistemas do Jogo

### 🌨️ Neve
Sistema procedural baseado em partículas com:
- posição aleatória
- velocidade individual
- atualização contínua por frame

---

### ⚠️ Espinhos
Construídos manualmente com células em grade:

- renderização por blocos
- colisão AABB
- desenho com QUADS e TRIANGLES

---

### 🪙 Moedas
- animação com seno (`sin(time)`)
- efeito de flutuação
- renderização dinâmica

---

### 📸 Screenshot
- captura do framebuffer
- salvamento automático na pasta `/screenshots`

---

## 🎨 Inspiração

- Celeste
- Super Meat Boy
- Jogos clássicos de plataforma 2D

---

## 👥 Créditos

### 👑 Gabriel Celestino
Direção criativa, world design, mapas, espinhos, neve, áudio e integração geral do jogo.

---

### 🎞️ Diogo
Sprites, animações do personagem e integração visual com stb_image.

---

### 🖥️ Igor
Menus, UI/UX e navegação entre telas.

---

### ⚙️ Gabriel Neves
Física do personagem, movimentação e colisões do jogador.

---

### 💾 Thiago Teodoro
Sistema de arquivos, screenshots e ranking.

---

## 🧠 Conceitos Utilizados

- Linguagem C
- OpenGL imediato
- Estruturas (struct)
- Ponteiros
- Física básica
- Colisão AABB
- Programação modular
- Manipulação de arquivos
- Sistemas procedurais
- Loop de jogo

---

## ❤️ Considerações Finais

**Althea** é um projeto acadêmico desenvolvido inteiramente em C puro, sem engines, focado em aprendizado de gráficos, física e arquitetura de jogos.

Mais do que um jogo, é um exercício completo de programação de baixo nível com OpenGL.
