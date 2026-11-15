#include "sistema_interno.h"
#include <stdlib.h>
#include <string.h>

// prepara um vetor maior sem alterar o antigo
void *criar_vetor_maior(const void *antigo, size_t quantidade, size_t capacidade,
                        size_t tamanho_item, size_t *nova_capacidade) {
    if (capacidade > SIZE_MAX / 2) {
        return NULL;
    }
    size_t nova = capacidade == 0 ? 4 : capacidade * 2;
    if (nova > SIZE_MAX / tamanho_item) {
        return NULL;
    }
    void *novo = calloc(nova, tamanho_item);
    if (!novo) {
        return NULL;
    }
    if (quantidade > 0) {
        memcpy(novo, antigo, quantidade * tamanho_item);
    }
    *nova_capacidade = nova;
    return novo;
}

// verifica se o sistema esta inicializado e tem usuario atual
bool sistema_esta_pronto(const Sistema *sistema) {
    return sistema && sistema->raiz && sistema->atual && sistema->usuarios &&
           sistema->quantidade_usuarios <= MAX_USUARIOS &&
           sistema->quantidade_usuarios <= sistema->capacidade_usuarios &&
           sistema->usuario_atual < sistema->quantidade_usuarios;
}

// libera os arquivos, vetores e pastas de toda a arvore
static void liberar_arvore(Diretorio *diretorio) {
    if (!diretorio) {
        return;
    }
    for (size_t i = 0; i < diretorio->quantidade_subpastas; ++i) {
        liberar_arvore(diretorio->subpastas[i]);
    }
    for (size_t i = 0; i < diretorio->quantidade_arquivos; ++i) {
        free(diretorio->arquivos[i]);
    }
    free(diretorio->arquivos);
    free(diretorio->subpastas);
    free(diretorio);
}

// cria a raiz e os usuarios iniciais e prepara o disco
Resultado sistema_inicializar(Sistema *sistema) {
    if (!sistema) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    Diretorio *raiz = calloc(1, sizeof *raiz);
    if (!raiz) {
        memset(sistema, 0, sizeof *sistema);
        return SIS_SEM_MEMORIA;
    }
    Usuario *usuarios = calloc(TOTAL_USUARIOS, sizeof *usuarios);
    if (!usuarios) {
        free(raiz);
        memset(sistema, 0, sizeof *sistema);
        return SIS_SEM_MEMORIA;
    }
    memset(sistema, 0, sizeof *sistema);
    sistema->usuarios = usuarios;
    sistema->capacidade_usuarios = TOTAL_USUARIOS;
    strcpy(raiz->nome, "/");
    sistema->raiz = raiz;
    sistema->atual = raiz;
    sistema->proximo_id = 1;
    const char *nomes[] = {"ana", "bruno", "carla"};
    for (size_t i = 0; i < TOTAL_USUARIOS; ++i) {
        sistema->usuarios[i].id = (unsigned int)i + 1;
        strcpy(sistema->usuarios[i].nome, nomes[i]);
        sistema->usuarios[i].grupo = i == 2 ? GRUPO_VISITANTES : GRUPO_ALUNOS;
    }
    sistema->quantidade_usuarios = TOTAL_USUARIOS;
    disco_inicializar(&sistema->disco);
    return SIS_OK;
}

// libera a memoria do sistema e zera sua estrutura
void sistema_liberar(Sistema *sistema) {
    if (sistema) {
        liberar_arvore(sistema->raiz);
        free(sistema->usuarios);
        memset(sistema, 0, sizeof *sistema);
    }
}

// retorna a mensagem correspondente ao resultado da operacao
const char *resultado_mensagem(Resultado resultado) {
    static const char *mensagens[] = {"Sucesso",
                                      "Sem alteracao",
                                      "Argumento invalido",
                                      "Nome invalido",
                                      "Caminho invalido",
                                      "Item nao encontrado",
                                      "Tipo incorreto",
                                      "Nome duplicado",
                                      "Permissao negada",
                                      "Diretorio cheio",
                                      "Profundidade maxima",
                                      "Arquivo grande demais",
                                      "Disco cheio",
                                      "Sem memoria",
                                      "IDs esgotados",
                                      "Buffer insuficiente",
                                      "Erro de relogio",
                                      "Limite de usuarios atingido"};
    return (unsigned)resultado < sizeof mensagens / sizeof *mensagens ? mensagens[resultado]
                                                                      : "Resultado invalido";
}
