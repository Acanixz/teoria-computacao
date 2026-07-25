// Este código implementa um algoritmo estratégico para resolver
// o passeio do cavalo no tabuleiro de xadrez. 
// O objetivo é encontrar um caminho que permita ao cavalo visitar todas as casas do tabuleiro exatamente uma vez.
// O algoritmo utiliza uma abordagem heurística, priorizando movimentos que levam a casas com menos opções de saída, 
// aumentando assim as chances de completar o passeio com sucesso.

#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>
using namespace std;

// ============ CONFIGURAÇÕES ============
// Posições X (colunas) e Y (linhas) iniciais do cavalo no tabuleiro
const int POSICAO_INICIAL[2] = {0, 0};

// Delay entre cada passo do cavalo (em milissegundos)
const int DELAY = 0;

// ============ CONSTANTES ============

// Tabuleiro de xadrez representado como uma matriz 8x8 (inalteravel)
const int TAMANHO_MATRIZ = 8;

// Número máximo de movimentos possíveis para o cavalo (inalteravel)
const int MAX_MOVIMENTOS = 8;

// === Caracteres ===
const char char_cavalo = 'C';
const char char_visitado = 'X';
const char char_nao_visitado = '.';

// ============ VARIÁVEIS GLOBAIS ============
// Tabuleiro
// . indica espaço não visitado, C indica posição do cavalo, e X indica casas já visitadas
char board[TAMANHO_MATRIZ][TAMANHO_MATRIZ];

// Cavalo
struct {
    
    int x, y;
    int movimentos[MAX_MOVIMENTOS][2] = {{2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}, {1, -2}, {2, -1}};
} cavalo;

// Tempo de execução do algoritmo (em milissegundos)
long long tempo_execucao = 0;

// Numero de passos realizados pelo cavalo
int passos = 1;

// Solução final do passeio é aberto (Não volta ao ponto de partida) ou fechado (Volta ao ponto de partida)
bool passeio_aberto = true;

// ============ FUNÇÕES AUXILIARES ============
// Inicializa o tabuleiro com espaços não visitados
void inicializarTabuleiro() {
    for (int i = 0; i < TAMANHO_MATRIZ; i++) {
        for (int j = 0; j < TAMANHO_MATRIZ; j++) {
            board[i][j] = char_nao_visitado;
        }
    }
}

// Exibe visualmente o tabuleiro no console, incluindo indicadores numéricos e eixo X e Y
void exibirTabuleiro() {
    cout << "Eixo X = colunas, Eixo Y = linhas" << endl;

    cout << "   ";
    for (int x = 0; x < TAMANHO_MATRIZ; x++) {
        cout << x << " ";
    }
    cout << endl;

    for (int y = 0; y < TAMANHO_MATRIZ; y++) {
        cout << y << " |";
        for (int x = 0; x < TAMANHO_MATRIZ; x++) {
            cout << board[x][y] << " ";
        }
        cout << endl;
    }
}

// Verifica se a posição (x, y) é válida para o cavalo se mover
bool posicaoValida(int x, int y) {
    // Regra 1: Posição está dentro dos limites do tabuleiro?
    bool nos_limites = (x >= 0 && x < TAMANHO_MATRIZ && y >= 0 && y < TAMANHO_MATRIZ);

    // Regra 2: Posição não foi visitada ainda?
    bool posicao_nao_visitada = (board[x][y] == char_nao_visitado);

    return (nos_limites && posicao_nao_visitada);
}

// Retorna o número de movimentos válidos disponíveis a partir da posição (x, y)
int contarMovimentosValidos(int x, int y) {
    int count = 0;
    for (int i = 0; i < MAX_MOVIMENTOS; i++) {
        int novo_x = x + cavalo.movimentos[i][0];
        int novo_y = y + cavalo.movimentos[i][1];
        if (posicaoValida(novo_x, novo_y)) {
            count++;
        }
    }
    return count;
}

// Retorna o próximo movimento com base na lista de movimentos válidos, priorizando aqueles que levam a casas com menos opções de saída
pair<int, int> proximoMovimento(int x, int y) {
    pair<int, int> melhor_movimento = {-1, -1};
    int menor_opcoes = 9; // Inicialmente maior que o máximo possível (8)

    // Analisa todos os movimentos possíveis do cavalo
    for (int i = 0; i < MAX_MOVIMENTOS; i++) {
        int novo_x = x + cavalo.movimentos[i][0];
        int novo_y = y + cavalo.movimentos[i][1];

        // Se é valido, guarda como melhor movimento se tiver menos opções de saída
        if (posicaoValida(novo_x, novo_y)) {
            int opcoes = contarMovimentosValidos(novo_x, novo_y);
            if (opcoes < menor_opcoes) {
                menor_opcoes = opcoes;
                melhor_movimento = {novo_x, novo_y};
            }
        }
    }
    return melhor_movimento;
}

// Renderiza o console, mostrando o tabuleiro e o progresso do passeio do cavalo
void renderizarConsole() {
    system("cls"); // Limpa o console (Windows)
    cout << "Passos realizados: " << passos << endl;
    cout << "Posição atual do cavalo: (" << cavalo.x << ", " << cavalo.y << ")" << endl;
    cout << "Tempo de execução: " << tempo_execucao << " ms" << endl;
    exibirTabuleiro();
}

int main() {
    // === Preparação do console para UTF-8 ===
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // === Inicialização do tabuleiro e das variáveis necessárias ===
    inicializarTabuleiro();
    cavalo.x = POSICAO_INICIAL[0];
    cavalo.y = POSICAO_INICIAL[1];
    board[cavalo.x][cavalo.y] = char_cavalo;
    auto inicio = std::chrono::high_resolution_clock::now();

    // === Implementação do algoritmo estratégico para o passeio do cavalo ===
    pair<int, int> proximo = proximoMovimento(cavalo.x, cavalo.y);
    while (proximo.first != -1 && proximo.second != -1) {
        // Renderiza console e aguarda um pequeno delay para continuar
        // renderizarConsole();
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

        board[cavalo.x][cavalo.y] = char_visitado;
        cavalo.x = proximo.first;
        cavalo.y = proximo.second;
        passos++;
        board[cavalo.x][cavalo.y] = char_cavalo;
        proximo = proximoMovimento(cavalo.x, cavalo.y);
    }
    // Renderiza o estado final do tabuleiro
    renderizarConsole();

    // === Calculo do tempo de execução do algoritmo ===
    auto fim = std::chrono::high_resolution_clock::now();
    tempo_execucao = std::chrono::duration_cast<std::chrono::milliseconds>(fim - inicio).count();
    // Desconsidera o delay no tempo de execução
    tempo_execucao -= passos * DELAY;

    // === Calculo se o passeio é aberto ou fechado ===
    bool pode_voltar = false;
    for (int i = 0; i < MAX_MOVIMENTOS; i++) {
        int novo_x = cavalo.x + cavalo.movimentos[i][0];
        int novo_y = cavalo.y + cavalo.movimentos[i][1];
        if (novo_x == POSICAO_INICIAL[0] && novo_y == POSICAO_INICIAL[1]) {
            pode_voltar = true;
            break;
        }
    }
    passeio_aberto = !pode_voltar;

    // === Exibição dos resultados finais ===
    cout << "Passeio do cavalo concluído com sucesso!" << endl;
    cout << endl << endl;

    cout << "===== Dados do passeio =====" << endl;

    cout << "Posição inicial do cavalo: (" << POSICAO_INICIAL[0] << ", " << POSICAO_INICIAL[1] << ")" << endl;
    cout << "Posição final do cavalo: (" << cavalo.x << ", " << cavalo.y << ")" << endl;
    cout << "Número total de passos realizados: " << passos << endl;
    cout << "O passeio é " << (passeio_aberto ? "aberto" : "fechado") << "." << endl;
    cout << "Tempo de execução: " << tempo_execucao << " ms" << endl;

    cout << "=============================" << endl;

    cout << "Aperte enter para sair..." << endl;
    cin.ignore();
    return 0;
}