#include "sistema_interno.h"
#include <stdlib.h>
#include <string.h>

// procura um arquivo pelo nome na pasta informada
Arquivo *buscar_arquivo(const Diretorio *diretorio, const char *nome) {
    for (size_t i = 0; i < diretorio->quantidade_arquivos; ++i) {
        if (strcmp(diretorio->arquivos[i]->nome, nome) == 0) {
            return diretorio->arquivos[i];
        }
    }
    return NULL;
}

// retorna o nome do tipo de arquivo
const char *tipo_nome(TipoArquivo tipo) {
    switch (tipo) {
    case TIPO_NUMERICO:
        return "numerico";
    case TIPO_CARACTERE:
        return "caractere";
    case TIPO_BINARIO:
        return "binario";
    case TIPO_PROGRAMA:
        return "programa";
    default:
        return "tipo invalido";
    }
}

// consulta um arquivo da pasta atual sem alterar suas datas
Resultado sistema_consultar_arquivo(const Sistema *sistema, const char *nome,
                                    const Arquivo **saida) {
    if (!sistema_esta_pronto(sistema) || !nome || !saida) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    Resultado resultado = nome_validar(nome);
    if (resultado != SIS_OK) {
        return resultado;
    }
    Arquivo *arquivo = buscar_arquivo(sistema->atual, nome);
    if (!arquivo) {
        return buscar_pasta(sistema->atual, nome) ? SIS_TIPO_INCORRETO : SIS_NAO_ENCONTRADO;
    }
    *saida = arquivo;
    return SIS_OK;
}

// obtem o arquivo da pasta atual para uma operacao
Resultado obter_arquivo_atual(Sistema *sistema, const char *nome, Arquivo **arquivo) {
    const Arquivo *arquivo_encontrado;
    Resultado resultado = sistema_consultar_arquivo(sistema, nome, &arquivo_encontrado);
    if (resultado == SIS_OK) {
        *arquivo = (Arquivo *)arquivo_encontrado;
    }
    return resultado;
}

// valida os dados e aloca um novo arquivo
static Resultado preparar_novo_arquivo(Sistema *sistema, Diretorio *diretorio, const char *nome,
                                       TipoArquivo tipo, Arquivo **saida) {
    Resultado resultado = nome_validar(nome);
    if (resultado != SIS_OK) {
        return resultado;
    }
    if (tipo < TIPO_NUMERICO || tipo > TIPO_PROGRAMA) {
        return SIS_TIPO_INCORRETO;
    }
    if (nome_ja_existe(diretorio, nome)) {
        return SIS_NOME_DUPLICADO;
    }
    if (!sistema->proximo_id) {
        return SIS_ID_ESGOTADO;
    }
    time_t agora = time(NULL);
    if (agora == (time_t)-1) {
        return SIS_ERRO_RELOGIO;
    }
    Arquivo *arquivo = calloc(1, sizeof *arquivo);
    if (!arquivo) {
        return SIS_SEM_MEMORIA;
    }
    strcpy(arquivo->nome, nome);
    arquivo->tipo = tipo;
    arquivo->id = sistema->proximo_id;
    arquivo->criacao = agora;
    arquivo->modificacao = agora;
    arquivo->acesso = agora;
    arquivo->dono_id = obter_usuario_atual(sistema)->id;
    arquivo->grupo = obter_usuario_atual(sistema)->grupo;
    arquivo->permissoes = PERMISSAO_PADRAO;
    *saida = arquivo;
    return SIS_OK;
}

// prepara mais espaco no vetor de arquivos, se necessario
static Resultado preparar_vetor_arquivos(Diretorio *diretorio, Arquivo ***vetor,
                                         size_t *capacidade) {
    *vetor = diretorio->arquivos;
    *capacidade = diretorio->capacidade_arquivos;
    if (diretorio->quantidade_arquivos < diretorio->capacidade_arquivos) {
        return SIS_OK;
    }
    *vetor =
        criar_vetor_maior(diretorio->arquivos, diretorio->quantidade_arquivos,
                          diretorio->capacidade_arquivos, sizeof *diretorio->arquivos, capacidade);
    return *vetor ? SIS_OK : SIS_SEM_MEMORIA;
}

// substitui o vetor antigo pelo novo quando houve crescimento
static void confirmar_vetor_arquivos(Diretorio *diretorio, Arquivo **vetor, size_t capacidade) {
    if (vetor != diretorio->arquivos) {
        free(diretorio->arquivos);
        diretorio->arquivos = vetor;
        diretorio->capacidade_arquivos = capacidade;
    }
}

// adiciona o arquivo na pasta e avanca o contador de ids
static void adicionar_arquivo_na_pasta(Sistema *sistema, Diretorio *diretorio, Arquivo *arquivo) {
    diretorio->arquivos[diretorio->quantidade_arquivos++] = arquivo;
    if (arquivo->id == UINT64_MAX) {
        sistema->proximo_id = 0;
    } else {
        sistema->proximo_id = arquivo->id + 1;
    }
}

// cria um arquivo vazio na pasta atual
Resultado sistema_criar_arquivo(Sistema *sistema, const char *nome, TipoArquivo tipo) {
    if (!sistema_esta_pronto(sistema) || !nome) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    Arquivo *arquivo;
    Resultado resultado = preparar_novo_arquivo(sistema, sistema->atual, nome, tipo, &arquivo);
    if (resultado != SIS_OK) {
        return resultado;
    }
    Arquivo **vetor;
    size_t capacidade;
    resultado = preparar_vetor_arquivos(sistema->atual, &vetor, &capacidade);
    if (resultado != SIS_OK) {
        free(arquivo);
        return resultado;
    }
    confirmar_vetor_arquivos(sistema->atual, vetor, capacidade);
    adicionar_arquivo_na_pasta(sistema, sistema->atual, arquivo);
    return SIS_OK;
}

// verifica a permissao e substitui o conteudo do arquivo
Resultado sistema_escrever_arquivo(Sistema *sistema, const char *nome,
                                   const unsigned char *conteudo, size_t tamanho) {
    Arquivo *arquivo;
    Resultado resultado = obter_arquivo_atual(sistema, nome, &arquivo);
    if (resultado != SIS_OK) {
        return resultado;
    }
    if (!arquivo_tem_permissao(arquivo, obter_usuario_atual(sistema), PERM_W)) {
        return SIS_PERMISSAO_NEGADA;
    }
    if (!conteudo && tamanho) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    if (tamanho > MAX_CONTEUDO) {
        return SIS_ARQUIVO_GRANDE;
    }
    for (size_t i = 0; i < tamanho; ++i) {
        if (!conteudo[i] || conteudo[i] == '\r' || conteudo[i] == '\n') {
            return SIS_ARGUMENTO_INVALIDO;
        }
    }
    time_t agora = time(NULL);
    if (agora == (time_t)-1) {
        return SIS_ERRO_RELOGIO;
    }
    resultado = disco_reescrever(&sistema->disco, arquivo, conteudo, tamanho);
    if (resultado == SIS_OK) {
        arquivo->modificacao = agora;
    }
    return resultado;
}

// verifica a permissao e le o conteudo do arquivo
Resultado sistema_ler_arquivo(Sistema *sistema, const char *nome, unsigned char *saida,
                              size_t capacidade, size_t *lido) {
    Arquivo *arquivo;
    Resultado resultado = obter_arquivo_atual(sistema, nome, &arquivo);
    if (resultado != SIS_OK) {
        return resultado;
    }
    if (!arquivo_tem_permissao(arquivo, obter_usuario_atual(sistema), PERM_R)) {
        return SIS_PERMISSAO_NEGADA;
    }
    time_t agora = time(NULL);
    if (agora == (time_t)-1) {
        return SIS_ERRO_RELOGIO;
    }
    resultado = disco_ler(&sistema->disco, arquivo, saida, capacidade, lido);
    if (resultado == SIS_OK) {
        arquivo->acesso = agora;
    }
    return resultado;
}

// verifica tipo e permissao para simular a execucao
Resultado sistema_executar_arquivo(Sistema *sistema, const char *nome) {
    Arquivo *arquivo;
    Resultado resultado = obter_arquivo_atual(sistema, nome, &arquivo);
    if (resultado != SIS_OK) {
        return resultado;
    }
    if (arquivo->tipo != TIPO_PROGRAMA) {
        return SIS_TIPO_INCORRETO;
    }
    if (!arquivo_tem_permissao(arquivo, obter_usuario_atual(sistema), PERM_X)) {
        return SIS_PERMISSAO_NEGADA;
    }
    time_t agora = time(NULL);
    if (agora == (time_t)-1) {
        return SIS_ERRO_RELOGIO;
    }
    arquivo->acesso = agora;
    return SIS_OK;
}

// cria uma copia com novo id e blocos independentes
Resultado sistema_copiar_arquivo(Sistema *sistema, const char *origem, const char *destino,
                                 const char *nome) {
    Arquivo *arquivo, *novo_arquivo;
    const Diretorio *diretorio;
    unsigned char conteudo[MAX_CONTEUDO];
    size_t tamanho;
    Resultado resultado = obter_arquivo_atual(sistema, origem, &arquivo);
    if (resultado != SIS_OK) {
        return resultado;
    }
    if (!arquivo_tem_permissao(arquivo, obter_usuario_atual(sistema), PERM_R)) {
        return SIS_PERMISSAO_NEGADA;
    }
    resultado = diretorio_resolver(sistema, destino, &diretorio);
    if (resultado != SIS_OK) {
        return resultado;
    }
    resultado =
        preparar_novo_arquivo(sistema, (Diretorio *)diretorio, nome, arquivo->tipo, &novo_arquivo);
    if (resultado != SIS_OK) {
        return resultado;
    }
    Arquivo **vetor;
    size_t capacidade;
    resultado = preparar_vetor_arquivos((Diretorio *)diretorio, &vetor, &capacidade);
    if (resultado != SIS_OK) {
        free(novo_arquivo);
        return resultado;
    }
    resultado = disco_ler(&sistema->disco, arquivo, conteudo, sizeof conteudo, &tamanho);
    if (resultado == SIS_OK) {
        resultado = disco_reescrever(&sistema->disco, novo_arquivo, conteudo, tamanho);
    }
    if (resultado != SIS_OK) {
        if (vetor != diretorio->arquivos) {
            free(vetor);
        }
        free(novo_arquivo);
        return resultado;
    }
    confirmar_vetor_arquivos((Diretorio *)diretorio, vetor, capacidade);
    arquivo->acesso = novo_arquivo->criacao;
    adicionar_arquivo_na_pasta(sistema, (Diretorio *)diretorio, novo_arquivo);
    return SIS_OK;
}

// retira a entrada do vetor sem liberar o arquivo
static void retirar_arquivo_da_pasta(Diretorio *diretorio, Arquivo *arquivo) {
    size_t i = 0;
    while (diretorio->arquivos[i] != arquivo) {
        ++i;
    }
    --diretorio->quantidade_arquivos;
    for (; i < diretorio->quantidade_arquivos; ++i) {
        diretorio->arquivos[i] = diretorio->arquivos[i + 1];
    }
    diretorio->arquivos[diretorio->quantidade_arquivos] = NULL;
}

// move ou renomeia o arquivo mantendo sua identidade
Resultado sistema_mover_arquivo(Sistema *sistema, const char *origem, const char *destino,
                                const char *nome) {
    Arquivo *arquivo;
    const Diretorio *diretorio;
    Resultado resultado = obter_arquivo_atual(sistema, origem, &arquivo);
    if (resultado != SIS_OK) {
        return resultado;
    }
    if (!arquivo_e_proprietario(arquivo, obter_usuario_atual(sistema))) {
        return SIS_PERMISSAO_NEGADA;
    }
    resultado = diretorio_resolver(sistema, destino, &diretorio);
    if (resultado != SIS_OK) {
        return resultado;
    }
    resultado = nome_validar(nome);
    if (resultado != SIS_OK) {
        return resultado;
    }
    if (diretorio == sistema->atual && strcmp(arquivo->nome, nome) == 0) {
        return SIS_SEM_ALTERACAO;
    }
    if (nome_ja_existe(diretorio, nome)) {
        return SIS_NOME_DUPLICADO;
    }
    if (diretorio != sistema->atual) {
        Arquivo **vetor;
        size_t capacidade;
        resultado = preparar_vetor_arquivos((Diretorio *)diretorio, &vetor, &capacidade);
        if (resultado != SIS_OK) {
            return resultado;
        }
        confirmar_vetor_arquivos((Diretorio *)diretorio, vetor, capacidade);
    }
    strcpy(arquivo->nome, nome);
    if (diretorio != sistema->atual) {
        retirar_arquivo_da_pasta(sistema->atual, arquivo);
        Diretorio *pasta_destino = (Diretorio *)diretorio;
        pasta_destino->arquivos[pasta_destino->quantidade_arquivos++] = arquivo;
    }
    return SIS_OK;
}

// exclui o arquivo e libera seus blocos e sua memoria
Resultado sistema_excluir_arquivo(Sistema *sistema, const char *nome) {
    Arquivo *arquivo;
    Resultado resultado = obter_arquivo_atual(sistema, nome, &arquivo);
    if (resultado != SIS_OK) {
        return resultado;
    }
    if (!arquivo_e_proprietario(arquivo, obter_usuario_atual(sistema))) {
        return SIS_PERMISSAO_NEGADA;
    }
    disco_liberar(&sistema->disco, arquivo->blocos, arquivo->quantidade_blocos);
    retirar_arquivo_da_pasta(sistema->atual, arquivo);
    free(arquivo);
    return SIS_OK;
}
