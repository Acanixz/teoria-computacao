// ============================================================
// PASSEIO DO CAVALO — programa unificado
//
// Junta os dois algoritmos originais (alg-forca-bruta.cpp e
// alg-estrategia-parede.cpp) em um único executável, com um
// menu de 4 opções:
//   1 - Executar Heuristica Aleatoria
//   2 - Executar Heuristica Parede (Warnsdorff)
//   3 - Executar teste completo (gera arquivo csv)
//   4 - Sair
// ============================================================

#define NOMINMAX

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <limits>
#include <string>
using namespace std;

#ifdef _WIN32
#include <windows.h>
#endif

// ============ CONSTANTES ============

// Tabuleiro de xadrez representado como uma matriz 8x8 (inalteravel)
const int TAMANHO_MATRIZ = 8;

// Número máximo de movimentos possíveis para o cavalo (inalteravel)
const int MAX_MOVIMENTOS = 8;

// Valor usado para marcar uma casa ainda não visitada
const int NAO_VISITADO = -1;

// Largura (em caracteres) usada para exibir cada número de passo no tabuleiro
const int LARGURA_CASA = 3;

// Limite de tempo da heurística aleatória (10 minutos), igual ao original
const long long LIMITE_TEMPO_ALEATORIO_MS = 10 * 60 * 1000;

// Posições fixas usadas na opção 3 (lista da planilha do professor)
const int POSICOES_TESTE[][2] = {
    {0, 0}, {7, 7}, {7, 0}, {0, 7}, {3, 4}, {1, 1},
    {0, 1}, {6, 7}, {7, 1}, {1, 7}, {5, 4}, {2, 3}
};
const int NUM_POSICOES_TESTE = sizeof(POSICOES_TESTE) / sizeof(POSICOES_TESTE[0]);

const char* ARQUIVO_CSV_SAIDA = "resultado_teste_completo.csv";

// ============ ESTRUTURAS ============

struct Cavalo {
    int x, y;
    int movimentos[MAX_MOVIMENTOS][2] = {
        {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}, {1, -2}, {2, -1}
    };
};

// Resultado de uma execução da heurística aleatória (força bruta)
struct ResultadoAleatorio {
    double tempo_ms;       // medido com precisão de microssegundos
    int passos;
    bool timeout;          // true se estourou os 10 minutos sem completar
    bool passeio_aberto;   // só tem sentido garantido quando timeout == false
};

// Resultado de uma execução da heurística parede (Warnsdorff)
struct ResultadoParede {
    double tempo_ms;       // medido com precisão de microssegundos
    int passos;
    bool passeio_aberto;
};

// ============ VARIÁVEIS GLOBAIS ============

// Cada posição guarda o número do passo em que foi visitada,
// ou NAO_VISITADO (-1) se a casa ainda não foi visitada
int board[TAMANHO_MATRIZ][TAMANHO_MATRIZ];
Cavalo cavalo;

// ============ FUNÇÕES AUXILIARES COMPARTILHADAS ============

// Inicializa o tabuleiro com casas não visitadas
void inicializarTabuleiro() {
    for (int i = 0; i < TAMANHO_MATRIZ; i++) {
        for (int j = 0; j < TAMANHO_MATRIZ; j++) {
            board[i][j] = NAO_VISITADO;
        }
    }
}

// Exibe visualmente o tabuleiro no console, com o número do passo em cada
// casa, incluindo indicadores numéricos e eixo X e Y
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

// Verifica se a posição (x, y) é válida para o cavalo se mover.
// Obs: aqui os limites são checados ANTES de acessar board[x][y] — os
// originais acessavam board[x][y] mesmo fora dos limites (leitura fora
// dos limites do array); corrigido aqui para evitar comportamento indefinido.
bool posicaoValida(int x, int y) {
    bool nos_limites = (x >= 0 && x < TAMANHO_MATRIZ && y >= 0 && y < TAMANHO_MATRIZ);
    if (!nos_limites) return false;
    return board[x][y] == NAO_VISITADO;
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

// Heurística aleatória: sorteia um movimento válido qualquer
pair<int, int> proximoMovimentoAleatorio(int x, int y) {
    int novo_x, novo_y, indice;
    do {
        indice = rand() % MAX_MOVIMENTOS;
        novo_x = x + cavalo.movimentos[indice][0];
        novo_y = y + cavalo.movimentos[indice][1];
    } while (!posicaoValida(novo_x, novo_y));
    return {novo_x, novo_y};
}

// Heurística de Warnsdorff ("Parede"): prioriza a casa com menos saídas
pair<int, int> proximoMovimentoParede(int x, int y) {
    pair<int, int> melhor_movimento = {-1, -1};
    int menor_opcoes = MAX_MOVIMENTOS + 1;

    for (int i = 0; i < MAX_MOVIMENTOS; i++) {
        int novo_x = x + cavalo.movimentos[i][0];
        int novo_y = y + cavalo.movimentos[i][1];

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

// Verifica se, a partir da posição final, o cavalo poderia voltar à posição
// inicial (passeio fechado) ou não (passeio aberto)
bool calcularPasseioAberto(int x_final, int y_final, int px_inicial, int py_inicial) {
    for (int i = 0; i < MAX_MOVIMENTOS; i++) {
        int novo_x = x_final + cavalo.movimentos[i][0];
        int novo_y = y_final + cavalo.movimentos[i][1];
        if (novo_x == px_inicial && novo_y == py_inicial) {
            return false; // fechado
        }
    }
    return true; // aberto
}

// Lê e valida a posição X e Y iniciais informadas pelo usuário
pair<int, int> lerPosicaoInicial() {
    int x = -1, y = -1;
    while (true) {
        cout << "Defina a posição X e Y iniciais (numeros separados por espaços)" << endl;
        if (cin >> x >> y && x >= 0 && x < TAMANHO_MATRIZ && y >= 0 && y < TAMANHO_MATRIZ) {
            break;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Posição inválida! Informe dois números entre 0 e " << (TAMANHO_MATRIZ - 1) << "." << endl;
    }
    return {x, y};
}

// ============ ALGORITMO 1: HEURÍSTICA ALEATÓRIA (FORÇA BRUTA) ============
// Mesma lógica do alg-forca-bruta.cpp original: sorteia movimentos válidos e
// repete tentativas até completar as 64 casas ou estourar o limite de 10 min.
// Quando mostrar == true, imprime tabuleiro e estatísticas no console
// (comportamento igual ao programa original).
ResultadoAleatorio executarAleatorio(int px, int py, bool mostrar) {
    int passos = 1;
    int melhor_passos = 0;
    int melhor_board[TAMANHO_MATRIZ][TAMANHO_MATRIZ];
    int melhor_x = px, melhor_y = py;
    bool encontrou_solucao_completa = false;
    long long tentativas = 0;

    auto inicio = std::chrono::high_resolution_clock::now();

    do {
        tentativas++;
        passos = 1;

        inicializarTabuleiro();
        cavalo.x = px;
        cavalo.y = py;
        board[cavalo.x][cavalo.y] = passos - 1;

        while (contarMovimentosValidos(cavalo.x, cavalo.y) > 0) {
            pair<int, int> proximo = proximoMovimentoAleatorio(cavalo.x, cavalo.y);
            cavalo.x = proximo.first;
            cavalo.y = proximo.second;
            passos++;
            board[cavalo.x][cavalo.y] = passos - 1;
        }

        if (passos > melhor_passos) {
            melhor_passos = passos;
            melhor_x = cavalo.x;
            melhor_y = cavalo.y;
            for (int i = 0; i < TAMANHO_MATRIZ; i++)
                for (int j = 0; j < TAMANHO_MATRIZ; j++)
                    melhor_board[i][j] = board[i][j];
        }

        auto agora = std::chrono::high_resolution_clock::now();
        long long decorrido_ms = std::chrono::duration_cast<std::chrono::milliseconds>(agora - inicio).count();
        if (decorrido_ms >= LIMITE_TEMPO_ALEATORIO_MS) {
            break;
        }

    } while (passos < TAMANHO_MATRIZ * TAMANHO_MATRIZ);

    auto fim = std::chrono::high_resolution_clock::now();
    double tempo_execucao = std::chrono::duration<double, std::milli>(fim - inicio).count();

    encontrou_solucao_completa = (passos == TAMANHO_MATRIZ * TAMANHO_MATRIZ);

    if (!encontrou_solucao_completa) {
        passos = melhor_passos;
        cavalo.x = melhor_x;
        cavalo.y = melhor_y;
        for (int i = 0; i < TAMANHO_MATRIZ; i++)
            for (int j = 0; j < TAMANHO_MATRIZ; j++)
                board[i][j] = melhor_board[i][j];
    }

    bool passeio_aberto = calcularPasseioAberto(cavalo.x, cavalo.y, px, py);

    if (mostrar) {
        exibirTabuleiro();
        cout << endl << endl;
        if (encontrou_solucao_completa) {
            cout << "Passeio do cavalo concluído com sucesso!" << endl;
        } else {
            cout << "Tempo limite de 10 minutos atingido. Nenhum passeio completo foi encontrado." << endl;
            cout << "Exibindo a melhor tentativa (" << melhor_passos << " passos)." << endl;
        }
        cout << endl;
        cout << "===== Dados do passeio =====" << endl;
        cout << "Posição inicial do cavalo: (" << px << ", " << py << ")" << endl;
        cout << "Posição final do cavalo: (" << cavalo.x << ", " << cavalo.y << ")" << endl;
        cout << "Número total de passos realizados: " << passos << endl;
        cout << "Número de tentativas realizadas: " << tentativas << endl;
        if (encontrou_solucao_completa) {
            cout << "O passeio é " << (passeio_aberto ? "aberto" : "fechado") << "." << endl;
        }
        cout << "Tempo de execução: " << fixed << setprecision(3) << tempo_execucao << " ms" << endl;
        cout.unsetf(ios_base::fixed);
        cout << "=============================" << endl;
    }

    ResultadoAleatorio resultado;
    resultado.tempo_ms = tempo_execucao;
    resultado.passos = passos;
    resultado.timeout = !encontrou_solucao_completa;
    resultado.passeio_aberto = passeio_aberto;
    return resultado;
}

// ============ ALGORITMO 2: HEURÍSTICA PAREDE (WARNSDORFF) ============
// Mesma lógica do alg-estrategia-parede.cpp original: em cada passo, escolhe
// a casa válida com o menor número de saídas futuras.
ResultadoParede executarParede(int px, int py, bool mostrar) {
    inicializarTabuleiro();
    int passos = 1;
    cavalo.x = px;
    cavalo.y = py;
    board[cavalo.x][cavalo.y] = passos;

    auto inicio = std::chrono::high_resolution_clock::now();

    pair<int, int> proximo = proximoMovimentoParede(cavalo.x, cavalo.y);
    while (proximo.first != -1 && proximo.second != -1) {
        cavalo.x = proximo.first;
        cavalo.y = proximo.second;
        passos++;
        board[cavalo.x][cavalo.y] = passos;
        proximo = proximoMovimentoParede(cavalo.x, cavalo.y);
    }

    auto fim = std::chrono::high_resolution_clock::now();
    double tempo_execucao = std::chrono::duration<double, std::milli>(fim - inicio).count();

    bool passeio_aberto = calcularPasseioAberto(cavalo.x, cavalo.y, px, py);

    if (mostrar) {
        exibirTabuleiro();
        cout << endl << endl;
        cout << "Passeio do cavalo concluído!" << endl << endl;
        cout << "===== Dados do passeio =====" << endl;
        cout << "Posição inicial do cavalo: (" << px << ", " << py << ")" << endl;
        cout << "Posição final do cavalo: (" << cavalo.x << ", " << cavalo.y << ")" << endl;
        cout << "Número total de passos realizados: " << passos << endl;
        cout << "O passeio é " << (passeio_aberto ? "aberto" : "fechado") << "." << endl;
        cout << "Tempo de execução: " << fixed << setprecision(3) << tempo_execucao << " ms" << endl;
        cout.unsetf(ios_base::fixed);
        cout << "=============================" << endl;
    }

    ResultadoParede resultado;
    resultado.tempo_ms = tempo_execucao;
    resultado.passos = passos;
    resultado.passeio_aberto = passeio_aberto;
    return resultado;
}

// ============ OPÇÃO 3: TESTE COMPLETO (GERA CSV) ============

string statusAleatorioParaTexto(const ResultadoAleatorio& r) {
    if (r.timeout) return "TIMEOUT";
    return r.passeio_aberto ? "ABERTO" : "FECHADO";
}

string statusParedeParaTexto(const ResultadoParede& r) {
    return r.passeio_aberto ? "ABERTO" : "FECHADO";
}

// Executa os dois algoritmos para cada posição fixada em POSICOES_TESTE,
// gravando os resultados em CSV.
void executarTesteCompleto() {
    ofstream csv(ARQUIVO_CSV_SAIDA);
    if (!csv.is_open()) {
        cout << "Erro: não foi possível criar o arquivo " << ARQUIVO_CSV_SAIDA << endl;
        return;
    }

    csv << "Posicao,Tempo_Total_Aleatorio,Passos_Aleatorio,Resultado_Aleatorio,"
        << "Tempo_Parede,Passos_Parede,Resultado_Parede" << endl;
    csv << fixed << setprecision(3);

    cout << "Iniciando teste completo com " << NUM_POSICOES_TESTE << " posições." << endl;
    cout << "Aviso: a heuristica aleatoria pode levar ate 10 minutos POR POSICAO " << endl;
    cout << "caso nao encontre um passeio completo (pior caso: ~" << (NUM_POSICOES_TESTE * 10)
         << " minutos no total)." << endl << endl;

    for (int i = 0; i < NUM_POSICOES_TESTE; i++) {
        int px = POSICOES_TESTE[i][0];
        int py = POSICOES_TESTE[i][1];

        cout << "[" << (i + 1) << "/" << NUM_POSICOES_TESTE << "] Posição (" << px << ", " << py << ")" << endl;

        cout << "  Executando heuristica aleatoria..." << endl;
        ResultadoAleatorio ra = executarAleatorio(px, py, false);
        cout << "  -> " << statusAleatorioParaTexto(ra) << " em " << fixed << setprecision(3) << ra.tempo_ms
             << " ms, " << ra.passos << " passos." << endl;
        cout.unsetf(ios_base::fixed);

        cout << "  Executando heuristica parede (Warnsdorff)..." << endl;
        ResultadoParede rp = executarParede(px, py, false);
        cout << "  -> " << statusParedeParaTexto(rp) << " em " << fixed << setprecision(3) << rp.tempo_ms
             << " ms, " << rp.passos << " passos." << endl << endl;
        cout.unsetf(ios_base::fixed);

        // Campo "Posicao" entre aspas por conter vírgula: "(x, y)"
        csv << "\"(" << px << ", " << py << ")\","
            << ra.tempo_ms << ","
            << ra.passos << ","
            << statusAleatorioParaTexto(ra) << ","
            << rp.tempo_ms << ","
            << rp.passos << ","
            << statusParedeParaTexto(rp)
            << endl;
        csv.flush(); // grava progresso incrementalmente
    }

    csv.close();
    cout << "Teste completo! Resultados salvos em " << ARQUIVO_CSV_SAIDA << endl;
}

// ============ MENU PRINCIPAL ============

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    srand(static_cast<unsigned int>(time(0))); // semente única pro programa inteiro

    int opcao = -1;
    bool primeira_execucao = true;

    while (true) {
        if (!primeira_execucao) {
            cout << endl << "=============== LOOP ===============" << endl << endl;
        }
        primeira_execucao = false;

        cout << "1 - Executar Heuristica Aleatoria" << endl;
        cout << "2 - Executar Heuristica Parede (Warnsdorff)" << endl;
        cout << "3 - Executar teste completo (gera arquivo csv)" << endl;
        cout << "4 - Sair" << endl;

        if (!(cin >> opcao)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        cout << endl << "===" << endl << endl;

        if (opcao == 4) {
            break;
        } else if (opcao == 1) {
            pair<int, int> pos = lerPosicaoInicial();
            cout << endl << "===" << endl << endl;
            executarAleatorio(pos.first, pos.second, true);
        } else if (opcao == 2) {
            pair<int, int> pos = lerPosicaoInicial();
            cout << endl << "===" << endl << endl;
            executarParede(pos.first, pos.second, true);
        } else if (opcao == 3) {
            executarTesteCompleto();
        } else {
            cout << "Opção inválida!" << endl;
        }
    }

    cout << "Aperte enter para sair..." << endl;
    cin.ignore();
    return 0;
}