// ============================================================
// BUSCA LOCAL MONÓTONA RANDOMIZADA — Trabalho M3
// Disciplina: Teoria da Computação
//
// Problema: distribuição de n tarefas entre m máquinas paralelas,
// minimizando o makespan (tempo de uso da máquina mais carregada).
//
// A busca local monótona randomizada combina, a cada iteração, dois
// comportamentos controlados pelo parâmetro alpha:
//   - com probabilidade alpha: dá um passo de "caminhada aleatória",
//     aceitando o vizinho sorteado mesmo que ele piore a solução atual;
//   - caso contrário: dá um passo de descida monótona convencional,
//     só aceitando o vizinho sorteado se ele não piorar a solução atual.
//
// Critério de parada: 1000 iterações consecutivas sem melhora do
// melhor makespan encontrado.
//
// Ao final, todos os experimentos são gravados em um arquivo CSV:
//   heuristica,n,m,replicacao,tempo,iteracoes,valor,parametro
// ============================================================

#define NOMINMAX

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <string>

#define _HAS_STD_BYTE 0
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

using namespace std;

// ============ CONSTANTES ============

// Quantidades de máquinas a simular
const int VALORES_M[] = {10, 20, 50};
const int NUM_VALORES_M = sizeof(VALORES_M) / sizeof(VALORES_M[0]);

// Expoentes usados para calcular n = m^r
// Numero de tarefas n = round(m^r) para cada m
const double VALORES_R[] = {1.5, 2.0};
const int NUM_VALORES_R = sizeof(VALORES_R) / sizeof(VALORES_R[0]);

// Valores do parâmetro alpha (frequência da caminhada aleatória no espaço de busca)
const double VALORES_ALPHA[] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
const int NUM_VALORES_ALPHA = sizeof(VALORES_ALPHA) / sizeof(VALORES_ALPHA[0]);

// Tempo mínimo e máximo de execução de cada tarefa (sorteado)
const int TEMPO_TAREFA_MIN = 1;
const int TEMPO_TAREFA_MAX = 100;

// Quantas vezes cada instância (m, r) é executada para cada alpha
const int NUM_REPLICACOES = 10;

// Critério de parada: iterações consecutivas sem melhora do melhor makespan
const long long ITERACOES_SEM_MELHORA_LIMITE = 1000;

const char* ARQUIVO_CSV_SAIDA = "resultado_monotona_randomizada.csv";

// ============ ESTRUTURAS ============

// Uma instância do problema: n tarefas, m máquinas e o tempo de cada tarefa
struct Instancia {
    int n;
    int m;
    vector<int> tempos; // tempos[i] = tempo de processamento da tarefa i
};

// Resultado de uma execução (replicação) da busca local
struct ResultadoExecucao {
    double tempo_ms;
    long long iteracoes;
    long long valor; // melhor makespan encontrado
};

// ============ FUNÇÕES AUXILIARES ============

// Gera uma instância aleatória com m máquinas e n = round(m^r) tarefas,
// cada tarefa com tempo de processamento sorteado entre TEMPO_TAREFA_MIN
// e TEMPO_TAREFA_MAX (inclusive)
Instancia gerarInstancia(int m, double r) {
    Instancia inst;

    // m maquinas
    inst.m = m;

    // n tarefas = round(m^r)
    inst.n = static_cast<int>(llround(pow(static_cast<double>(m), r)));

    // tempo de cada tarefa sorteado aleatoriamente
    inst.tempos.resize(inst.n);
    for (int i = 0; i < inst.n; i++) {
        inst.tempos[i] = TEMPO_TAREFA_MIN + rand() % (TEMPO_TAREFA_MAX - TEMPO_TAREFA_MIN + 1);
    }
    return inst;
}

// Retorna o makespan (carga da máquina mais carregada) dado o vetor de
// cargas de cada máquina
long long calcularMakespan(const vector<long long>& cargaMaquinas) {
    long long maior = 0;
    for (long long carga : cargaMaquinas) {
        if (carga > maior) maior = carga;
    }
    return maior;
}

// ============ BUSCA LOCAL MONÓTONA RANDOMIZADA ============
//
// A cada iteração sorteia-se uma tarefa e uma máquina destino diferente
// da atual (um vizinho por realocação de uma única tarefa). Dependendo
// de um sorteio contra o parâmetro alpha, o movimento é:
//   - sempre aceito (passo de caminhada aleatória), ou
//   - aceito somente se não piorar o makespan atual (passo monótono).
ResultadoExecucao buscaLocalMonotonaRandomizada(const Instancia& inst, double alpha) {
    auto inicio = std::chrono::high_resolution_clock::now();

    int n = inst.n;
    int m = inst.m;

    // Solução inicial: cada tarefa alocada a uma máquina aleatória
    vector<int> maquinaDaTarefa(n);
    vector<long long> cargaMaquinas(m, 0);
    for (int i = 0; i < n; i++) {
        int maq = rand() % m;
        maquinaDaTarefa[i] = maq;
        cargaMaquinas[maq] += inst.tempos[i];
    }

    long long makespanAtual = calcularMakespan(cargaMaquinas);
    long long melhorMakespan = makespanAtual;

    long long iteracoes = 0;
    long long iterSemMelhora = 0;

    // Loop da busca local até atingir o limite de iterações sem melhora
    while (iterSemMelhora < ITERACOES_SEM_MELHORA_LIMITE) {
        iteracoes++;

        // Sorteia uma tarefa e uma máquina de destino diferente da atual
        int tarefa = rand() % n;
        int maquinaOrigem = maquinaDaTarefa[tarefa];
        int maquinaDestino;
        if (m > 1) {
            do {
                maquinaDestino = rand() % m;
            } while (maquinaDestino == maquinaOrigem);
        } else {
            maquinaDestino = maquinaOrigem; // só existe uma máquina
        }

        // Calcula o makespan hipotético caso o movimento seja aplicado,
        // sem alterar de fato o estado ainda
        long long cargaOrigemNova = cargaMaquinas[maquinaOrigem] - inst.tempos[tarefa];
        long long cargaDestinoNova = cargaMaquinas[maquinaDestino] + inst.tempos[tarefa];

        long long makespanVizinho = 0;
        for (int j = 0; j < m; j++) {
            long long carga = cargaMaquinas[j];
            if (j == maquinaOrigem) carga = cargaOrigemNova;
            else if (j == maquinaDestino) carga = cargaDestinoNova;
            if (carga > makespanVizinho) makespanVizinho = carga;
        }

        // Sorteio para decidir se aceita o vizinho (caminhada aleatória) ou não (passo monótono)
        double sorteio = static_cast<double>(rand()) / RAND_MAX;
        bool aceitar;
        if (sorteio < alpha) {
            // Aceita o vizinho mesmo se piorar
            aceitar = true;
        } else {
            // Só aceita se o vizinho não piorar a solução atual
            aceitar = (makespanVizinho <= makespanAtual);
        }

        if (aceitar) {
            maquinaDaTarefa[tarefa] = maquinaDestino;
            cargaMaquinas[maquinaOrigem] = cargaOrigemNova;
            cargaMaquinas[maquinaDestino] = cargaDestinoNova;
            makespanAtual = makespanVizinho;
        }

        if (makespanAtual < melhorMakespan) {
            melhorMakespan = makespanAtual;
            iterSemMelhora = 0;
        } else {
            iterSemMelhora++;
        }
    }

    auto fim = std::chrono::high_resolution_clock::now();
    double tempo_execucao = std::chrono::duration<double, std::milli>(fim - inicio).count();

    ResultadoExecucao resultado;
    resultado.tempo_ms = tempo_execucao;
    resultado.iteracoes = iteracoes;
    resultado.valor = melhorMakespan;
    return resultado;
}

// ============ EXECUÇÃO COMPLETA DE TODOS OS EXPERIMENTOS ============

void executarTodosExperimentos() {
    ofstream csv(ARQUIVO_CSV_SAIDA);
    if (!csv.is_open()) {
        cout << "Erro: não foi possível criar o arquivo " << ARQUIVO_CSV_SAIDA << endl;
        return;
    }

    csv << "heuristica,n,m,replicacao,tempo,iteracoes,valor,parametro" << endl;
    csv << fixed << setprecision(3);

    // Combinações = m (maquinas) x r (expoente para n = m^r)
    int totalCombinacoes = NUM_VALORES_M * NUM_VALORES_R;
    int combinacaoAtual = 0;

    for (int im = 0; im < NUM_VALORES_M; im++) {
        for (int ir = 0; ir < NUM_VALORES_R; ir++) {
            int m = VALORES_M[im];
            double r = VALORES_R[ir];

            combinacaoAtual++;
            Instancia inst = gerarInstancia(m, r);

            cout << "[" << combinacaoAtual << "/" << totalCombinacoes << "] "
                 << "Instância m=" << m << ", r=" << r << " (n=" << inst.n << ")" << endl;

            for (int ia = 0; ia < NUM_VALORES_ALPHA; ia++) {
                double alpha = VALORES_ALPHA[ia];

                cout << "  alpha=" << alpha << ": ";

                // Repete 10 vezes para cada combinação de (m, r, alpha)
                for (int rep = 1; rep <= NUM_REPLICACOES; rep++) {
                    ResultadoExecucao res = buscaLocalMonotonaRandomizada(inst, alpha);

                    csv << "monotonarandomizada,"
                        << inst.n << ","
                        << inst.m << ","
                        << rep << ","
                        << res.tempo_ms << ","
                        << res.iteracoes << ","
                        << res.valor << ","
                        << alpha
                        << endl;

                    cout << res.valor;
                    if (rep < NUM_REPLICACOES) cout << ", ";
                }
                cout << endl;
                csv.flush(); // grava progresso incrementalmente
            }
            cout << endl;
        }
    }

    csv.close();
    cout << "Experimentos concluídos! Resultados salvos em " << ARQUIVO_CSV_SAIDA << endl;
}

// ============ MENU PRINCIPAL ============

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    srand(static_cast<unsigned int>(time(0)));

    cout << "===== Busca Local Monótona Randomizada =====" << endl;
    cout << "Problema: distribuição de tarefas entre máquinas (makespan)" << endl;
    cout << "m em {10, 20, 50}, r em {1.5, 2.0}, alpha em {0.1,...,0.9}" << endl;
    cout << "10 replicações por combinação, critério de parada: 1000 iterações sem melhora" << endl;
    cout << "==============================================" << endl << endl;

    executarTodosExperimentos();

    cout << endl << "Aperte enter para sair..." << endl;
    cin.get();
    return 0;
}
