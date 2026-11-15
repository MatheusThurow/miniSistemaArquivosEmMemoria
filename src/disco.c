#include "sistema.h"
#include <string.h>

// zera os dados e marca todos os blocos como livres
void disco_inicializar(Disco *disco) {
    memset(disco, 0, sizeof *disco);
}

// calcula quantos blocos o conteudo precisa
size_t disco_blocos_necessarios(size_t tamanho) {
    size_t quantidade = tamanho / TAM_BLOCO;
    if (tamanho % TAM_BLOCO != 0) {
        quantidade++;
    }
    return quantidade;
}

// conta os blocos disponiveis no disco
size_t disco_blocos_livres(const Disco *disco) {
    size_t quantidade_livres = 0;
    for (size_t i = 0; i < TOTAL_BLOCOS; ++i) {
        if (disco->ocupado[i] == false) {
            quantidade_livres++;
        }
    }
    return quantidade_livres;
}

// reserva todos os blocos pedidos ou nenhum
Resultado disco_reservar(Disco *disco, size_t quantidade, size_t *indices) {
    size_t blocos_encontrados[MAX_BLOCOS_ARQUIVO], quantidade_encontrada = 0;
    if (!disco || quantidade > MAX_BLOCOS_ARQUIVO || (quantidade && !indices)) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    /* primeiro encontra todos os blocos; so depois confirma a reserva */
    for (size_t i = 0; i < TOTAL_BLOCOS && quantidade_encontrada < quantidade; ++i) {
        if (!disco->ocupado[i]) {
            blocos_encontrados[quantidade_encontrada++] = i;
        }
    }
    if (quantidade_encontrada < quantidade) {
        return SIS_DISCO_CHEIO;
    }
    for (size_t i = 0; i < quantidade; ++i) {
        indices[i] = blocos_encontrados[i];
        disco->ocupado[blocos_encontrados[i]] = true;
    }
    return SIS_OK;
}

// limpa os blocos informados e os marca como livres
void disco_liberar(Disco *disco, const size_t *indices, size_t quantidade) {
    for (size_t i = 0; i < quantidade; ++i) {
        disco->ocupado[indices[i]] = false;
        memset(disco->dados[indices[i]], 0, TAM_BLOCO);
    }
}

// grava o conteudo e ajusta os blocos usados pelo arquivo
Resultado disco_reescrever(Disco *disco, Arquivo *arquivo, const unsigned char *conteudo,
                           size_t tamanho) {
    size_t indices[MAX_BLOCOS_ARQUIVO], quantidade_necessaria;
    if (!disco || !arquivo || (tamanho && !conteudo)) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    if (tamanho > MAX_CONTEUDO) {
        return SIS_ARQUIVO_GRANDE;
    }
    quantidade_necessaria = disco_blocos_necessarios(tamanho);
    memcpy(indices, arquivo->blocos, sizeof indices);
    /* reaproveita os blocos atuais e reserva apenas o que falta */
    if (quantidade_necessaria > arquivo->quantidade_blocos) {
        Resultado resultado =
            disco_reservar(disco, quantidade_necessaria - arquivo->quantidade_blocos,
                           indices + arquivo->quantidade_blocos);
        if (resultado != SIS_OK) {
            return resultado;
        }
    }
    for (size_t i = 0; i < quantidade_necessaria; ++i) {
        size_t bytes_no_bloco = tamanho - i * TAM_BLOCO;
        if (bytes_no_bloco > TAM_BLOCO) {
            bytes_no_bloco = TAM_BLOCO;
        }
        memset(disco->dados[indices[i]], 0, TAM_BLOCO);
        memcpy(disco->dados[indices[i]], conteudo + i * TAM_BLOCO, bytes_no_bloco);
    }
    if (quantidade_necessaria < arquivo->quantidade_blocos) {
        disco_liberar(disco, arquivo->blocos + quantidade_necessaria,
                      arquivo->quantidade_blocos - quantidade_necessaria);
    }
    memset(arquivo->blocos, 0, sizeof arquivo->blocos);
    memcpy(arquivo->blocos, indices, quantidade_necessaria * sizeof *indices);
    arquivo->tamanho = tamanho;
    arquivo->quantidade_blocos = quantidade_necessaria;
    return SIS_OK;
}

// copia o conteudo dos blocos para o buffer de saida
Resultado disco_ler(const Disco *disco, const Arquivo *arquivo, unsigned char *saida,
                    size_t capacidade, size_t *tamanho_lido) {
    if (!disco || !arquivo || !tamanho_lido || (!saida && capacidade)) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    if (capacidade < arquivo->tamanho) {
        return SIS_BUFFER_INSUFICIENTE;
    }
    for (size_t i = 0; i < arquivo->quantidade_blocos; ++i) {
        size_t bytes_no_bloco = arquivo->tamanho - i * TAM_BLOCO;
        if (bytes_no_bloco > TAM_BLOCO) {
            bytes_no_bloco = TAM_BLOCO;
        }
        memcpy(saida + i * TAM_BLOCO, disco->dados[arquivo->blocos[i]], bytes_no_bloco);
    }
    *tamanho_lido = arquivo->tamanho;
    return SIS_OK;
}
