#ifndef SISTEMA_INTERNO_H
#define SISTEMA_INTERNO_H

#include "sistema.h"

// funcoes auxiliares compartilhadas entre os modulos; o menu usa sistemah
void *criar_vetor_maior(const void *antigo, size_t quantidade, size_t capacidade,
                        size_t tamanho_item, size_t *nova_capacidade);
bool sistema_esta_pronto(const Sistema *sistema);
const Usuario *obter_usuario_atual(const Sistema *sistema);
Arquivo *buscar_arquivo(const Diretorio *diretorio, const char *nome);
Diretorio *buscar_pasta(const Diretorio *diretorio, const char *nome);
bool nome_ja_existe(const Diretorio *diretorio, const char *nome);
Resultado obter_arquivo_atual(Sistema *sistema, const char *nome, Arquivo **arquivo);

#endif
