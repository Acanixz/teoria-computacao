#include <iostream>
#include <thread>
#include <windows.h>
#include <chrono>
#include <ctime>
using namespace std;

// ============ CONFIGURAÇÕES ============
// Posições X (colunas) e Y (linhas) iniciais do cavalo no tabuleiro
const int POSICAO_INICIAL[2] = {0, 0};


// ============ CONSTANTES ============

// 10 minutos
const long long LIMITE_TEMPO_MS = 10 * 60 * 1000;

// Tabuleiro de xadrez representado como uma matriz 8x8 (inalteravel)
const int TAMANHO_MATRIZ = 8;

// Número máximo de movimentos possíveis para o cavalo (inalteravel)
const int MAX_MOVIMENTOS = 8;

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


// ============ FUNÇÕES AUXILIARES ============
// Inicializa o tabuleiro com espaços não visitados
void inicializarTabuleiro() {
    for (int i = 0; i < TAMANHO_MATRIZ; i++) {
        for (int j = 0; j < TAMANHO_MATRIZ; j++) {
            board[i][j] = '.';
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
    bool posicao_nao_visitada = (board[x][y] == '.');

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

// Retorna o próximo movimento aleatório e válido
pair<int, int> proximoMovimento(int x, int y) {
    int menor_opcoes = 9; // Inicialmente maior que o máximo possível (8)
    pair<int, int> movimento = {-1, -1};
    int novo_x;
    int novo_y;
    do{
    novo_x = x + cavalo.movimentos[rand() % 8][0];
    novo_y = y + cavalo.movimentos[rand() % 8][1];
    }    
    while(posicaoValida(novo_x, novo_y) == false); 
    movimento = {novo_x, novo_y};
    return movimento;  
    
}

    

// Renderiza o console, mostrando o tabuleiro e o progresso do passeio do cavalo
void renderizarConsole(int passos) {
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
    srand(time(0));
    int passos = 1;
    bool passeio_aberto = true;

    // === Variáveis para guardar a melhor tentativa, caso o tempo esgote ===
    int melhor_passos = 0;
    char melhor_board[TAMANHO_MATRIZ][TAMANHO_MATRIZ];
    int melhor_x = POSICAO_INICIAL[0], melhor_y = POSICAO_INICIAL[1];
    bool encontrou_solucao_completa = false;
    long long tentativas = 0;

    auto inicio = std::chrono::high_resolution_clock::now();



    do{
        tentativas++;
        passeio_aberto = true;
        passos = 1;

        // === Inicialização do tabuleiro e das variáveis necessárias ===
        inicializarTabuleiro();
        cavalo.x = POSICAO_INICIAL[0];
        cavalo.y = POSICAO_INICIAL[1];
        board[cavalo.x][cavalo.y] = 'C';

        // === Implementação do algoritmo de força bruta para o passeio do cavalo ===  
        while (contarMovimentosValidos(cavalo.x,cavalo.y) > 0) {  
            pair<int, int> proximo = proximoMovimento(cavalo.x, cavalo.y);     
            board[cavalo.x][cavalo.y] = 'X';
            cavalo.x = proximo.first;
            cavalo.y = proximo.second;
            passos++;
            board[cavalo.x][cavalo.y] = 'C';    
        }

        // === Guarda a melhor tentativa até agora ===
        if (passos > melhor_passos) {
            melhor_passos = passos;
            melhor_x = cavalo.x;
            melhor_y = cavalo.y;
            for (int i = 0; i < TAMANHO_MATRIZ; i++)
                for (int j = 0; j < TAMANHO_MATRIZ; j++)
                    melhor_board[i][j] = board[i][j];
        }

        // === Checa se o tempo limite foi atingido ===
        auto agora = std::chrono::high_resolution_clock::now();
        long long decorrido_ms = std::chrono::duration_cast<std::chrono::milliseconds>(agora - inicio).count();
        if (decorrido_ms >= LIMITE_TEMPO_MS) {
            break; // encerra o loop externo, sucesso ou não
        }

    }
    while (passos < 64);

    auto fim = std::chrono::high_resolution_clock::now();
    tempo_execucao = std::chrono::duration_cast<std::chrono::milliseconds>(fim - inicio).count();

    encontrou_solucao_completa = (passos == 64);

    // === Se não achou solução completa, usa a melhor tentativa para exibir ===
    if (!encontrou_solucao_completa) {
        passos = melhor_passos;
        cavalo.x = melhor_x;
        cavalo.y = melhor_y;
        for (int i = 0; i < TAMANHO_MATRIZ; i++)
            for (int j = 0; j < TAMANHO_MATRIZ; j++)
                board[i][j] = melhor_board[i][j];
    }

    // Renderiza o estado final do tabuleiro
    renderizarConsole(passos);

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
        if (pode_voltar) {
            passeio_aberto = false;
        } else {
            passeio_aberto = true;
        }

    // === Exibição dos resultados finais ===
    if (encontrou_solucao_completa) {
        cout << "Passeio do cavalo concluído com sucesso!" << endl;
    } else {
        cout << "Tempo limite de 10 minutos atingido. Nenhum passeio completo foi encontrado." << endl;
        cout << "Exibindo a melhor tentativa (" << melhor_passos << " passos)." << endl;
    }
    cout << endl << endl;

    cout << "===== Dados do passeio =====" << endl;
    cout << "Posição inicial do cavalo: (" << POSICAO_INICIAL[0] << ", " << POSICAO_INICIAL[1] << ")" << endl;
    cout << "Posição final do cavalo: (" << cavalo.x << ", " << cavalo.y << ")" << endl;
    cout << "Número total de passos realizados: " << passos << endl;
    cout << "Número de tentativas realizadas: " << tentativas << endl;
    if (encontrou_solucao_completa) {
        cout << "O passeio é " << (passeio_aberto ? "aberto" : "fechado") << "." << endl;
    }
    cout << "Tempo de execução: " << tempo_execucao << " ms" << endl;
    cout << "=============================" << endl;

    cout << "Aperte enter para sair..." << endl;
    cin.ignore();
    return 0;
}
