#ifndef SISTEMA_H
#define SISTEMA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#define TOTAL_BLOCOS 128       // capacidade finita do disco simulado
#define TAM_BLOCO 64          // blocos pequenos facilitam a demonstracao
#define MAX_BLOCOS_ARQUIVO 16 // limita o vetor de indices no fcb
#define MAX_CONTEUDO (MAX_BLOCOS_ARQUIVO * TAM_BLOCO) // limite de bytes por arquivo
#define MAX_NOME 63           // limita os buffers de nomes
#define MAX_PROFUNDIDADE 16   // limita a recursao na arvore
#define TOTAL_USUARIOS 3      // usuarios iniciais para testar permissoes
#define MAX_USUARIOS 67       // limite solicitado pelo usuario
#define TAM_CAMINHO (MAX_PROFUNDIDADE * (MAX_NOME + 1) + 2) // espaco para o maior caminho
#define PERMISSAO_PADRAO 0644u // permissoes iniciais dos arquivos
#define PERM_R 4u             // bit de leitura
#define PERM_W 2u             // bit de escrita
#define PERM_X 1u             // bit de execucao

typedef enum { TIPO_NUMERICO = 1, TIPO_CARACTERE, TIPO_BINARIO, TIPO_PROGRAMA } TipoArquivo;

typedef enum {
    SIS_OK = 0,
    SIS_SEM_ALTERACAO,
    SIS_ARGUMENTO_INVALIDO,
    SIS_NOME_INVALIDO,
    SIS_CAMINHO_INVALIDO,
    SIS_NAO_ENCONTRADO,
    SIS_TIPO_INCORRETO,
    SIS_NOME_DUPLICADO,
    SIS_PERMISSAO_NEGADA,
    SIS_DIRETORIO_CHEIO,
    SIS_PROFUNDIDADE_MAXIMA,
    SIS_ARQUIVO_GRANDE,
    SIS_DISCO_CHEIO,
    SIS_SEM_MEMORIA,
    SIS_ID_ESGOTADO,
    SIS_BUFFER_INSUFICIENTE,
    SIS_ERRO_RELOGIO,
    SIS_USUARIOS_CHEIO
} Resultado;

typedef enum { GRUPO_ALUNOS = 1, GRUPO_VISITANTES } Grupo;

typedef struct {
    unsigned int id;
    char nome[MAX_NOME + 1];
    Grupo grupo;
} Usuario;

typedef struct {
    uint64_t id;
    char nome[MAX_NOME + 1];
    TipoArquivo tipo;
    size_t tamanho;
    time_t criacao;
    time_t modificacao;
    time_t acesso;
    unsigned int dono_id;
    Grupo grupo;
    unsigned int permissoes;
    size_t quantidade_blocos;
    size_t blocos[MAX_BLOCOS_ARQUIVO];
} Arquivo;

typedef struct Diretorio {
    char nome[MAX_NOME + 1];
    struct Diretorio *pai;
    Arquivo **arquivos;
    size_t capacidade_arquivos;
    struct Diretorio **subpastas;
    size_t capacidade_subpastas;
    size_t quantidade_arquivos;
    size_t quantidade_subpastas;
} Diretorio;

typedef struct {
    unsigned char dados[TOTAL_BLOCOS][TAM_BLOCO];
    bool ocupado[TOTAL_BLOCOS];
} Disco;

typedef struct {
    Diretorio *raiz;
    Diretorio *atual;
    Usuario *usuarios;
    size_t capacidade_usuarios;
    size_t quantidade_usuarios;
    size_t usuario_atual;
    Disco disco;
    // ids comecam em 1; zero indica esgotamento
    uint64_t proximo_id;
} Sistema;

// regras detalhadas de uso e memoria em docs/contratos_t01md

// inicia zerado; a liberacao pode ser repetida
Resultado sistema_inicializar(Sistema *sistema);
void sistema_liberar(Sistema *sistema);
const char *resultado_mensagem(Resultado resultado);
const char *tipo_nome(TipoArquivo tipo);
const char *grupo_nome(Grupo grupo);

Resultado nome_validar(const char *nome);
// busca caminho absoluto sem mudar a pasta atual; nao libere a saida
Resultado diretorio_resolver(const Sistema *sistema, const char *caminho, const Diretorio **saida);
Resultado sistema_caminho_atual(const Sistema *sistema, char *saida, size_t capacidade);
Resultado sistema_criar_pasta(Sistema *sistema, const char *nome);
// aceita somente o nome de uma subpasta da pasta atual
Resultado sistema_entrar_pasta(Sistema *sistema, const char *nome);
Resultado sistema_voltar_pai(Sistema *sistema);
// consulta o arquivo sem alterar suas datas; nao libere a saida
Resultado sistema_consultar_arquivo(const Sistema *sistema, const char *nome,
                                    const Arquivo **saida);

// cadastra sem trocar o usuario atual
Resultado sistema_criar_usuario(Sistema *sistema, const char *nome, Grupo grupo);
Resultado sistema_trocar_usuario(Sistema *sistema, const char *nome);
Resultado permissao_converter(const char *texto, unsigned int *saida);
// buffers minimos: 4 para octal e 10 para simbolica
Resultado permissao_formatar(unsigned int mascara, char *octal, size_t capacidade_octal,
                             char *simbolica, size_t capacidade_simbolica);
bool arquivo_e_proprietario(const Arquivo *arquivo, const Usuario *usuario);
// confere os bits pedidos na classe de acesso escolhida
bool arquivo_tem_permissao(const Arquivo *arquivo, const Usuario *usuario, unsigned int bits);
Resultado sistema_alterar_permissoes(Sistema *sistema, const char *nome, const char *texto);

Resultado sistema_criar_arquivo(Sistema *sistema, const char *nome, TipoArquivo tipo);
// substitui o texto; null so e permitido com tamanho zero
Resultado sistema_escrever_arquivo(Sistema *sistema, const char *nome,
                                   const unsigned char *conteudo, size_t tamanho);
// le bytes sem terminador e atualiza acesso no sucesso
Resultado sistema_ler_arquivo(Sistema *sistema, const char *nome, unsigned char *saida,
                              size_t capacidade, size_t *tamanho_lido);
Resultado sistema_copiar_arquivo(Sistema *sistema, const char *origem, const char *destino_absoluto,
                                 const char *novo_nome);
// move ou renomeia; mesmo nome e pasta nao altera nada
Resultado sistema_mover_arquivo(Sistema *sistema, const char *origem, const char *destino_absoluto,
                                const char *novo_nome);
Resultado sistema_excluir_arquivo(Sistema *sistema, const char *nome);
// exige tipo programa e permissao x; atualiza acesso
Resultado sistema_executar_arquivo(Sistema *sistema, const char *nome);

// disco cuida dos blocos; arquivosc verifica o acesso
void disco_inicializar(Disco *disco);
size_t disco_blocos_necessarios(size_t tamanho);
size_t disco_blocos_livres(const Disco *disco);
// reserva todos os blocos pedidos ou nenhum
Resultado disco_reservar(Disco *disco, size_t quantidade, size_t *indices);
void disco_liberar(Disco *disco, const size_t *indices, size_t quantidade);
// reescreve e ajusta blocos; falhas preservam os dados
Resultado disco_reescrever(Disco *disco, Arquivo *arquivo, const unsigned char *conteudo,
                           size_t tamanho);
// le os blocos sem alterar datas
Resultado disco_ler(const Disco *disco, const Arquivo *arquivo, unsigned char *saida,
                    size_t capacidade, size_t *tamanho_lido);

#endif
