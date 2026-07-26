# Trabalho M1 - Passeio do Cavalo

Trabalho de Complexidade de Algoritmos - dois algoritmos para o Passeio do Cavalo, em um único programa (`passeio_cavalo.cpp`).

Um cavalo em um tabuleiro de xadrez, seguindo as regras do jogo, precisa passar por todas as casas exatamente uma vez em movimentos consecutivos.

Detalhes em: https://en.wikipedia.org/wiki/Knight%27s_tour

## Compilando

Precisa de um compilador com suporte a **C++17**. O arquivo fica dentro da pasta `M1/` (navegue até ela usando o comando **cd** se necessário).

**Se tiver g++ (MinGW/MSYS2/WSL):**
```
cd M1
g++ -std=c++17 -O2 -o passeio_cavalo passeio_cavalo.cpp
```

**Se só tiver o `cl.exe` (Visual Studio / Build Tools) — comum no PC da faculdade:**

Abra o "x64 Native Tools Command Prompt" (vem com o Visual Studio) e rode:
```
cd M1
cl /EHsc /std:c++17 /O2 passeio_cavalo.cpp /Fe:passeio_cavalo.exe
```

Se abrir um prompt comum (não o "Native Tools"), o `cl` não vai ser reconhecido — precisa carregar as variáveis de ambiente primeiro com o `vcvars64.bat` (fica em `...\VC\Auxiliary\Build\`) antes de compilar.

**Se der erro de flag `/std:c++17` não reconhecida:** a versão do MSVC instalada é antiga demais. Tente rebaixar para `/std:c++14` — o código não usa nada exclusivo do C++17, então deve compilar sem alterações.

**Caminho recomendado caso nada funcione:**
```
- Abra o Developer Command Prompt for VS2022
- Navegue até a pasta do projeto usando cd
- Use code . para abrir o vscode na pasta
```
Ao abrir o VSCode através do Developer Command Prompt, você possui acesso ao cl.exe sem precisar das variaveis de ambiente, facilitando o processo de build (CTRL + SHIFT + B)

## Executar

```
cd M1
.\passeio_cavalo.exe
```

O programa mostra um menu:
1. Executar Heurística Aleatória
2. Executar Heurística Parede (Warnsdorff)
3. Executar teste completo (gera arquivo csv)
4. Sair

Nas opções 1 e 2, informe a posição inicial do cavalo (linha e coluna, de 0 a 7) quando solicitado.
