#include "sistema.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <inttypes.h>
#include <unistd.h>

static void mostrar_menu(void) {
    puts(
        "1. Listar conteudo\n2. Criar pasta\n3. Entrar em subpasta pelo nome\n4. Voltar ao pai\n5. Criar arquivo\n6. Escrever arquivo\n7. Ler arquivo\n8. Copiar arquivo\n9. Mover ou renomear arquivo\n10. Excluir arquivo\n11. Detalhes\n12. Alterar permissoes\n13. Trocar usuario\n14. Simular execucao\n15. Mostrar blocos\n16. Criar usuario\n0. Sair");
}

static void mostrar_localizacao(const Sistema *sistema, const char *caminho) {
    const Usuario *usuario = &sistema->usuarios[sistema->usuario_atual];
    printf("Usuario: %s | Grupo: %s | Pasta atual: %s\n",
           usuario->nome, grupo_nome(usuario->grupo), caminho);
}

static void mostrar_arquivos_legiveis(const Sistema *sistema) {
    const Usuario *usuario = &sistema->usuarios[sistema->usuario_atual];
    bool encontrou = false;
    for (size_t i = 0; i < sistema->atual->quantidade_arquivos; ++i) {
        const Arquivo *arquivo = sistema->atual->arquivos[i];
        if (arquivo_tem_permissao(arquivo, usuario, PERM_R)) {
            printf("%s%s", encontrou ? ", " : "Arquivos com leitura permitida: ", arquivo->nome);
            encontrou = true;
        }
    }
    if (encontrou) {
        puts("");
    }
}

// le uma linha: retorna 1 se valida, 0 no fim ou -1 se longa
static int ler_linha(const char *mensagem, char *texto, size_t limite) {
    printf("%s", mensagem);
    fflush(stdout);
    if (!fgets(texto, (int)(limite + 2), stdin)) {
        return 0;
    }
    size_t tamanho = strlen(texto);
    if (tamanho && texto[tamanho - 1] == '\n') {
        texto[--tamanho] = 0;
    } else if (!feof(stdin)) {
        int caractere;
        while ((caractere = getchar()) != '\n' && caractere != EOF) {
        }
        puts("Entrada longa demais; operacao cancelada.");
        return -1;
    }
    if (tamanho && texto[tamanho - 1] == '\r') {
        texto[--tamanho] = 0;
    }
    if (tamanho > limite) {
        puts("Entrada longa demais; operacao cancelada.");
        return -1;
    }
    return 1;
}

// converte o texto em numero e verifica a faixa permitida
static bool ler_numero(const char *texto, long minimo, long maximo, long *saida) {
    char *fim;
    errno = 0;
    long valor = strtol(texto, &fim, 10);
    if (fim == texto || errno) {
        return false;
    }
    while (isspace((unsigned char)*fim)) {
        ++fim;
    }
    if (*fim || valor < minimo || valor > maximo) {
        return false;
    }
    *saida = valor;
    return true;
}

// exibe os metadados e os indices dos blocos do arquivo
static void mostrar_detalhes(const Sistema *sistema, const Arquivo *arquivo) {
    char permissao_numerica[4], permissao_simbolica[10], datas[3][32];
    time_t tempos[] = {arquivo->criacao, arquivo->modificacao, arquivo->acesso};
    permissao_formatar(arquivo->permissoes, permissao_numerica, sizeof permissao_numerica,
                       permissao_simbolica, sizeof permissao_simbolica);
    for (size_t i = 0; i < 3; ++i) {
        struct tm *data_local = localtime(&tempos[i]);
        if (!data_local || !strftime(datas[i], sizeof datas[i], "%Y-%m-%d %H:%M:%S", data_local)) {
            strcpy(datas[i], "indisponivel");
        }
    }
    const char *dono = "desconhecido";
    for (size_t i = 0; i < sistema->quantidade_usuarios; ++i) {
        if (sistema->usuarios[i].id == arquivo->dono_id) {
            dono = sistema->usuarios[i].nome;
        }
    }
    printf("%s | ID=%" PRIu64
           " | %s | %zu bytes | %s/%s | %s %s\nCriacao: %s | Modificacao: %s | Acesso: %s\nBlocos:",
           arquivo->nome, arquivo->id, tipo_nome(arquivo->tipo), arquivo->tamanho, dono,
           grupo_nome(arquivo->grupo), permissao_numerica, permissao_simbolica, datas[0], datas[1],
           datas[2]);
    for (size_t i = 0; i < arquivo->quantidade_blocos; ++i) {
        printf(" %zu", arquivo->blocos[i]);
    }
    puts("");
}

// exibe os blocos dos arquivos em toda a arvore
static void mostrar_blocos(const Diretorio *diretorio, const char *prefixo) {
    char caminho[TAM_CAMINHO];
    for (size_t j = 0; j < diretorio->quantidade_arquivos; ++j) {
        const Arquivo *arquivo = diretorio->arquivos[j];
        printf("%s/%s (%zu bytes):", prefixo, arquivo->nome, arquivo->tamanho);
        for (size_t i = 0; i < arquivo->quantidade_blocos; ++i) {
            size_t bytes_no_bloco = arquivo->tamanho - i * TAM_BLOCO;
            if (bytes_no_bloco > TAM_BLOCO) {
                bytes_no_bloco = TAM_BLOCO;
            }
            printf(" [%zu: %zu bytes uteis]", arquivo->blocos[i], bytes_no_bloco);
        }
        puts("");
    }
    for (size_t i = 0; i < diretorio->quantidade_subpastas; ++i) {
        snprintf(caminho, sizeof caminho, "%s/%s", prefixo, diretorio->subpastas[i]->nome);
        mostrar_blocos(diretorio->subpastas[i], caminho);
    }
}

// inicia o sistema, controla o menu e libera a memoria ao sair
int main(void) {
    Sistema sistema = {0};
    Resultado resultado = sistema_inicializar(&sistema);
    if (resultado != SIS_OK) {
        fprintf(stderr, "%s\n", resultado_mensagem(resultado));
        return EXIT_FAILURE;
    }
    char entrada[MAX_CONTEUDO + 2], nome[MAX_NOME + 2], destino[TAM_CAMINHO + 2],
        novo_nome[MAX_NOME + 2], caminho[TAM_CAMINHO];
    long opcao, tipo;
    int situacao_leitura;
    const char *terminal = getenv("TERM");
    bool tela_interativa = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO) &&
                           terminal && strcmp(terminal, "dumb") != 0;
    if (tela_interativa) {
        fputs("\033[H\033[2J", stdout);
    }
    mostrar_menu();
    for (;;) {
        sistema_caminho_atual(&sistema, caminho, sizeof caminho);
        puts("");
        mostrar_localizacao(&sistema, caminho);
        mostrar_arquivos_legiveis(&sistema);
        situacao_leitura = ler_linha("Opcao: ", entrada, MAX_CONTEUDO);
        if (!situacao_leitura) {
            break;
        }
        if (situacao_leitura < 0) {
            continue;
        }
        if (!ler_numero(entrada, 0, 16, &opcao)) {
            puts("Opcao invalida.");
            continue;
        }
        if (!opcao) {
            break;
        }
        if (tela_interativa) {
            fputs("\033[H\033[2J", stdout);
            mostrar_menu();
            puts("");
            mostrar_localizacao(&sistema, caminho);
        }
        resultado = SIS_OK;
        if (opcao == 1) {
            for (size_t i = 0; i < sistema.atual->quantidade_subpastas; ++i) {
                printf("[pasta] %s\n", sistema.atual->subpastas[i]->nome);
            }
            for (size_t i = 0; i < sistema.atual->quantidade_arquivos; ++i) {
                mostrar_detalhes(&sistema, sistema.atual->arquivos[i]);
            }
            continue;
        }
        if (opcao == 4) {
            resultado = sistema_voltar_pai(&sistema);
        } else if (opcao == 15) {
            size_t livres = disco_blocos_livres(&sistema.disco);
            printf("Capacidade: %d bytes | Livres: %zu | Ocupados: %zu\n", TOTAL_BLOCOS * TAM_BLOCO,
                   livres, TOTAL_BLOCOS - livres);
            mostrar_blocos(sistema.raiz, "");
            continue;
        } else if (opcao == 3) {
            if (sistema.atual->quantidade_subpastas == 0) {
                puts("Nao ha subpastas na pasta atual.");
                continue;
            }
            puts("Subpastas disponiveis:");
            for (size_t i = 0; i < sistema.atual->quantidade_subpastas; ++i) {
                printf("[pasta] %s\n", sistema.atual->subpastas[i]->nome);
            }
            situacao_leitura =
                ler_linha("Nome da subpasta: ", nome, MAX_NOME);
            if (!situacao_leitura) {
                break;
            }
            if (situacao_leitura < 0) {
                continue;
            }
            resultado = sistema_entrar_pasta(&sistema, nome);
        } else {
            if (opcao == 13) {
                puts("Usuarios cadastrados:");
                for (size_t i = 0; i < sistema.quantidade_usuarios; ++i) {
                    printf("%s (%s)\n", sistema.usuarios[i].nome,
                           grupo_nome(sistema.usuarios[i].grupo));
                }
            }
            situacao_leitura = ler_linha(
                opcao == 13 || opcao == 16 ? "Nome do usuario: " : "Nome na pasta atual: ", nome,
                MAX_NOME);
            if (!situacao_leitura) {
                break;
            }
            if (situacao_leitura < 0) {
                continue;
            }
            switch (opcao) {
            case 2:
                resultado = sistema_criar_pasta(&sistema, nome);
                break;
            case 5:
                situacao_leitura =
                    ler_linha("Tipo (1 numerico, 2 caractere, 3 binario, 4 programa): ", entrada,
                              MAX_CONTEUDO);
                if (situacao_leitura != 1) {
                    break;
                }
                if (!ler_numero(entrada, 1, 4, &tipo)) {
                    resultado = SIS_TIPO_INCORRETO;
                } else {
                    resultado = sistema_criar_arquivo(&sistema, nome, (TipoArquivo)tipo);
                }
                break;
            case 6:
                situacao_leitura =
                    ler_linha("Conteudo (uma linha; vazio permitido): ", entrada, MAX_CONTEUDO);
                if (situacao_leitura != 1) {
                    break;
                }
                resultado = sistema_escrever_arquivo(&sistema, nome, (unsigned char *)entrada,
                                                     strlen(entrada));
                break;
            case 7: {
                unsigned char bytes[MAX_CONTEUDO];
                size_t tamanho_lido;
                resultado = sistema_ler_arquivo(&sistema, nome, bytes, sizeof bytes, &tamanho_lido);
                if (resultado == SIS_OK) {
                    printf("Arquivo: %s%s%s\n", caminho, strcmp(caminho, "/") == 0 ? "" : "/", nome);
                    fwrite(bytes, 1, tamanho_lido, stdout);
                    puts("");
                }
                break;
            }
            case 8:
            case 9:
                situacao_leitura =
                    ler_linha("Destino: caminho absoluto da pasta: ", destino, TAM_CAMINHO - 1);
                if (situacao_leitura != 1) {
                    break;
                }
                situacao_leitura = ler_linha(
                    "Nome final (para renomear, use a pasta atual como destino): ", novo_nome,
                    MAX_NOME);
                if (situacao_leitura != 1) {
                    break;
                }
                if (opcao == 8) {
                    resultado = sistema_copiar_arquivo(&sistema, nome, destino, novo_nome);
                } else {
                    resultado = sistema_mover_arquivo(&sistema, nome, destino, novo_nome);
                }
                break;
            case 10:
                resultado = sistema_excluir_arquivo(&sistema, nome);
                break;
            case 11: {
                const Arquivo *arquivo;
                resultado = sistema_consultar_arquivo(&sistema, nome, &arquivo);
                if (resultado == SIS_OK) {
                    mostrar_detalhes(&sistema, arquivo);
                }
                break;
            }
            case 12:
                situacao_leitura =
                    ler_linha("Permissoes (tres digitos 0-7): ", entrada, MAX_CONTEUDO);
                if (situacao_leitura != 1) {
                    break;
                }
                resultado = sistema_alterar_permissoes(&sistema, nome, entrada);
                break;
            case 16:
                situacao_leitura =
                    ler_linha("Grupo (1 alunos, 2 visitantes): ", entrada, MAX_CONTEUDO);
                if (situacao_leitura != 1) {
                    break;
                }
                if (!ler_numero(entrada, 1, 2, &tipo)) {
                    resultado = SIS_ARGUMENTO_INVALIDO;
                } else {
                    resultado = sistema_criar_usuario(&sistema, nome, (Grupo)tipo);
                }
                break;
            case 13:
                resultado = sistema_trocar_usuario(&sistema, nome);
                break;
            case 14:
                resultado = sistema_executar_arquivo(&sistema, nome);
                if (resultado == SIS_OK) {
                    puts("Execucao simulada; nenhum comando real foi executado.");
                }
                break;
            }
            if (!situacao_leitura) {
                break;
            }
            if (situacao_leitura < 0) {
                continue;
            }
        }
        puts(resultado_mensagem(resultado));
    }
    sistema_liberar(&sistema);
    puts("Sistema liberado. Ate mais.");
    return EXIT_SUCCESS;
}

