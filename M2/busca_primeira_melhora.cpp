// ============================================================
// BUSCA LOCAL PRIMEIRA MELHORA — Trabalho M2
// Disciplina: Teoria da Computação (UNIVALI)
//
// Problema: distribuição de n tarefas entre m máquinas paralelas,
// minimizando o makespan (tempo de uso da máquina mais carregada).
//
// A busca local primeira melhora ("first improvement") percorre a
// vizinhança da solução atual (mover uma única tarefa para outra
// máquina) e aceita o primeiro vizinho que melhora o makespan, 
// reiniciando a varredura a partir dele. O processo para
// quando uma varredura completa da vizinhança não encontra nenhum
// vizinho melhor, ou seja, a solução atual é um ótimo local.
//
// Ao final, todos os experimentos são gravados em um arquivo CSV com
// as mesmas colunas do M3, para permitir juntar os dois arquivos e
// comparar as heurísticas diretamente:
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
const double VALORES_R[] = {1.5, 2.0};
const int NUM_VALORES_R = sizeof(VALORES_R) / sizeof(VALORES_R[0]);

// Tempo mínimo e máximo de execução de cada tarefa (sorteado)
const int TEMPO_TAREFA_MIN = 1;
const int TEMPO_TAREFA_MAX = 100;

// Quantas vezes cada instância (m, r) é executada
const int NUM_REPLICACOES = 10;

const char* ARQUIVO_CSV_SAIDA = "resultado_primeira_melhora.csv";

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
    long long valor; // melhor makespan encontrado (ótimo local)
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

// ============ BUSCA LOCAL PRIMEIRA MELHORA ============
//
// A cada varredura, percorre-se todas as tarefas e, para cada uma,
// todas as máquinas de destino possíveis (vizinhança de realocação de
// uma única tarefa). Assim que um vizinho que melhora o makespan atual
// é encontrado, ele é aplicado imediatamente e a varredura recomeça a
// partir da nova solução. Quando uma varredura inteira não encontra
// nenhum vizinho melhor, a busca para: a solução atual é um ótimo
// local.
ResultadoExecucao buscaLocalPrimeiraMelhora(const Instancia& inst) {
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

    long long iteracoes = 0;
    bool encontrouMelhora;

    do {
        encontrouMelhora = false;

        // Varre a vizinhança: cada tarefa combinada com cada máquina destino até encontrar a primeira melhora
        for (int tarefa = 0; tarefa < n && !encontrouMelhora; tarefa++) {
            int maquinaOrigem = maquinaDaTarefa[tarefa];

            for (int maquinaDestino = 0; maquinaDestino < m; maquinaDestino++) {
                if (maquinaDestino == maquinaOrigem) continue;

                iteracoes++;

                // Calcula o makespan hipotético caso o movimento seja aplicado
                long long cargaOrigemNova = cargaMaquinas[maquinaOrigem] - inst.tempos[tarefa];
                long long cargaDestinoNova = cargaMaquinas[maquinaDestino] + inst.tempos[tarefa];

                long long makespanVizinho = 0;
                for (int j = 0; j < m; j++) {
                    long long carga = cargaMaquinas[j];
                    if (j == maquinaOrigem) carga = cargaOrigemNova;
                    else if (j == maquinaDestino) carga = cargaDestinoNova;
                    if (carga > makespanVizinho) makespanVizinho = carga;
                }

                // Primeira melhora: aceita e para de varrer assim que achar
                // um vizinho estritamente melhor que a solução atual
                if (makespanVizinho < makespanAtual) {
                    maquinaDaTarefa[tarefa] = maquinaDestino;
                    cargaMaquinas[maquinaOrigem] = cargaOrigemNova;
                    cargaMaquinas[maquinaDestino] = cargaDestinoNova;
                    makespanAtual = makespanVizinho;
                    encontrouMelhora = true;
                    break; // reinicia a varredura a partir da nova solução
                }
            }
        }
    } while (encontrouMelhora); // para quando uma varredura completa não melhora nada

    auto fim = std::chrono::high_resolution_clock::now();
    double tempo_execucao = std::chrono::duration<double, std::milli>(fim - inicio).count();

    ResultadoExecucao resultado;
    resultado.tempo_ms = tempo_execucao;
    resultado.iteracoes = iteracoes;
    resultado.valor = makespanAtual;
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

    int totalCombinacoes = NUM_VALORES_M * NUM_VALORES_R;
    int combinacaoAtual = 0;

    for (int im = 0; im < NUM_VALORES_M; im++) {
        for (int ir = 0; ir < NUM_VALORES_R; ir++) {
            int m = VALORES_M[im];
            double r = VALORES_R[ir];

            combinacaoAtual++;
            Instancia inst = gerarInstancia(m, r);

            cout << "[" << combinacaoAtual << "/" << totalCombinacoes << "] "
                 << "Instância m=" << m << ", r=" << r << " (n=" << inst.n << "): ";

            for (int rep = 1; rep <= NUM_REPLICACOES; rep++) {
                ResultadoExecucao res = buscaLocalPrimeiraMelhora(inst);

                csv << "primeiramelhora,"
                    << inst.n << ","
                    << inst.m << ","
                    << rep << ","
                    << res.tempo_ms << ","
                    << res.iteracoes << ","
                    << res.valor << ","
                    << "NA"
                    << endl;

                cout << res.valor;
                if (rep < NUM_REPLICACOES) cout << ", ";
            }
            cout << endl;
            csv.flush(); // grava progresso incrementalmente
        }
    }

    csv.close();
    cout << endl << "Experimentos concluídos! Resultados salvos em " << ARQUIVO_CSV_SAIDA << endl;
}

// ============ MENU PRINCIPAL ============

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    srand(static_cast<unsigned int>(time(0)));

    cout << "===== Busca Local Primeira Melhora =====" << endl;
    cout << "Problema: distribuição de tarefas entre máquinas (makespan)" << endl;
    cout << "m em {10, 20, 50}, r em {1.5, 2.0}" << endl;
    cout << "10 replicações por instância, critério de parada: ótimo local" << endl;
    cout << "(nenhum vizinho de melhoria encontrado em uma varredura completa)" << endl;
    cout << "==========================================" << endl << endl;

    executarTodosExperimentos();

    cout << endl << "Aperte enter para sair..." << endl;
    cin.get();
    return 0;
}
