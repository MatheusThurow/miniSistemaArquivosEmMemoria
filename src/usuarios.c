#include "sistema_interno.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// retorna o usuario selecionado no sistema
const Usuario *obter_usuario_atual(const Sistema *sistema) {
    return &sistema->usuarios[sistema->usuario_atual];
}

// retorna o nome do grupo informado
const char *grupo_nome(Grupo grupo) {
    if (grupo == GRUPO_ALUNOS) {
        return "alunos";
    }
    if (grupo == GRUPO_VISITANTES) {
        return "visitantes";
    }
    return "grupo invalido";
}

// cadastra um usuario sem alterar o usuario atual
Resultado sistema_criar_usuario(Sistema *sistema, const char *nome, Grupo grupo) {
    if (!sistema_esta_pronto(sistema) || !nome) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    Resultado resultado = nome_validar(nome);
    if (resultado != SIS_OK) {
        return resultado;
    }
    if (grupo != GRUPO_ALUNOS && grupo != GRUPO_VISITANTES) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    for (size_t i = 0; i < sistema->quantidade_usuarios; ++i) {
        if (strcmp(sistema->usuarios[i].nome, nome) == 0) {
            return SIS_NOME_DUPLICADO;
        }
    }
    if (sistema->quantidade_usuarios == MAX_USUARIOS) {
        return SIS_USUARIOS_CHEIO;
    }
    if (sistema->quantidade_usuarios >= UINT_MAX) {
        return SIS_ID_ESGOTADO;
    }
    if (sistema->quantidade_usuarios == sistema->capacidade_usuarios) {
        size_t capacidade;
        Usuario *novos_usuarios =
            criar_vetor_maior(sistema->usuarios, sistema->quantidade_usuarios,
                              sistema->capacidade_usuarios, sizeof *sistema->usuarios, &capacidade);
        if (!novos_usuarios) {
            return SIS_SEM_MEMORIA;
        }
        free(sistema->usuarios);
        sistema->usuarios = novos_usuarios;
        sistema->capacidade_usuarios = capacidade;
    }
    Usuario novo_usuario = {0};
    novo_usuario.id = (unsigned int)sistema->quantidade_usuarios + 1;
    strcpy(novo_usuario.nome, nome);
    novo_usuario.grupo = grupo;
    sistema->usuarios[sistema->quantidade_usuarios++] = novo_usuario;
    return SIS_OK;
}

// seleciona um usuario cadastrado pelo nome
Resultado sistema_trocar_usuario(Sistema *sistema, const char *nome) {
    if (!sistema_esta_pronto(sistema) || !nome) {
        return SIS_ARGUMENTO_INVALIDO;
    }
    for (size_t i = 0; i < sistema->quantidade_usuarios; ++i) {
        if (strcmp(sistema->usuarios[i].nome, nome) == 0) {
            sistema->usuario_atual = i;
            return SIS_OK;
        }
    }
    return SIS_NAO_ENCONTRADO;
}
