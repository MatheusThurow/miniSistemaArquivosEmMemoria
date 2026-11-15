#include "sistema_interno.h"
#include <stdlib.h>
#include <string.h>

// procura uma subpasta pelo nome
Diretorio *buscar_pasta(const Diretorio *diretorio, const char *nome) {
    for (size_t i = 0; i < diretorio->quantidade_subpastas; ++i) {
        if (strcmp(diretorio->subpastas[i]->nome, nome) == 0) {
            return diretorio->subpastas[i];
        }
    }
    return NULL;
}

// verifica se uma pasta ou arquivo ja usa o nome
bool nome_ja_existe(const Diretorio *diretorio, const char *nome) {
    return buscar_arquivo(diretorio, nome) || buscar_pasta(diretorio, nome);
}

// verifica se o nome respeita as regras do simulador
Resultado nome_validar(const char *nome) {
    bool tem_caractere_visivel = false;
    if (!nome) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    size_t tamanho = strlen(nome);
    if (!tamanho || tamanho > MAX_NOME || strcmp(nome, ".") == 0 || strcmp(nome, "..") == 0) {
        return SIS_NOME_INVALIDO;
    }
    for (size_t i = 0; i < tamanho; ++i) {
        unsigned char caractere = (unsigned char)nome[i];
        if (caractere < 32 || caractere == 127 || caractere == '/') {
            return SIS_NOME_INVALIDO;
        }
        if (caractere != ' ') {
            tem_caractere_visivel = true;
        }
    }
    return tem_caractere_visivel ? SIS_OK : SIS_NOME_INVALIDO;
}

// localiza uma pasta pelo caminho absoluto
Resultado diretorio_resolver(const Sistema *sistema, const char *trecho_caminho,
                             const Diretorio **saida) {
    if (!sistema_esta_pronto(sistema) || !trecho_caminho || !saida) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    if (*trecho_caminho != '/' || strlen(trecho_caminho) >= TAM_CAMINHO) {
        return SIS_CAMINHO_INVALIDO;
    }
    const Diretorio *diretorio = sistema->raiz;
    ++trecho_caminho;
    while (*trecho_caminho) {
        const char *barra = strchr(trecho_caminho, '/');
        size_t tamanho_nome = barra ? (size_t)(barra - trecho_caminho) : strlen(trecho_caminho);
        char nome[MAX_NOME + 1];
        if (!tamanho_nome || tamanho_nome > MAX_NOME) {
            return SIS_CAMINHO_INVALIDO;
        }
        memcpy(nome, trecho_caminho, tamanho_nome);
        nome[tamanho_nome] = 0;
        if (nome_validar(nome) != SIS_OK) {
            return SIS_CAMINHO_INVALIDO;
        }
        const Diretorio *proximo = buscar_pasta(diretorio, nome);
        if (!proximo) {
            return buscar_arquivo(diretorio, nome) ? SIS_TIPO_INCORRETO : SIS_NAO_ENCONTRADO;
        }
        diretorio = proximo;
        if (!barra) {
            break;
        }
        trecho_caminho = barra + 1;
    }
    *saida = diretorio;
    return SIS_OK;
}

// monta o caminho completo da pasta atual
Resultado sistema_caminho_atual(const Sistema *sistema, char *saida, size_t capacidade) {
    const Diretorio *pilha[MAX_PROFUNDIDADE];
    size_t quantidade_pastas = 0, posicao = 0;
    char caminho[TAM_CAMINHO];
    if (!sistema_esta_pronto(sistema) || !saida) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    for (const Diretorio *diretorio = sistema->atual; diretorio->pai; diretorio = diretorio->pai) {
        pilha[quantidade_pastas++] = diretorio;
    }
    if (!quantidade_pastas) {
        caminho[posicao++] = '/';
    }
    while (quantidade_pastas) {
        const char *nome = pilha[--quantidade_pastas]->nome;
        caminho[posicao++] = '/';
        size_t tamanho_nome = strlen(nome);
        memcpy(caminho + posicao, nome, tamanho_nome);
        posicao += tamanho_nome;
    }
    caminho[posicao] = 0;
    if (capacidade <= posicao) {
        return SIS_BUFFER_INSUFICIENTE;
    }
    memcpy(saida, caminho, posicao + 1);
    return SIS_OK;
}

// cria uma subpasta dentro da pasta atual
Resultado sistema_criar_pasta(Sistema *sistema, const char *nome) {
    if (!sistema_esta_pronto(sistema) || !nome) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    Resultado resultado = nome_validar(nome);
    if (resultado != SIS_OK) {
        return resultado;
    }
    if (nome_ja_existe(sistema->atual, nome)) {
        return SIS_NOME_DUPLICADO;
    }
    size_t profundidade = 0;
    for (Diretorio *diretorio = sistema->atual; diretorio->pai; diretorio = diretorio->pai) {
        ++profundidade;
    }
    if (profundidade >= MAX_PROFUNDIDADE) {
        return SIS_PROFUNDIDADE_MAXIMA;
    }
    Diretorio *diretorio = calloc(1, sizeof *diretorio);
    if (!diretorio) {
        return SIS_SEM_MEMORIA;
    }
    Diretorio *pai = sistema->atual;
    if (pai->quantidade_subpastas == pai->capacidade_subpastas) {
        size_t capacidade;
        Diretorio **novas_pastas =
            criar_vetor_maior(pai->subpastas, pai->quantidade_subpastas, pai->capacidade_subpastas,
                              sizeof *pai->subpastas, &capacidade);
        if (!novas_pastas) {
            free(diretorio);
            return SIS_SEM_MEMORIA;
        }
        free(pai->subpastas);
        pai->subpastas = novas_pastas;
        pai->capacidade_subpastas = capacidade;
    }
    strcpy(diretorio->nome, nome);
    diretorio->pai = sistema->atual;
    sistema->atual->subpastas[sistema->atual->quantidade_subpastas++] = diretorio;
    return SIS_OK;
}

// entra em uma subpasta usando apenas seu nome
Resultado sistema_entrar_pasta(Sistema *sistema, const char *nome) {
    if (!sistema_esta_pronto(sistema) || !nome) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    Resultado resultado = nome_validar(nome);
    if (resultado != SIS_OK) {
        return resultado;
    }
    Diretorio *pasta = buscar_pasta(sistema->atual, nome);
    if (!pasta) {
        return buscar_arquivo(sistema->atual, nome) ? SIS_TIPO_INCORRETO : SIS_NAO_ENCONTRADO;
    }
    sistema->atual = pasta;
    return SIS_OK;
}

// volta para a pasta pai, sem ultrapassar a raiz
Resultado sistema_voltar_pai(Sistema *sistema) {
    if (!sistema_esta_pronto(sistema)) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    if (!sistema->atual->pai) {
        return SIS_SEM_ALTERACAO;
    }
    sistema->atual = sistema->atual->pai;
    return SIS_OK;
}
