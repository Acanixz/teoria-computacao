#define NOMINMAX

#include <iostream>
#include <thread>
#include <windows.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <limits>
using namespace std;

// ============ CONFIGURAÇÕES ============
// Posições X (colunas) e Y (linhas) iniciais do cavalo no tabuleiro.
// Deixou de ser const: agora e definida pelo usuario a cada execucao, via menu.
int posicao_inicial[2] = {2, 3};


// ============ CONSTANTES ============

// 10 minutos
const long long LIMITE_TEMPO_MS = 10 * 60 * 1000;

// Tabuleiro de xadrez representado como uma matriz 8x8 (inalteravel)
const int TAMANHO_MATRIZ = 8;

// Número máximo de movimentos possíveis para o cavalo (inalteravel)
const int MAX_MOVIMENTOS = 8;

// Valor usado para marcar uma casa ainda não visitada
const int NAO_VISITADO = -1;

// Largura (em caracteres) usada para exibir cada número de passo no tabuleiro
const int LARGURA_CASA = 3;

// ============ VARIÁVEIS GLOBAIS ============
// Tabuleiro
// Cada posição guarda o número do passo em que foi visitada (0 = posição inicial),
// ou NAO_VISITADO (-1) se a casa ainda não foi visitada
int board[TAMANHO_MATRIZ][TAMANHO_MATRIZ];

// Cavalo
struct {
    
    int x, y;
    int movimentos[MAX_MOVIMENTOS][2] = {{2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}, {1, -2}, {2, -1}};
} cavalo;

// Tempo de execução do algoritmo (em milissegundos)
long long tempo_execucao = 0;


// ============ FUNÇÕES AUXILIARES ============
// Inicializa o tabuleiro com casas não visitadas
void inicializarTabuleiro() {
    for (int i = 0; i < TAMANHO_MATRIZ; i++) {
        for (int j = 0; j < TAMANHO_MATRIZ; j++) {
            board[i][j] = NAO_VISITADO;
        }
    }
}

// Exibe visualmente o tabuleiro no console, com o número do passo em cada casa, incluindo indicadores numéricos e eixo X e Y
void exibirTabuleiro() {
    cout << "Eixo X = colunas, Eixo Y = linhas" << endl;

    cout << "   ";
    for (int x = 0; x < TAMANHO_MATRIZ; x++) {
        cout << setw(LARGURA_CASA) << x;
    }
    cout << endl;

    for (int y = 0; y < TAMANHO_MATRIZ; y++) {
        cout << y << " |";
        for (int x = 0; x < TAMANHO_MATRIZ; x++) {
            if (board[x][y] == NAO_VISITADO) {
                cout << setw(LARGURA_CASA) << ".";
            } else {
                cout << setw(LARGURA_CASA) << board[x][y];
            }
        }
        cout << endl;
    }
}


// Verifica se a posição (x, y) é válida para o cavalo se mover
bool posicaoValida(int x, int y) {
    // Regra 1: Posição está dentro dos limites do tabuleiro?
    bool nos_limites = (x >= 0 && x < TAMANHO_MATRIZ && y >= 0 && y < TAMANHO_MATRIZ);

    // Regra 2: Posição não foi visitada ainda?
    bool posicao_nao_visitada = (board[x][y] == NAO_VISITADO);

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
    int novo_x, novo_y;
    int indice;
    do {
        indice = rand() % 8;                          // um único índice sorteado
        novo_x = x + cavalo.movimentos[indice][0];
        novo_y = y + cavalo.movimentos[indice][1];    // mesmo índice usado nos dois
    } while (!posicaoValida(novo_x, novo_y));
    return {novo_x, novo_y};
}

    

// Renderiza o console, mostrando o tabuleiro e o progresso do passeio do cavalo
// Nota: Comentado informações para conseguir gerar resultados em sequencia
void renderizarConsole(int passos) {
    // system("cls"); // Limpa o console (Windows)
    // cout << "Passos realizados: " << passos << endl;
    // cout << "Posição atual do cavalo: (" << cavalo.x << ", " << cavalo.y << ")" << endl;
    // cout << "Tempo de execução: " << tempo_execucao << " ms" << endl;
    exibirTabuleiro();
}

// Lê e valida a posição X e Y iniciais informadas pelo usuário
void lerPosicaoInicial() {
    int x = -1, y = -1;
    while (true) {
        cout << "Defina a posição X e Y iniciais (numeros separados por espaços)" << endl;
        if (cin >> x >> y && x >= 0 && x < TAMANHO_MATRIZ && y >= 0 && y < TAMANHO_MATRIZ) {
            break;
        }
        // Limpa o estado de erro do cin e descarta o resto da linha, caso a entrada seja inválida
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Posição inválida! Informe dois números entre 0 e " << (TAMANHO_MATRIZ - 1) << "." << endl;
    }
    posicao_inicial[0] = x;
    posicao_inicial[1] = y;
}

// Executa uma rodada completa do algoritmo aleatorio (com tentativas repetidas ate acertar ou estourar o tempo limite)
void executarAlgoritmo() {
    int passos = 1;
    bool passeio_aberto = true;

    // === Variáveis para guardar a melhor tentativa, caso o tempo esgote ===
    int melhor_passos = 0;
    int melhor_board[TAMANHO_MATRIZ][TAMANHO_MATRIZ];
    int melhor_x = posicao_inicial[0], melhor_y = posicao_inicial[1];
    bool encontrou_solucao_completa = false;
    long long tentativas = 0;

    auto inicio = std::chrono::high_resolution_clock::now();

    do{
        tentativas++;
        passeio_aberto = true;
        passos = 1;

        // === Inicialização do tabuleiro e das variáveis necessárias ===
        inicializarTabuleiro();
        cavalo.x = posicao_inicial[0];
        cavalo.y = posicao_inicial[1];
        board[cavalo.x][cavalo.y] = passos - 1; // Marca a posição inicial como o passo 0

        // === Implementação do algoritmo de força bruta para o passeio do cavalo ===  
        while (contarMovimentosValidos(cavalo.x,cavalo.y) > 0) {  
            pair<int, int> proximo = proximoMovimento(cavalo.x, cavalo.y);     
            cavalo.x = proximo.first;
            cavalo.y = proximo.second;
            passos++;
            board[cavalo.x][cavalo.y] = passos - 1; // Marca a casa com o número do passo (0-indexado)
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
            if (novo_x == posicao_inicial[0] && novo_y == posicao_inicial[1]) {
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
    cout << "Posição inicial do cavalo: (" << posicao_inicial[0] << ", " << posicao_inicial[1] << ")" << endl;
    cout << "Posição final do cavalo: (" << cavalo.x << ", " << cavalo.y << ")" << endl;
    cout << "Número total de passos realizados: " << passos << endl;
    cout << "Número de tentativas realizadas: " << tentativas << endl;
    if (encontrou_solucao_completa) {
        cout << "O passeio é " << (passeio_aberto ? "aberto" : "fechado") << "." << endl;
    }
    cout << "Tempo de execução: " << tempo_execucao << " ms" << endl;
    cout << "=============================" << endl;
}

int main() {
    // === Preparação do console para UTF-8 ===
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    srand(time(0)); // Semente unica pro programa inteiro, nao por execucao (evita repetir a mesma sequencia)

    int opcao = -1;
    bool primeira_execucao = true;

    while (true) {
        // === Menu ===
        if (!primeira_execucao) {
            cout << endl;
            cout << "=============== LOOP ===============" << endl;
            cout << endl;
        }
        primeira_execucao = false;

        cout << "1 - Executar algoritmo" << endl;
        cout << "2 - Sair" << endl;

        if (!(cin >> opcao)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        cout << endl << "===" << endl << endl;

        if (opcao == 2) {
            break;
        } else if (opcao == 1) {
            lerPosicaoInicial();
            cout << endl << "===" << endl << endl;
            executarAlgoritmo();
        } else {
            cout << "Opção inválida!" << endl;
        }
    }

    cout << "Aperte enter para sair..." << endl;
    cin.ignore();
    return 0;
}