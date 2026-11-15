#include "sistema_interno.h"
#include <stdio.h>
#include <string.h>

// converte tres digitos octais em bits de permissao
Resultado permissao_converter(const char *texto, unsigned int *saida) {
    if (!texto || !saida) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    if (strlen(texto) != 3) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    /* cada digito octal representa tres bits: leitura, escrita e execucao */
    unsigned int mascara = 0;
    for (size_t i = 0; i < 3; ++i) {
        if (texto[i] < '0' || texto[i] > '7') {
            return SIS_ARGUMENTO_INVALIDO;
        }
        mascara = (mascara << 3) | (unsigned)(texto[i] - '0');
    }
    *saida = mascara;
    return SIS_OK;
}

// transforma os bits em permissoes numericas e simbolicas
Resultado permissao_formatar(unsigned int mascara, char *permissao_numerica,
                             size_t capacidade_numerica, char *permissao_simbolica,
                             size_t capacidade_simbolica) {
    if (!permissao_numerica || !permissao_simbolica || mascara > 0777) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    if (capacidade_numerica < 4 || capacidade_simbolica < 10) {
        return SIS_BUFFER_INSUFICIENTE;
    }
    snprintf(permissao_numerica, capacidade_numerica, "%03o", mascara);
    for (size_t i = 0; i < 9; ++i) {
        permissao_simbolica[i] = mascara & (1u << (8 - i)) ? "rwx"[i % 3] : '-';
    }
    permissao_simbolica[9] = 0;
    return SIS_OK;
}

// verifica se o usuario e o dono do arquivo
bool arquivo_e_proprietario(const Arquivo *arquivo, const Usuario *usuario) {
    return arquivo->dono_id == usuario->id;
}

// confere o acesso pela classe de dono, grupo ou outros
bool arquivo_tem_permissao(const Arquivo *arquivo, const Usuario *usuario, unsigned int bits) {
    if (bits == 0 || bits > 7) {
        return false;
    }

    /* escolhe somente uma classe: dono, grupo ou outros */
    unsigned int deslocamento;
    if (arquivo_e_proprietario(arquivo, usuario)) {
        deslocamento = 6;
    } else if (arquivo->grupo == usuario->grupo) {
        deslocamento = 3;
    } else {
        deslocamento = 0;
    }

    /* traz a classe escolhida para os tres bits finais */
    unsigned int permissoes_da_classe = arquivo->permissoes >> deslocamento;
    return (permissoes_da_classe & bits) == bits;
}

// permite que o dono altere as permissoes do arquivo
Resultado sistema_alterar_permissoes(Sistema *sistema, const char *nome, const char *texto) {
    Arquivo *arquivo;
    unsigned int mascara;
    Resultado resultado = obter_arquivo_atual(sistema, nome, &arquivo);
    if (resultado != SIS_OK) {
        return resultado;
    }
    if (!arquivo_e_proprietario(arquivo, obter_usuario_atual(sistema))) {
        return SIS_PERMISSAO_NEGADA;
    }
    resultado = permissao_converter(texto, &mascara);
    if (resultado == SIS_OK) {
        arquivo->permissoes = mascara;
    }
    return resultado;
}
