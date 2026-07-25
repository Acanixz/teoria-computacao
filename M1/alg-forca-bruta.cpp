//O PASSEIO DO CAVALO
 
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <iomanip>
#include <cstdlib>
using namespace std;

const int TAMANHO_TABULEIRO = 8;   // Tabuleiro de xadrez oficial (8x8)
const int TOTAL_CASAS = 64;        // Quantidade de casas a visitar

const int DESLOCAMENTO_LINHA[8] =
{
     2,  1, -1, -2,
    -2, -1,  1,  2
};
const int DESLOCAMENTO_COLUNA[8] =
{
     1,  2,  2,  1,
    -1, -2, -2, -1
};

// Contador global de tentativas feitas pelo backtracking.
long long tentativas = 0;

const long long INTERVALO_RELATORIO_MS = 160000; // 160 segundos mostra a tela

// Instante em que a busca comecou, usado para calcular o tempo decorrido.
chrono::steady_clock::time_point instanteInicial;

// Instante do ultimo relatorio de progresso impresso (para saber quando
chrono::steady_clock::time_point ultimoRelatorio;

//limparTela

void limparTela(){
#ifdef _WIN32
    int r = system("cls");
#else
    int r = system("clear");
#endif
    (void)r;
}

 // exibirQuadro

void exibirQuadro(vector<vector<int>>& tabuleiro, int linhaAtual, int colunaAtual){
    cout << "      O PASSEIO DO CAVALO\n";

    for(int linha=0; linha<TAMANHO_TABULEIRO; linha++){
        for(int coluna=0; coluna<TAMANHO_TABULEIRO; coluna++){
            if(linha==linhaAtual && coluna==colunaAtual)
                cout << " C ";                                   // Cavalo esta aqui agora.
            else if(tabuleiro[linha][coluna] == -2)
                cout << " . ";
            else if(tabuleiro[linha][coluna] >= 0)
                cout << setw(2) << tabuleiro[linha][coluna] << " "; // Casa ja visitada: mostra o passo.
            else
                cout << " - ";                                   // Casa ainda nao visitada.
        }
        cout << endl;
    }

    auto agora = chrono::steady_clock::now();
    auto tempoDecorridoMs = chrono::duration_cast<chrono::milliseconds>(agora - instanteInicial).count();

    cout << "\nTentativas: " << tentativas;
    cout << "\nTempo: " << tempoDecorridoMs << " ms";
    cout << "\n";
}

/*
 * movimentoValido
 * ----------------
 * Um movimento e valido quando a casa de destino (linha, coluna):
 *   1) existe dentro do tabuleiro 0 a 7 em cada eixo e
 *   2) ainda nao foi visitada tabuleiro[linha][coluna] == -1
 */
bool movimentoValido(int linha, int coluna, vector<vector<int>>& tabuleiro){
    return linha>=0 && linha<TAMANHO_TABULEIRO &&
           coluna>=0 && coluna<TAMANHO_TABULEIRO &&
           tabuleiro[linha][coluna]==-1;
}


  //relatarProgressoSePreciso

void relatarProgressoSePreciso(){
    auto agora = chrono::steady_clock::now();
    auto desdeUltimoRelatorioMs = chrono::duration_cast<chrono::milliseconds>(agora - ultimoRelatorio).count();

    if(desdeUltimoRelatorioMs < INTERVALO_RELATORIO_MS)
        return;

    auto tempoTotalMs = chrono::duration_cast<chrono::milliseconds>(agora - instanteInicial).count();
    cout << "Tentativas: " << tentativas << " | Tempo decorrido: " << tempoTotalMs << " ms\n";

    ultimoRelatorio = agora;
}


  //passeioDoCavalo

bool passeioDoCavalo(vector<vector<int>>& tabuleiro, int linha, int coluna, int passoAtual){
    if(passoAtual==TOTAL_CASAS)
        return true; // Todas as casas foram visitadas: passeio completo.

    for(int i=0; i<8; i++){
        int novaLinha = linha + DESLOCAMENTO_LINHA[i];
        int novaColuna = coluna + DESLOCAMENTO_COLUNA[i];

        if(movimentoValido(novaLinha, novaColuna, tabuleiro)){
            tentativas++;
            relatarProgressoSePreciso(); // So imprime algo a cada 160s.

            tabuleiro[novaLinha][novaColuna] = passoAtual;

            if(passeioDoCavalo(tabuleiro, novaLinha, novaColuna, passoAtual+1))
                return true;

            // Backtracking: desfaz o caminho errado, sem exibir nada na tela.
            tabuleiro[novaLinha][novaColuna] = -1;
        }
    }

    return false; // Nenhum movimento a partir daqui leva a uma solucao.
}

int main(){
    int linhaInicial, colunaInicial;

    cout << "      PASSEIO DO CAVALO 8x8\n\n";
    cout << "Linha inicial (0-7): ";
    cin >> linhaInicial;
    cout << "Coluna inicial (0-7): ";
    cin >> colunaInicial;

    // Tabuleiro 8x8 todo vazio (-1 = casa ainda nao visitada).
    vector<vector<int>> tabuleiro(TAMANHO_TABULEIRO, vector<int>(TAMANHO_TABULEIRO, -1));
    tabuleiro[linhaInicial][colunaInicial] = 0; // Casa inicial = passo 0.

    instanteInicial = chrono::steady_clock::now();
    ultimoRelatorio = instanteInicial; // Zera a contagem dos 160s a partir do inicio da busca.

    cout << "\nBuscando : "
            "aparece a cada 160 segundos, se a busca demorar tanto\n\n";

    bool passeioEncontrado = passeioDoCavalo(tabuleiro, linhaInicial, colunaInicial, 1);

    limparTela();
    if(passeioEncontrado){
        cout << "SOLUCAO ENCONTRADA!\n\n";
        exibirQuadro(tabuleiro, -1, -1); // -1,-1 = nenhuma casa marcada como "C".
    } else {
        cout << "Nao foi possivel encontrar solucao.\n";
    }

    cout << "\nTotal tentativas: " << tentativas << endl;
    return 0;
}
